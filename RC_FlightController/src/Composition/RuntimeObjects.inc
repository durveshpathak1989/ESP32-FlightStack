static Logger flightLogger(FLIGHT_LOGGER_CAPACITY);
static Esp32MotorServiceAdapter motorServiceAdapter;
static Esp32SystemServiceAdapter clockServiceAdapter;
static Esp32DiagnosticServiceAdapter diagnosticServiceAdapter;

// ─────────────────────────────────────────────────────────────
//  IMU loop timing instrumentation — Test 7.1
// ─────────────────────────────────────────────────────────────
struct TimingStats {
    uint32_t count;
    double   wMean;
    double   wM2;
    uint32_t jitterMax;
    uint32_t jitterViolations;
    double   jMean;
    double   jM2;
    uint16_t buf[TIMING_BUF_SIZE];
    uint16_t bufHead;
    bool     bufFull;
};

// Loop Jitter timing stats

struct ExecTimingStats {
    uint32_t lastControlUs;
    uint32_t lastFullUs;
    uint32_t maxControlUs;
    uint32_t maxFullUs;
    uint32_t controlOverruns;
    uint32_t fullOverruns;

    // Phase timing to identify the blocker when ctrl time is too high.
    uint32_t lastImuUs;
    uint32_t lastRcUs;
    uint32_t lastAhrsUs;
    uint32_t lastMotorUs;
    uint32_t lastStateUs;
    uint32_t lastLogUs;
    uint32_t missedTimerReleases;
};

static volatile ExecTimingStats g_execTiming = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static volatile uint32_t g_lastWifiServiceUs = 0;
static TelemetryWifiServiceAdapter wifiServiceAdapter(telemetryWiFi);
static WifiServiceTask wifiServiceTask(
    wifiServiceAdapter, clockServiceAdapter, g_lastWifiServiceUs);

static void updateExecTimingAndPrint(uint32_t controlUs, uint32_t fullUs,
                                     uint32_t periodUs, uint32_t targetUs)
{
    g_execTiming.lastControlUs = controlUs;
    g_execTiming.lastFullUs    = fullUs;

    if (controlUs > g_execTiming.maxControlUs) {
        g_execTiming.maxControlUs = controlUs;
    }

    if (fullUs > g_execTiming.maxFullUs) {
        g_execTiming.maxFullUs = fullUs;
    }

    if (controlUs > targetUs) {
        g_execTiming.controlOverruns++;
    }

    if (fullUs > targetUs) {
        g_execTiming.fullOverruns++;
    }

    if (!LOG_CONTROL_TIMING) return;

    // Print only once per second so Serial does not dominate the control loop.
    static uint32_t lastPrintMs = 0;
    uint32_t nowMs = clockServiceAdapter.milliseconds();

    if (nowMs - lastPrintMs >= 1000) {
        lastPrintMs = nowMs;

        int32_t jitterSigned  = (int32_t)periodUs - (int32_t)targetUs;
        int32_t headroomFull  = (int32_t)targetUs - (int32_t)fullUs;

        DBG_PRINTF("[TIME] period=%luus jitter=%+ldus ctrl=%luus full=%luus "
                      "headroom=%+ldus maxCtrl=%luus maxFull=%luus "
                      "overCtrl=%lu overFull=%lu missed=%lu "
                      "phase imu=%luus rc=%luus ahrs=%luus motor=%luus state=%luus log=%luus wifi=%luus\n",
                      (unsigned long)periodUs,
                      (long)jitterSigned,
                      (unsigned long)controlUs,
                      (unsigned long)fullUs,
                      (long)headroomFull,
                      (unsigned long)g_execTiming.maxControlUs,
                      (unsigned long)g_execTiming.maxFullUs,
                      (unsigned long)g_execTiming.controlOverruns,
                      (unsigned long)g_execTiming.fullOverruns,
                      (unsigned long)g_execTiming.missedTimerReleases,
                      (unsigned long)g_execTiming.lastImuUs,
                      (unsigned long)g_execTiming.lastRcUs,
                      (unsigned long)g_execTiming.lastAhrsUs,
                      (unsigned long)g_execTiming.lastMotorUs,
                      (unsigned long)g_execTiming.lastStateUs,
                      (unsigned long)g_execTiming.lastLogUs,
                      (unsigned long)g_lastWifiServiceUs);
    }
}

static TimingStats       g_timing;
static SnapshotRte<TimingStats> timingRte(g_timing);

static void resetTimingStats()
{
    if (timingRte.lock(pdMS_TO_TICKS(20))) {
        memset(&g_timing, 0, sizeof(g_timing));
        timingRte.unlock();
    }
}

// ─────────────────────────────────────────────────────────────
//  IMU + attitude estimator objects
// ─────────────────────────────────────────────────────────────
SelectedImu imu(PIN_MPU_CS);
SelectedImuServiceAdapter imuServiceAdapter(imu);
CalibrationManager calManager;
MahonyAHRS mahony;
AttitudeEstimate mahonyAtt;
AttitudeEKF attitudeEKF;
MadgwickAHRS madgwickAHRS;
static rte::SenderReceiverPort<AttitudeEstimatorInput> estimatorInputPort;
static rte::SenderReceiverPort<AttitudeEstimate> estimatorOutputPort;
AttitudeEstimatorRouter attitudeEstimator(
    attitudeEKF, mahony, madgwickAHRS,
    estimatorInputPort, estimatorOutputPort);
NotchFilter notchAx, notchAy, notchAz;
NotchFilter notchGx, notchGy, notchGz;
static FlightToF_VL53L4CX tofSensor(Wire);
static BatteryMonitor batteryMonitor;
static Esp32BatteryServiceAdapter batteryServiceAdapter(
    batteryMonitor, PIN_BATTERY_ADC, BATTERY_CELL_COUNT,
    BATTERY_RTOP_OHMS, BATTERY_RBOTTOM_OHMS,
    BATTERY_NOMINAL_V, BATTERY_ADC_CAL_SCALE);
static float    g_dynamicNotchHz = TUNE_NOTCH_FREQ_HZ;
static bool     g_dynamicNotchTracking = false;
static uint32_t g_lastDynamicNotchUpdateMs = 0;

SpectrumAnalyzer spectrumAnalyzer(NOTCH_SAMPLE_HZ);

#define FLIGHT_LOG_SIZE 100   // expanded debug row: 300 samples @ 100 Hz = 3 s



// ─────────────────────────────────────────────────────────────
//  Shared flight state
// ─────────────────────────────────────────────────────────────
static FlightState       g_state;
static SnapshotRte<FlightState> flightStateRte(g_state);
static Esp32GpsServiceAdapter gpsServiceAdapter(
    gps, PIN_GPS_RX, PIN_GPS_TX, 9600);
static GpsServiceTask gpsServiceTask(
    gpsServiceAdapter, clockServiceAdapter, flightStateRte, g_state);
static Esp32CpuLoadServiceAdapter cpuLoadServiceAdapter(cpuUtilization);
static CpuServiceTask cpuServiceTask(cpuLoadServiceAdapter, flightStateRte, g_state);
static SemaphoreHandle_t g_i2cMutex;
static Esp32RangeServiceAdapter rangeServiceAdapter(
    tofSensor, g_i2cMutex, TOF_ST_ADDRESS, TOF_TIMING_BUDGET_US,
    TOF_INTER_MEASUREMENT_MS, TOF_STALE_MS);
static Esp32BarometerServiceAdapter barometerServiceAdapter(
    bmp280, g_i2cMutex, PIN_BMP_SDA, PIN_BMP_SCL, 100000);
static TofServiceTask tofServiceTask(
    rangeServiceAdapter, flightStateRte, g_state, TOF_TASK_PERIOD_MS);
static BarometerServiceTask barometerServiceTask(
    barometerServiceAdapter, clockServiceAdapter, flightStateRte, g_state);

static volatile uint32_t g_pidResetCount = 0;
static volatile uint32_t g_modeTransitionCount = 0;
static volatile uint32_t g_lastModeChangeMs = 0;
static volatile uint32_t g_armingTransitionCount = 0;
static volatile uint32_t g_lastArmChangeMs = 0;
static bool g_transitionTrackerInitialized = false;
static flight::FlightMode g_lastObservedMode = flight::FlightMode::Disarmed;
static bool g_lastObservedArmed = false;

static rte::SenderReceiverPort<LevelTrimInput> levelTrimInputPort;
static rte::SenderReceiverPort<LevelTrimOutput> levelTrimOutputPort;
static LevelTrimService levelTrim(levelTrimInputPort, levelTrimOutputPort);

// PID-trace toggle (Serial 'p'). Off at boot so the log stays clean.
static volatile bool g_pidTrace = false;

static TuningState       g_tuning;
static SnapshotRte<TuningState> tuningRte(g_tuning);
static PreferencesTuningStore g_tuningStore;

// Tune transaction diagnostics for GCS verification.
// request_seq increments when /tune accepts a payload while disarmed.
// apply_seq follows request_seq after taskControl applies the values to PID/AHRS objects.
// reject_seq increments when /tune is rejected, typically because the drone is armed.
static volatile uint32_t g_tuneRequestSeq = 0;
static volatile uint32_t g_tuneApplySeq   = 0;
static volatile uint32_t g_tuneRejectSeq  = 0;
static volatile bool     g_notchResetPending = false;
static volatile uint8_t  g_ahrsFilterModeActive = 0;
static rte::SenderReceiverPort<flight::ReceiverFrame> receiverFramePort;
static Esp32ReceiverServiceAdapter receiverServiceAdapter(
    rcReceiver, PIN_IBUS_RX, PIN_IBUS_TX, 2);
static CalibrationServiceAdapter calibrationServiceAdapter(calManager);

static Esp32FlightScheduler flightScheduler;

static PidController pidRateRoll(
    TUNE_RATE_ROLL_KP, TUNE_RATE_ROLL_KI, TUNE_RATE_ROLL_KD);
static PidController pidRatePitch(
    TUNE_RATE_PITCH_KP, TUNE_RATE_PITCH_KI, TUNE_RATE_PITCH_KD);
static PidController pidRateYaw(
    TUNE_RATE_YAW_KP, TUNE_RATE_YAW_KI, TUNE_RATE_YAW_KD);

static PidController pidAngleRoll(
    TUNE_ANGLE_ROLL_KP, TUNE_ANGLE_ROLL_KI, TUNE_ANGLE_ROLL_KD);
static PidController pidAnglePitch(
    TUNE_ANGLE_PITCH_KP, TUNE_ANGLE_PITCH_KI, TUNE_ANGLE_PITCH_KD);
static PidController pidAngleYaw(TUNE_ANGLE_YAW_KP, 0.0f, 0.0f);
static rte::SenderReceiverPort<CascadedControlInput> controllerInputPort;
static rte::SenderReceiverPort<TuningState> controllerConfigPort;
static rte::SenderReceiverPort<CascadedControlOutput> controllerOutputPort;
static CascadedController flightController(
    pidRateRoll, pidRatePitch, pidRateYaw,
    pidAngleRoll, pidAnglePitch, pidAngleYaw,
    controllerInputPort, controllerConfigPort, controllerOutputPort);
static rte::SenderReceiverPort<MotorMixerInput> motorMixerInputPort;
static rte::SenderReceiverPort<MotorMixerConfig> motorMixerConfigPort;
static rte::SenderReceiverPort<MotorMixerOutput> motorMixerOutputPort;
static MotorMixer motorMixer(
    motorMixerInputPort, motorMixerConfigPort, motorMixerOutputPort);

// ─────────────────────────────────────────────────────────────
