/*
 * Name: RC_FlightController.ino
 * Use: Main Arduino sketch that starts sensors, tasks, telemetry, calibration, PID control, and motor output.
 * Version: 6.1.0
 * Created by: Durvesh Pathak dp676@cornell.edu
 */

/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v6.1.0                                ║
 * ║  FlySky FS-iA6B iBUS  +  MPU-9250/6500  +  BMP280  +  GPS       ║
 * ║  Fully autonomous — no keyboard required                         ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  v2.3.2 changes (over the pasted v2.3.1 working file):           ║
 * ║   • PID telemetry FIXED: real rO/pO/yO now published in          ║
 * ║     FlightState and printed (was feeding 4 motor values into     ║
 * ║     3 out-slots, garbling yawSP/hold). [PID] line is now a       ║
 * ║     sibling of the 1 Hz block at ~4 Hz, toggled by Serial 'p'.   ║
 * ║   • No flight-behavior changes: gains, yaw-stick negation,       ║
 * ║     heading hold, LPF reset, output limits all preserved.        ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                           ║
 * ║   FS-iA6B iBUS port → GPIO 16 (UART2 RX)                        ║
 * ║   MPU-9250 SCLK→5  MOSI→18  MISO→19  NCS→33  VCC→3.3V          ║
 * ║   BMP280 SDA→21  SCL→22  VCC→3.3V  GND→GND  CSB→3.3V           ║
 * ║   GPS TXD→13  VCC→3.3V  GND→GND (UART1)                         ║
 * ║   Motors: FL→25  FR→15  RL→14  RR→32                            ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  RC SWITCH ASSIGNMENTS                                            ║
 * ║   CH7  SWA  → ARM / DISARM                                       ║
 * ║   CH8  SWB  → ANGLE / ACRO                                       ║
 * ║   CH9  SWC  → Accel confirm during calibration                   ║
 * ║   CH10 SWD  → CALIBRATION trigger (flip UP while disarmed)       ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  SERIAL COMMANDS                                                  ║
 * ║   p  → toggle ~4 Hz [PID] tuning trace on/off (off at boot)     ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  HTTP ENDPOINTS                                                   ║
 * ║   GET  /telemetry     — full state JSON                          ║
 * ║   POST /tune          — apply PID / Mahony gains (disarmed)      ║
 * ║   GET/POST /update    — Web OTA update (disarmed/motors off)     ║
 * ║   GET  /log?since=N   — calibration log lines                    ║
 * ║   GET  /timing        — IMU jitter stats JSON (Test 7.1)         ║
 * ║   POST /timing/reset  — reset jitter stats (between conditions)  ║
 * ║   GET  /timing/csv    — raw period_us ring buffer CSV            ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <SPI.h>
#include <stdarg.h>
#include "src/Submodules/DebugConfig/DebugConfig.h"
#include "src/Submodules/MotorControl/MotorControl.h"
#include "src/Platforms/Esp32/Imu/SelectedImu.h"
#include "src/Submodules/iFly/FlySkyiBUS.h"
#include "src/Submodules/WiFiTelemetry/TelemetryWiFi.h"
#include "src/Submodules/BatteryMonitor/BatteryMonitor.h"
#include "src/Submodules/BMP280/BMP280Sensor.h"
#include "src/Submodules/EKF/AttitudeEKF.h"
#include "src/Submodules/ToF/FlightToF_VL53L4CX.h"
#include "src/Submodules/NotchFilter/NotchFilter.h"
#include "src/Submodules/FFT/SpectrumAnalyzer.h"
#include "src/Submodules/Madgwick/MadgwickAHRS.h"
#include "src/Submodules/MahonyAHRS/MahonyAHRS.h"
#include "src/Submodules/ESP32Core/CPUUtilization.h"
#include "src/Submodules/GPS/GPSSensor.h"
#include "src/Submodules/Logger/Logger.h"
#include "FirmwareBuildInfo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "src/Submodules/CalManager/CalibrationManager.h"
#include "src/Application/FlightConfig.h"
#include "src/Application/Calibration/LevelTrimService.h"
#include "src/Application/Control/CascadedController.h"
#include "src/Application/Control/LowPassFilter.h"
#include "src/Application/Control/MotorMixer.h"
#include "src/Application/Control/PidController.h"
#include "src/Application/Configuration/TuningState.h"
#include "src/Application/Estimation/AttitudeEstimatorRouter.h"
#include "src/Application/Safety/FlightSafetyPolicy.h"
#include "src/Application/State/FlightState.h"
#include "src/Platforms/Esp32/Storage/PreferencesTuningStore.h"
#include "src/Platforms/Esp32/Runtime/Esp32FlightScheduler.h"
#include "src/Platforms/Esp32/Runtime/SnapshotRte.h"
#include "src/Platforms/Esp32/Tasks/CpuServiceTask.h"
#include "src/Platforms/Esp32/Tasks/GpsServiceTask.h"
#include "src/Platforms/Esp32/Tasks/ReceiverServiceTask.h"
#include "src/Platforms/Esp32/Tasks/BarometerServiceTask.h"
#include "src/Platforms/Esp32/Tasks/TofServiceTask.h"
#include "src/Platforms/Esp32/Tasks/WifiServiceTask.h"

static Logger flightLogger(FLIGHT_LOGGER_CAPACITY);

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
static WifiServiceTask wifiServiceTask(telemetryWiFi, g_lastWifiServiceUs);

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
    uint32_t nowMs = millis();

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
CalibrationManager calManager;
MahonyAHRS mahony;
AttitudeEstimate mahonyAtt;
AttitudeEKF attitudeEKF;
MadgwickAHRS madgwickAHRS;
AttitudeEstimatorRouter attitudeEstimator(attitudeEKF, mahony, madgwickAHRS);
NotchFilter notchAx, notchAy, notchAz;
NotchFilter notchGx, notchGy, notchGz;
static FlightToF_VL53L4CX tofSensor(Wire);
static BatteryMonitor batteryMonitor;
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
static GpsServiceTask gpsServiceTask(gps, flightStateRte, g_state);
static CpuServiceTask cpuServiceTask(cpuUtilization, flightStateRte, g_state);
static SemaphoreHandle_t g_i2cMutex;
static TofServiceTask tofServiceTask(
    tofSensor, g_i2cMutex, flightStateRte, g_state,
    TOF_TASK_PERIOD_MS, TOF_STALE_MS);
static BarometerServiceTask barometerServiceTask(
    bmp280, g_i2cMutex, flightStateRte, g_state);

static volatile uint32_t g_pidResetCount = 0;
static volatile uint32_t g_modeTransitionCount = 0;
static volatile uint32_t g_lastModeChangeMs = 0;
static volatile uint32_t g_armingTransitionCount = 0;
static volatile uint32_t g_lastArmChangeMs = 0;
static bool g_transitionTrackerInitialized = false;
static FlightMode g_lastObservedMode = FlightMode::DISARMED;
static bool g_lastObservedArmed = false;

static LevelTrimService levelTrim;

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

// ─────────────────────────────────────────────────────────────
//  Calibration state machine
// ─────────────────────────────────────────────────────────────
enum class CalibState : uint8_t {
    IDLE=0, REQUESTED, RUNNING_GYRO, RUNNING_ACCEL, RUNNING_MAG, SAVING, DONE
};
static volatile CalibState g_calibState = CalibState::IDLE;

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
static CascadedController flightController(
    pidRateRoll, pidRatePitch, pidRateYaw,
    pidAngleRoll, pidAnglePitch, pidAngleYaw);
static MotorMixer motorMixer;

// ─────────────────────────────────────────────────────────────
//  Tuning sync helpers
// ─────────────────────────────────────────────────────────────
static void syncTuningFromObjects()
{
    // Copy compile-time defaults into runtime-tunable state at boot.
    g_tuning.max_angle_deg              = TUNE_MAX_ANGLE_DEG;
    g_tuning.max_rate_dps               = TUNE_MAX_RATE_DPS;
    g_tuning.max_pitch_rate_dps         = TUNE_MAX_RATE_DPS;

    g_tuning.roll_output_limit          = TUNE_ROLL_OUTPUT_LIMIT;
    g_tuning.pitch_output_limit         = TUNE_PITCH_OUTPUT_LIMIT;
    g_tuning.yaw_output_limit           = TUNE_YAW_OUTPUT_LIMIT;

    g_tuning.throttle_expo              = TUNE_THROTTLE_EXPO;
    g_tuning.throttle_up_rate_per_sec   = TUNE_THROTTLE_UP_RATE_PER_SEC;
    g_tuning.throttle_down_rate_per_sec = TUNE_THROTTLE_DOWN_RATE_PER_SEC;
    g_tuning.motor_idle                 = TUNE_MOTOR_IDLE;
    g_tuning.motor_max                  = TUNE_MOTOR_MAX;
    g_tuning.throttle_cut               = TUNE_THROTTLE_CUT;
    g_tuning.idle_ramp_end              = TUNE_IDLE_RAMP_END;
    g_tuning.pid_ilimit                 = pidRateRoll.iLimit;

    g_tuning.pid_roll_kp                = pidRateRoll.kp;
    g_tuning.pid_roll_ki                = pidRateRoll.ki;
    g_tuning.pid_roll_kd                = pidRateRoll.kd;
    g_tuning.pid_pitch_kp               = pidRatePitch.kp;
    g_tuning.pid_pitch_ki               = pidRatePitch.ki;
    g_tuning.pid_pitch_kd               = pidRatePitch.kd;
    g_tuning.pid_yaw_kp                 = pidRateYaw.kp;
    g_tuning.pid_yaw_ki                 = pidRateYaw.ki;
    g_tuning.pid_yaw_kd                 = pidRateYaw.kd;
    g_tuning.pid_roll_ff                = TUNE_RATE_ROLL_FF;
    g_tuning.pid_pitch_ff               = TUNE_RATE_PITCH_FF;
    g_tuning.pid_yaw_ff                 = TUNE_RATE_YAW_FF;
    g_tuning.pid_roll_d_lpf_hz          = TUNE_RATE_ROLL_D_LPF_HZ;
    g_tuning.pid_pitch_d_lpf_hz         = TUNE_RATE_PITCH_D_LPF_HZ;
    g_tuning.pid_yaw_d_lpf_hz           = TUNE_RATE_YAW_D_LPF_HZ;

    g_tuning.pid_angle_roll_kp          = pidAngleRoll.kp;
    g_tuning.pid_angle_roll_ki          = pidAngleRoll.ki;
    g_tuning.pid_angle_roll_kd          = pidAngleRoll.kd;
    g_tuning.pid_angle_pitch_kp         = pidAnglePitch.kp;
    g_tuning.pid_angle_pitch_ki         = pidAnglePitch.ki;
    g_tuning.pid_angle_pitch_kd         = pidAnglePitch.kd;
    g_tuning.pid_angle_yaw_kp           = pidAngleYaw.kp;
    g_tuning.yaw_deadband               = TUNE_YAW_DEADBAND;
    g_tuning.yaw_max_rate_dps           = TUNE_YAW_MAX_RATE_DPS;

    g_tuning.mahony_kp                  = mahony.kp();
    g_tuning.mahony_ki                  = mahony.ki();
    g_tuning.ahrs_filter_mode           = TUNE_AHRS_FILTER_MODE;
    g_tuning.madgwick_beta              = TUNE_MADGWICK_BETA;
    g_tuning.notch_enable               = TUNE_NOTCH_ENABLE;
    g_tuning.notch_freq_hz              = TUNE_NOTCH_FREQ_HZ;
    g_tuning.notch_q                    = TUNE_NOTCH_Q;
    g_tuning.ekf_angle_q                = TUNE_EKF_ANGLE_Q;
    g_tuning.ekf_bias_q                 = TUNE_EKF_BIAS_Q;
    g_tuning.ekf_accel_r                = TUNE_EKF_ACCEL_R;
    g_tuning.ekf_mag_r                  = TUNE_EKF_MAG_R;
    g_tuning.ekf_mag_declination_deg    = TUNE_EKF_MAG_DECL_DEG;
    g_tuning.ekf_mag_yaw_offset_deg     = TUNE_EKF_MAG_YAW_OFF_DEG;
    g_tuning.ekf_mag_yaw_sign           = TUNE_EKF_MAG_YAW_SIGN;
    g_tuning.dirty                      = false;
}
// Copy g_tuning into live PID/AHRS objects. Caller already holds tuningRte.
static void applyTuningToObjectsLocked()
{
    pidRateRoll.kp         = g_tuning.pid_roll_kp;
    pidRateRoll.ki         = g_tuning.pid_roll_ki;
    pidRateRoll.kd         = g_tuning.pid_roll_kd;
    pidRatePitch.kp        = g_tuning.pid_pitch_kp;
    pidRatePitch.ki        = g_tuning.pid_pitch_ki;
    pidRatePitch.kd        = g_tuning.pid_pitch_kd;
    pidRateYaw.kp          = g_tuning.pid_yaw_kp;
    pidRateYaw.ki          = g_tuning.pid_yaw_ki;
    pidRateYaw.kd          = g_tuning.pid_yaw_kd;

    pidAngleRoll.kp        = g_tuning.pid_angle_roll_kp;
    pidAngleRoll.ki        = g_tuning.pid_angle_roll_ki;
    pidAngleRoll.kd        = g_tuning.pid_angle_roll_kd;
    pidAnglePitch.kp       = g_tuning.pid_angle_pitch_kp;
    pidAnglePitch.ki       = g_tuning.pid_angle_pitch_ki;
    pidAnglePitch.kd       = g_tuning.pid_angle_pitch_kd;
    pidAngleYaw.kp         = g_tuning.pid_angle_yaw_kp;

    // Apply the global I-limit to every PID object so the Config tab lever works.
    pidRateRoll.iLimit     = g_tuning.pid_ilimit;
    pidRatePitch.iLimit    = g_tuning.pid_ilimit;
    pidRateYaw.iLimit      = g_tuning.pid_ilimit;
    pidAngleRoll.iLimit    = g_tuning.pid_ilimit;
    pidAnglePitch.iLimit   = g_tuning.pid_ilimit;
    pidAngleYaw.iLimit     = g_tuning.pid_ilimit;

    mahony.setGains(g_tuning.mahony_kp, g_tuning.mahony_ki);
    madgwickAHRS.setBeta(constrain(g_tuning.madgwick_beta, 0.001f, 1.000f));
    g_tuning.ahrs_filter_mode = constrain(g_tuning.ahrs_filter_mode, 0.0f, 2.0f);

    const float notchHz = constrain(g_tuning.notch_freq_hz, 1.0f, NOTCH_SAMPLE_HZ * 0.45f);
    const float notchQ  = constrain(g_tuning.notch_q, 0.5f, 50.0f);
    notchAx.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchAy.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchAz.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGx.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGy.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGz.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);

    attitudeEKF.setProcessNoise(constrain(g_tuning.ekf_angle_q, 0.000001f, 0.050000f),
                                constrain(g_tuning.ekf_bias_q,  0.000000001f, 0.001000f));
    attitudeEKF.setAccelMeasurementNoise(constrain(g_tuning.ekf_accel_r, 0.001f, 2.000f));
    attitudeEKF.setMagMeasurementNoise(constrain(g_tuning.ekf_mag_r, 0.001f, 5.000f));
    attitudeEKF.setMagDeclinationDeg(constrain(g_tuning.ekf_mag_declination_deg, -30.0f, 30.0f));
    attitudeEKF.setMagYawOffsetDeg(constrain(g_tuning.ekf_mag_yaw_offset_deg, -180.0f, 180.0f));
    attitudeEKF.setMagYawSign(g_tuning.ekf_mag_yaw_sign);

    g_tuning.dirty         = false;
    g_tuneApplySeq         = g_tuneRequestSeq;
}

static void applyTuningToObjects()
{
    if (!tuningRte.lock(0)) return;
    applyTuningToObjectsLocked();
    g_notchResetPending = true;
    tuningRte.unlock();
}
// ─────────────────────────────────────────────────────────────
//  Log helpers (push to WiFi ring buffer + Serial)
// ─────────────────────────────────────────────────────────────
static void calLog(const char* msg)  { telemetryWiFi.pushLog(msg); }
static void calLogf(const char* fmt, ...) {
    char buf[WIFI_LOG_LINE_LEN];
    va_list args; va_start(args,fmt); vsnprintf(buf,sizeof(buf),fmt,args); va_end(args);
    telemetryWiFi.pushLog(buf);
}

static const char* calModeName(CalibrationMode mode)
{
    switch (mode) {
        case CalibrationMode::ESC:            return "ESC";
        case CalibrationMode::GYRO_BIAS:      return "GYRO_BIAS";
        case CalibrationMode::ACCEL_6_FACE:   return "ACCEL_6_FACE";
        case CalibrationMode::MAG_MINMAX:     return "MAG_MINMAX";
        case CalibrationMode::IMU_ALL_GUIDED: return "IMU_ALL_GUIDED";
        case CalibrationMode::NONE:
        default:                              return "NONE";
    }
}

static const char* calStateName(CalibrationState state)
{
    switch (state) {
        case CalibrationState::REQUESTED:              return "REQUESTED";
        case CalibrationState::WAITING_FOR_SAFE:       return "WAITING_FOR_SAFE";
        case CalibrationState::WAITING_FOR_STILLNESS:  return "WAITING_FOR_STILLNESS";
        case CalibrationState::WAITING_FOR_USER_STEP:  return "WAITING_FOR_USER_STEP";
        case CalibrationState::COLLECTING:             return "COLLECTING";
        case CalibrationState::COMPUTING:              return "COMPUTING";
        case CalibrationState::SAVING:                 return "SAVING";
        case CalibrationState::DONE:                   return "DONE";
        case CalibrationState::FAILED:                 return "FAILED";
        case CalibrationState::CANCELLED:              return "CANCELLED";
        case CalibrationState::IDLE:
        default:                                       return "IDLE";
    }
}

static const char* calSourceName(CalibrationSource source)
{
    switch (source) {
        case CalibrationSource::WEB: return "WEB";
        case CalibrationSource::RC:  return "RC";
        case CalibrationSource::NONE:
        default:                     return "NONE";
    }
}

// ─────────────────────────────────────────────────────────────
//  Timing JSON provider — /timing endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingJson()
{
    if (!timingRte.lock(pdMS_TO_TICKS(20))) {
        return F("{\"ok\":false,\"error\":\"mutex timeout\"}");
    }

    uint32_t count      = g_timing.count;
    double   periodMean = g_timing.wMean;
    double   periodStd  = (count > 1) ? sqrt(g_timing.wM2 / (count - 1)) : 0.0;
    double   jitterMean = g_timing.jMean;
    double   jitterStd  = (count > 1) ? sqrt(g_timing.jM2 / (count - 1)) : 0.0;
    uint32_t jitterMax  = g_timing.jitterMax;
    uint32_t violations = g_timing.jitterViolations;

    static uint16_t sortBuf[TIMING_BUF_SIZE];
    uint16_t n    = g_timing.bufFull ? TIMING_BUF_SIZE : g_timing.bufHead;
    uint16_t head = g_timing.bufFull ? g_timing.bufHead : 0;
    for (uint16_t i = 0; i < n; i++) {
        uint16_t period = g_timing.buf[(head + i) % TIMING_BUF_SIZE];
        int32_t  jit    = (int32_t)period - (int32_t)TIMING_TARGET_US;
        if (jit < 0) jit = -jit;
        sortBuf[i] = (uint16_t)min((uint32_t)jit, (uint32_t)65535);
    }
    timingRte.unlock();

    for (uint16_t i = 1; i < n; i++) {
        uint16_t key = sortBuf[i]; int j = i - 1;
        while (j >= 0 && sortBuf[j] > key) { sortBuf[j+1]=sortBuf[j]; j--; }
        sortBuf[j+1] = key;
    }

    float p99 = 0.0f;
    if (n > 0) {
        uint16_t idx = (uint16_t)(0.99f * (float)(n - 1));
        p99 = (float)sortBuf[idx];
    }

    const float targetHz = 1000000.0f / (float)TIMING_TARGET_US;
    const float violationRatePct = (count > 0)
                                 ? (100.0f * (float)violations / (float)count)
                                 : 0.0f;

    String j;
    j.reserve(520);
    j += F("{\"ok\":true");
    j += F(",\"loop_name\":\"control\"");
    j += F(",\"target_us\":");      j += String(TIMING_TARGET_US);
    j += F(",\"period_target_us\":"); j += String(TIMING_TARGET_US); // alias for GCS clarity
    j += F(",\"target_hz\":");      j += String(targetHz, 1);
    j += F(",\"violation_thresh_us\":"); j += String(JITTER_VIOLATION_US);
    j += F(",\"count\":");          j += String(count);
    j += F(",\"period_mean_us\":"); j += String((float)periodMean, 2);
    j += F(",\"period_std_us\":");  j += String((float)periodStd,  2);
    j += F(",\"jitter_mean_us\":"); j += String((float)jitterMean, 2);
    j += F(",\"jitter_std_us\":");  j += String((float)jitterStd,  2);
    j += F(",\"jitter_max_us\":");  j += String(jitterMax);
    j += F(",\"jitter_p99_us\":");  j += String(p99, 1);
    j += F(",\"violations\":");     j += String(violations);
    j += F(",\"violation_rate_pct\":"); j += String(violationRatePct, 3);
    j += F(",\"buf_samples\":");    j += String(n);
    j += '}';
    return j;
}

static String provideIdentityJson()
{
    String j;
    j.reserve(260);
    j += F("{\"ok\":true");
    j += F(",\"name\":\"");         j += FW_NAME;             j += '"';
    j += F(",\"version\":\"");      j += FW_VERSION;          j += '"';
    j += F(",\"sourceBranch\":\""); j += FW_SOURCE_BRANCH;    j += '"';
    j += F(",\"sourceCommit\":\""); j += FW_SOURCE_COMMIT;    j += '"';
    j += F(",\"sourceDirty\":");    j += FW_SOURCE_DIRTY ? F("true") : F("false");
    j += F(",\"buildId\":\"");      j += FW_BUILD_ID;         j += '"';
    j += F(",\"buildTime\":\"");    j += FW_BUILD_TIME_ISO;   j += '"';
    j += '}';
    return j;
}

// ─────────────────────────────────────────────────────────────
//  Timing CSV provider — /timing/csv endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingCsv()
{
    if (!timingRte.lock(pdMS_TO_TICKS(20))) {
        return F("error,mutex_timeout\n");
    }
    uint16_t n    = g_timing.bufFull ? TIMING_BUF_SIZE : g_timing.bufHead;
    uint16_t head = g_timing.bufFull ? g_timing.bufHead : 0;

    static uint16_t tmp[TIMING_BUF_SIZE];
    for (uint16_t i = 0; i < n; i++) {
        tmp[i] = g_timing.buf[(head + i) % TIMING_BUF_SIZE];
    }
    timingRte.unlock();

    String csv;
    csv.reserve((size_t)n * 18 + 32);
    csv += F("index,period_us,jitter_us\n");
    for (uint16_t i = 0; i < n; i++) {
        int32_t jit = (int32_t)tmp[i] - TIMING_TARGET_US;
        if (jit < 0) jit = -jit;
        csv += String(i);  csv += ',';
        csv += String(tmp[i]); csv += ',';
        csv += String((uint32_t)jit); csv += '\n';
    }
    return csv;
}

// ─────────────────────────────────────────────────────────────
//  Wi-Fi telemetry provider — /telemetry endpoint
// ─────────────────────────────────────────────────────────────
static const char* flightModeToStr(FlightMode m) {
    switch(m) {
        case FlightMode::DISARMED: return "DISARMED";
        case FlightMode::ANGLE:    return "ANGLE";
        case FlightMode::ACRO:     return "ACRO";
        case FlightMode::FAILSAFE: return "FAILSAFE";
        default:                   return "UNKNOWN";
    }
}

static void updateControlTransitionCounters(const RCCommand& cmd)
{
    const bool armedNow = cmd.valid &&
                          cmd.mode != FlightMode::DISARMED &&
                          cmd.mode != FlightMode::FAILSAFE;
    const uint32_t nowMs = millis();

    if (!g_transitionTrackerInitialized) {
        g_transitionTrackerInitialized = true;
        g_lastObservedMode = cmd.mode;
        g_lastObservedArmed = armedNow;
        return;
    }

    bool resetEvent = false;
    if (cmd.mode != g_lastObservedMode) {
        g_lastObservedMode = cmd.mode;
        g_modeTransitionCount++;
        g_lastModeChangeMs = nowMs;
        resetEvent = true;
    }

    if (armedNow != g_lastObservedArmed) {
        g_lastObservedArmed = armedNow;
        g_armingTransitionCount++;
        g_lastArmChangeMs = nowMs;
        resetEvent = true;
    }

    if (resetEvent) g_pidResetCount++;
}

static bool provideTelemetry(TelemetryPacket& out)
{
    FlightState s;
    if (!flightStateRte.read(s, pdMS_TO_TICKS(15))) return false;

    TuningState t;
    memset(&t, 0, sizeof(t));
    if (tuningRte.read(t, pdMS_TO_TICKS(5))) {
    }

    out.tick      = s.loopCount;
    out.mode      = flightModeToStr(s.rc.mode);
    out.armed     = s.armed;
    out.rc_valid  = s.rc.valid;
    out.imu_valid = s.imuValid;
    out.mag_valid = s.magValid;
    out.ahrs_filter_mode = s.ahrsFilterMode;
    out.ahrs_filter = (s.ahrsFilterMode == 1) ? "Mahony" : ((s.ahrsFilterMode == 2) ? "Madgwick" : "EKF");
    out.roll_deg  = s.roll_deg;  out.pitch_deg = s.pitch_deg; out.yaw_deg = s.yaw_deg;
    out.q0=s.q0; out.q1=s.q1; out.q2=s.q2; out.q3=s.q3;
    out.roll_ctrl_deg  = s.roll_ctrl_deg;
    out.pitch_ctrl_deg = s.pitch_ctrl_deg;
    out.yaw_ctrl_deg   = s.yaw_ctrl_deg;
    out.roll_offset_deg  = s.roll_offset_deg;
    out.pitch_offset_deg = s.pitch_offset_deg;
    out.yaw_offset_deg   = s.yaw_offset_deg;
    out.ax_g=s.ax_g; out.ay_g=s.ay_g; out.az_g=s.az_g;
    out.gx_dps=s.gx_dps; out.gy_dps=s.gy_dps; out.gz_dps=s.gz_dps;
    out.mx_uT=s.mx_uT;   out.my_uT=s.my_uT;   out.mz_uT=s.mz_uT;
    out.imu_temp_c=s.imuTemp_c;
    out.raw_ax_g=s.rawAx_g; out.raw_ay_g=s.rawAy_g; out.raw_az_g=s.rawAz_g;
    out.raw_gx_dps=s.rawGx_dps; out.raw_gy_dps=s.rawGy_dps; out.raw_gz_dps=s.rawGz_dps;
    out.filt_ax_g=s.filtAx_g; out.filt_ay_g=s.filtAy_g; out.filt_az_g=s.filtAz_g;
    out.filt_gx_dps=s.filtGx_dps; out.filt_gy_dps=s.filtGy_dps; out.filt_gz_dps=s.filtGz_dps;
    out.mag_norm_uT=s.magNorm_uT;
    out.ekf_mag_used=s.ekfMagUsed;
    out.ekf_bgx_dps=s.ekfBgx_dps; out.ekf_bgy_dps=s.ekfBgy_dps; out.ekf_bgz_dps=s.ekfBgz_dps;
    out.target_roll_deg=s.targetRollDeg; out.target_pitch_deg=s.targetPitchDeg; out.target_yaw_deg=s.targetYawDeg;
    out.target_roll_rate_dps=s.targetRollRateDps; out.target_pitch_rate_dps=s.targetPitchRateDps; out.target_yaw_rate_dps=s.targetYawRateDps;
    out.roll_rate_error_dps=s.rollRateError_dps; out.pitch_rate_error_dps=s.pitchRateError_dps; out.yaw_rate_error_dps=s.yawRateError_dps;
    out.yaw_error_deg=s.yawError_deg;
    out.motor_saturated=s.motorSaturated;
    out.angle_mode_active=s.angleModeActive; out.acro_mode_active=s.acroModeActive;
    out.rc_ch1_us=s.rc.raw[0]; out.rc_ch2_us=s.rc.raw[1]; out.rc_ch3_us=s.rc.raw[2]; out.rc_ch4_us=s.rc.raw[3]; out.rc_ch5_us=s.rc.raw[4];
    out.rc_ch6_us=s.rc.raw[5]; out.rc_ch7_us=s.rc.raw[6]; out.rc_ch8_us=s.rc.raw[7]; out.rc_ch9_us=s.rc.raw[8]; out.rc_ch10_us=s.rc.raw[9];
    out.rc_failsafe_count=s.rcFailsafeCount;
    out.mode_switch_raw_us=s.modeSwitchRaw_us; out.arm_switch_raw_us=s.armSwitchRaw_us;
    out.aux_tune1_raw_us=s.auxTune1Raw_us; out.aux_tune2_raw_us=s.auxTune2Raw_us;
    out.angle_loop_enabled=s.angleLoopEnabled; out.rate_loop_enabled=s.rateLoopEnabled;
    out.actual_roll_rate_dps=s.actualRollRate_dps;
    out.actual_pitch_rate_dps=s.actualPitchRate_dps;
    out.actual_yaw_rate_dps=s.actualYawRate_dps;
    out.angle_roll_p=s.angleRollP; out.angle_roll_i=s.angleRollI; out.angle_roll_d=s.angleRollD;
    out.angle_pitch_p=s.anglePitchP; out.angle_pitch_i=s.anglePitchI; out.angle_pitch_d=s.anglePitchD;
    out.rate_roll_p=s.rateRollP; out.rate_roll_i=s.rateRollI; out.rate_roll_d=s.rateRollD;
    out.rate_pitch_p=s.ratePitchP; out.rate_pitch_i=s.ratePitchI; out.rate_pitch_d=s.ratePitchD;
    out.rate_yaw_p=s.rateYawP; out.rate_yaw_i=s.rateYawI; out.rate_yaw_d=s.rateYawD;
    out.angle_roll_iterm=s.angleRollIterm; out.angle_pitch_iterm=s.anglePitchIterm;
    out.pid_reset_count=s.pidResetCount;
    out.mode_transition_count=s.modeTransitionCount; out.last_mode_change_ms=s.lastModeChange_ms;
    out.arming_transition_count=s.armingTransitionCount; out.last_arm_change_ms=s.lastArmChange_ms;
    out.throttle_low=s.throttleLow;
    out.control_authority_remaining=s.controlAuthorityRemaining;
    out.roll_output_limited=s.rollOutputLimited;
    out.pitch_output_limited=s.pitchOutputLimited;
    out.yaw_output_limited=s.yawOutputLimited;
    out.rate_output_limited=s.rateOutputLimited;
    out.bmp_vertical_speed_mps=s.bmpVerticalSpeed_mps;

    out.accel_roll_deg  = s.accelRoll_deg;
    out.accel_pitch_deg = s.accelPitch_deg;
    out.gyro_roll_deg   = s.gyroRoll_deg;
    out.gyro_pitch_deg  = s.gyroPitch_deg;
    out.gyro_yaw_deg    = s.gyroYaw_deg;
    out.roll_angle_error_deg  = s.rollAngleError_deg;
    out.pitch_angle_error_deg = s.pitchAngleError_deg;
    out.pid_roll_out  = s.pidRollOut;
    out.pid_pitch_out = s.pidPitchOut;
    out.pid_yaw_out   = s.pidYawOut;
    out.throttle=s.rc.throttle; out.rc_roll=s.rc.roll;
    out.rc_pitch=s.rc.pitch;    out.rc_yaw=s.rc.yaw;
    out.rc_hz=rcReceiver.getFrameRate();
    out.battery_voltage_v=s.batteryVoltage_v;
    out.battery_adc_v=s.batteryAdcVoltage_v;
    out.battery_cell_v=s.batteryCellVoltage_v;
    out.battery_percent=s.batteryPercent;
    out.battery_valid=s.batteryValid;
    out.battery_low=s.batteryLow;
    out.battery_critical=s.batteryCritical;
    out.motor_fl=s.motorFL; out.motor_fr=s.motorFR;
    out.motor_rl=s.motorRL; out.motor_rr=s.motorRR;
    out.motor_fl_pre_sat=s.motorFLPreSat; out.motor_fr_pre_sat=s.motorFRPreSat;
    out.motor_rl_pre_sat=s.motorRLPreSat; out.motor_rr_pre_sat=s.motorRRPreSat;
    out.loop_period_us=s.loopPeriod_us;
    out.loop_jitter_us=s.loopJitter_us;
    out.imu_read_us=s.imuRead_us;
    out.rc_read_us=s.rcRead_us;
    out.ahrs_update_us=s.ahrsUpdate_us;
    out.control_update_us=s.controlUpdate_us;
    out.motor_write_us=s.motorWrite_us;
    out.wifi_service_us=s.wifiService_us;
    out.onboard_log_write_us=s.onboardLogWrite_us;
    out.missed_loop_count=s.missedLoopCount;
    out.cmd_rpm_fl=s.cmdRpmFL; out.cmd_rpm_fr=s.cmdRpmFR;
    out.cmd_rpm_rl=s.cmdRpmRL; out.cmd_rpm_rr=s.cmdRpmRR;
    // Backward-compatible aliases: until ESC RPM telemetry is added, rpm* means commanded/estimated RPM.
    out.rpm_fl=out.cmd_rpm_fl; out.rpm_fr=out.cmd_rpm_fr;
    out.rpm_rl=out.cmd_rpm_rl; out.rpm_rr=out.cmd_rpm_rr;
    out.actual_rpm_fl=s.actualRpmFL; out.actual_rpm_fr=s.actualRpmFR;
    out.actual_rpm_rl=s.actualRpmRL; out.actual_rpm_rr=s.actualRpmRR;
    out.rpm_actual_valid=s.rpmActualValid;
    out.bmp_temp_c=s.bmpTemp_c; out.bmp_pressure_hpa=s.bmpPressure_hpa;
    out.bmp_altitude_m=s.bmpAltitude_m; out.bmp_valid=s.bmpValid;
    out.cpu_core0_pct=s.cpuCore0_pct; out.cpu_core1_pct=s.cpuCore1_pct;
    out.cpu_valid=s.cpuValid;

    out.max_angle_deg              = t.max_angle_deg;
    out.max_rate_dps               = t.max_rate_dps;
    out.max_pitch_rate_dps         = t.max_pitch_rate_dps;
    out.roll_output_limit          = t.roll_output_limit;
    out.pitch_output_limit         = t.pitch_output_limit;
    out.yaw_output_limit           = t.yaw_output_limit;
    out.throttle_expo              = t.throttle_expo;
    out.throttle_up_rate_per_sec   = t.throttle_up_rate_per_sec;
    out.throttle_down_rate_per_sec = t.throttle_down_rate_per_sec;
    out.motor_idle                 = t.motor_idle;
    out.motor_max                  = t.motor_max;
    out.throttle_cut               = t.throttle_cut;
    out.idle_ramp_end              = t.idle_ramp_end;
    out.pid_ilimit                 = t.pid_ilimit;

    out.pid_roll_kp=t.pid_roll_kp;   out.pid_roll_ki=t.pid_roll_ki;   out.pid_roll_kd=t.pid_roll_kd;
    out.pid_pitch_kp=t.pid_pitch_kp; out.pid_pitch_ki=t.pid_pitch_ki; out.pid_pitch_kd=t.pid_pitch_kd;
    out.pid_yaw_kp=t.pid_yaw_kp;     out.pid_yaw_ki=t.pid_yaw_ki;     out.pid_yaw_kd=t.pid_yaw_kd;
    out.pid_roll_ff=t.pid_roll_ff;   out.pid_pitch_ff=t.pid_pitch_ff; out.pid_yaw_ff=t.pid_yaw_ff;
    out.pid_roll_d_lpf_hz=t.pid_roll_d_lpf_hz; out.pid_pitch_d_lpf_hz=t.pid_pitch_d_lpf_hz; out.pid_yaw_d_lpf_hz=t.pid_yaw_d_lpf_hz;
    out.pid_angle_roll_kp=t.pid_angle_roll_kp;
    out.pid_angle_roll_ki=t.pid_angle_roll_ki;
    out.pid_angle_roll_kd=t.pid_angle_roll_kd;
    out.pid_angle_pitch_kp=t.pid_angle_pitch_kp;
    out.pid_angle_pitch_ki=t.pid_angle_pitch_ki;
    out.pid_angle_pitch_kd=t.pid_angle_pitch_kd;
    out.pid_angle_yaw_kp=t.pid_angle_yaw_kp;
    out.yaw_deadband=t.yaw_deadband;
    out.yaw_max_rate_dps=t.yaw_max_rate_dps;
    out.mahony_kp=t.mahony_kp; out.mahony_ki=t.mahony_ki;
    out.ahrs_filter_mode=t.ahrs_filter_mode;
    out.madgwick_beta=t.madgwick_beta;
    out.notch_enable=t.notch_enable;
    out.notch_freq_hz=t.notch_freq_hz;
    out.notch_q=t.notch_q;
    out.ekf_angle_q=t.ekf_angle_q;
    out.ekf_bias_q=t.ekf_bias_q;
    out.ekf_accel_r=t.ekf_accel_r;
    out.ekf_mag_r=t.ekf_mag_r;
    out.ekf_mag_declination_deg=t.ekf_mag_declination_deg;
    out.ekf_mag_yaw_offset_deg=t.ekf_mag_yaw_offset_deg;
    out.ekf_mag_yaw_sign=t.ekf_mag_yaw_sign;
    out.tune_request_seq = g_tuneRequestSeq;
    out.tune_apply_seq   = g_tuneApplySeq;
    out.tune_reject_seq  = g_tuneRejectSeq;
    out.tune_dirty       = t.dirty;
    out.gps_valid       = s.gps.valid;
    out.gps_lat         = s.gps.latitude;
    out.gps_lon         = s.gps.longitude;
    out.gps_altitude_m  = s.gps.altitude_m;
    out.gps_speed_kmh   = s.gps.speed_kmh;
    out.gps_course_deg  = s.gps.course_deg;
    out.gps_satellites  = s.gps.satellites;
    out.gps_hdop        = s.gps.hdop;
    out.gps_fix_quality = s.gps.fix_quality;
    out.gps_hour        = s.gps.hour;
    out.gps_minute      = s.gps.minute;
    out.gps_second      = s.gps.second;
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Wi-Fi tune handler — /tune endpoint (Test 7.5)
// ─────────────────────────────────────────────────────────────
static bool handleTune(const TunePacket& in)
{
    bool armed = false;
    if (flightStateRte.lock(pdMS_TO_TICKS(5))) {
        armed = g_state.armed; flightStateRte.unlock();
    }
    if (armed) {
        g_tuneRejectSeq++;
        calLog("[TUNE] REJECTED — disarm first.");
        return false;
    }

    if (!tuningRte.lock(pdMS_TO_TICKS(10))) {
        g_tuneRejectSeq++;
        calLog("[TUNE] REJECTED — tune mutex timeout.");
        return false;
    }

    if (in.has_max_angle_deg)              g_tuning.max_angle_deg              = constrain(in.max_angle_deg, 5.0f, 80.0f);
    if (in.has_max_rate_dps)               g_tuning.max_rate_dps               = constrain(in.max_rate_dps, 30.0f, 1200.0f);
    if (in.has_max_pitch_rate_dps)         g_tuning.max_pitch_rate_dps         = constrain(in.max_pitch_rate_dps, 30.0f, 1200.0f);

    if (in.has_roll_output_limit)          g_tuning.roll_output_limit          = constrain(in.roll_output_limit, 0.02f, 1.00f);
    if (in.has_pitch_output_limit)         g_tuning.pitch_output_limit         = constrain(in.pitch_output_limit, 0.02f, 1.00f);
    if (in.has_yaw_output_limit)           g_tuning.yaw_output_limit           = constrain(in.yaw_output_limit, 0.01f, 1.00f);

    if (in.has_throttle_expo)              g_tuning.throttle_expo              = constrain(in.throttle_expo, 0.0f, 0.95f);
    if (in.has_throttle_up_rate_per_sec)   g_tuning.throttle_up_rate_per_sec   = constrain(in.throttle_up_rate_per_sec, 0.05f, 10.0f);
    if (in.has_throttle_down_rate_per_sec) g_tuning.throttle_down_rate_per_sec = constrain(in.throttle_down_rate_per_sec, 0.05f, 10.0f);
    if (in.has_motor_idle)                 g_tuning.motor_idle                 = constrain(in.motor_idle, 0.0f, 0.40f);
    if (in.has_motor_max)                  g_tuning.motor_max                  = constrain(in.motor_max, 0.10f, 1.00f);
    if (in.has_throttle_cut)               g_tuning.throttle_cut               = constrain(in.throttle_cut, 0.0f, 0.30f);
    if (in.has_idle_ramp_end)              g_tuning.idle_ramp_end              = constrain(in.idle_ramp_end, 0.01f, 0.60f);
    if (in.has_pid_ilimit)                 g_tuning.pid_ilimit                 = constrain(in.pid_ilimit, 0.0f, 1000.0f);

    // Keep idle/ramp relationships sane.
    if (g_tuning.motor_idle > g_tuning.motor_max) g_tuning.motor_idle = g_tuning.motor_max;
    if (g_tuning.idle_ramp_end <= g_tuning.throttle_cut) g_tuning.idle_ramp_end = g_tuning.throttle_cut + 0.01f;

    if (in.has_pid_roll_kp)        g_tuning.pid_roll_kp        = in.pid_roll_kp;
    if (in.has_pid_roll_ki)        g_tuning.pid_roll_ki        = in.pid_roll_ki;
    if (in.has_pid_roll_kd)        g_tuning.pid_roll_kd        = in.pid_roll_kd;
    if (in.has_pid_pitch_kp)       g_tuning.pid_pitch_kp       = in.pid_pitch_kp;
    if (in.has_pid_pitch_ki)       g_tuning.pid_pitch_ki       = in.pid_pitch_ki;
    if (in.has_pid_pitch_kd)       g_tuning.pid_pitch_kd       = in.pid_pitch_kd;
    if (in.has_pid_yaw_kp)         g_tuning.pid_yaw_kp         = in.pid_yaw_kp;
    if (in.has_pid_yaw_ki)         g_tuning.pid_yaw_ki         = in.pid_yaw_ki;
    if (in.has_pid_yaw_kd)         g_tuning.pid_yaw_kd         = in.pid_yaw_kd;
    if (in.has_pid_roll_ff)        g_tuning.pid_roll_ff        = in.pid_roll_ff;
    if (in.has_pid_pitch_ff)       g_tuning.pid_pitch_ff       = in.pid_pitch_ff;
    if (in.has_pid_yaw_ff)         g_tuning.pid_yaw_ff         = in.pid_yaw_ff;
    if (in.has_pid_roll_d_lpf_hz)  g_tuning.pid_roll_d_lpf_hz  = constrain(in.pid_roll_d_lpf_hz, 0.0f, 200.0f);
    if (in.has_pid_pitch_d_lpf_hz) g_tuning.pid_pitch_d_lpf_hz = constrain(in.pid_pitch_d_lpf_hz, 0.0f, 200.0f);
    if (in.has_pid_yaw_d_lpf_hz)   g_tuning.pid_yaw_d_lpf_hz   = constrain(in.pid_yaw_d_lpf_hz, 0.0f, 200.0f);
    if (in.has_pid_angle_roll_kp)  g_tuning.pid_angle_roll_kp  = in.pid_angle_roll_kp;
    if (in.has_pid_angle_roll_ki)  g_tuning.pid_angle_roll_ki  = in.pid_angle_roll_ki;
    if (in.has_pid_angle_roll_kd)  g_tuning.pid_angle_roll_kd  = in.pid_angle_roll_kd;
    if (in.has_pid_angle_pitch_kp) g_tuning.pid_angle_pitch_kp = in.pid_angle_pitch_kp;
    if (in.has_pid_angle_pitch_ki) g_tuning.pid_angle_pitch_ki = in.pid_angle_pitch_ki;
    if (in.has_pid_angle_pitch_kd) g_tuning.pid_angle_pitch_kd = in.pid_angle_pitch_kd;
    if (in.has_pid_angle_yaw_kp)   g_tuning.pid_angle_yaw_kp   = in.pid_angle_yaw_kp;
    if (in.has_yaw_deadband)       g_tuning.yaw_deadband       = constrain(in.yaw_deadband, 0.0f, 0.50f);
    if (in.has_yaw_max_rate_dps)   g_tuning.yaw_max_rate_dps   = constrain(in.yaw_max_rate_dps, 10.0f, 500.0f);
    if (in.has_mahony_kp)          g_tuning.mahony_kp          = in.mahony_kp;
    if (in.has_mahony_ki)          g_tuning.mahony_ki          = in.mahony_ki;
    if (in.has_ahrs_filter_mode)   g_tuning.ahrs_filter_mode   = constrain(in.ahrs_filter_mode, 0.0f, 2.0f);
    if (in.has_madgwick_beta)      g_tuning.madgwick_beta      = constrain(in.madgwick_beta, 0.001f, 1.000f);
    if (in.has_notch_enable)       g_tuning.notch_enable       = in.notch_enable;
    if (in.has_notch_freq_hz)      g_tuning.notch_freq_hz      = constrain(in.notch_freq_hz, 1.0f, NOTCH_SAMPLE_HZ * 0.45f);
    if (in.has_notch_q)            g_tuning.notch_q            = constrain(in.notch_q, 0.5f, 50.0f);

    if (in.has_ekf_angle_q)              g_tuning.ekf_angle_q             = constrain(in.ekf_angle_q, 0.000001f, 0.050000f);
    if (in.has_ekf_bias_q)               g_tuning.ekf_bias_q              = constrain(in.ekf_bias_q, 0.000000001f, 0.001000f);
    if (in.has_ekf_accel_r)              g_tuning.ekf_accel_r             = constrain(in.ekf_accel_r, 0.001f, 2.000f);
    if (in.has_ekf_mag_r)                g_tuning.ekf_mag_r               = constrain(in.ekf_mag_r, 0.001f, 5.000f);
    if (in.has_ekf_mag_declination_deg)  g_tuning.ekf_mag_declination_deg = constrain(in.ekf_mag_declination_deg, -30.0f, 30.0f);
    if (in.has_ekf_mag_yaw_offset_deg)   g_tuning.ekf_mag_yaw_offset_deg  = constrain(in.ekf_mag_yaw_offset_deg, -180.0f, 180.0f);
    if (in.has_ekf_mag_yaw_sign)         g_tuning.ekf_mag_yaw_sign        = (in.ekf_mag_yaw_sign < 0.0f) ? -1.0f : 1.0f;
    // Accept and apply immediately while disarmed. This removes the confusing
    // one-cycle handshake where the POST succeeded but telemetry still showed
    // old values until taskControl got around to applying dirty tuning.
    g_tuneRequestSeq++;
    g_tuning.dirty = true;
    applyTuningToObjectsLocked();
    g_notchResetPending = true;
    const bool persisted = g_tuningStore.save(g_tuning);
    uint32_t appliedSeq = g_tuneApplySeq;
    tuningRte.unlock();

    if (!persisted) {
        DBG_PRINTF("[TUNE] Applied seq=%lu but NVS save verification FAILED.\n",
                   (unsigned long)appliedSeq);
        calLogf("[TUNE][ERROR] Applied seq=%lu but permanent save failed.",
                (unsigned long)appliedSeq);
        return false;
    }
    DBG_PRINTF("[TUNE] Applied and saved to NVS seq=%lu.\n", (unsigned long)appliedSeq);
    calLogf("[TUNE] Applied and saved permanently seq=%lu.", (unsigned long)appliedSeq);
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Motor + SWC helpers
// ─────────────────────────────────────────────────────────────
static void motorsBegin()                                    { motorBegin(); }
static void writeMotors(float fl,float fr,float rl,float rr) { motorSet(fl,fr,rl,rr); }
static void motorsOff()                                      { motorOff(); }
static bool swcIsUp()     { return rcReceiver.getChannel(RC_CH_AUX3) >= SWC_THRESHOLD; }
static void waitSwcDown() { while (swcIsUp()) delay(20); }
static void silentWait(uint32_t ms) {
    uint32_t t = millis();
    while (millis()-t < ms) delay(50);
}

// ═════════════════════════════════════════════════════════════
//  AUTONOMOUS CALIBRATION (runs inside taskControl, Core 1)
// ═════════════════════════════════════════════════════════════
static void runAutonomousCalibration()
{
    calLog("[CAL] ═══ AUTONOMOUS CALIBRATION STARTED ═══");

    if (!SelectedImu::supportsManualCalibration()) {
        calLog("[CAL] Manual calibration is not used by this IMU backend.");
        calLog("[CAL] Follow the BNO085 calibration procedure and save DCD in the sensor adapter.");
        g_calibState = CalibState::DONE;
        return;
    }

    // ── Stage 1: Gyro ────────────────────────────────────────
    g_calibState = CalibState::RUNNING_GYRO;
    calLogf("[CAL] 1/3 GYRO — flat + still. Settle %ds, sample %ds...",
            GYRO_SETTLE_MS/1000, GYRO_SAMPLE_MS/1000);
    silentWait(GYRO_SETTLE_MS);
    {
        ImuSensorData avg;
        imu.sampleAvg(GYRO_SAMPLE_MS / 2, avg);
        imu.cal.gx_b = avg.gx_dps;
        imu.cal.gy_b = avg.gy_dps;
        imu.cal.gz_b = avg.gz_dps;
        calLogf("[CAL] Gyro bias X=%+.4f Y=%+.4f Z=%+.4f dps",
                imu.cal.gx_b, imu.cal.gy_b, imu.cal.gz_b);
    }
    calLog("[CAL] ✓ GYRO done.");

    // ── Stage 2: Accel ───────────────────────────────────────
    g_calibState = CalibState::RUNNING_ACCEL;

    struct AccelPose {
        const char* physicalPose;
        uint8_t axis;
        int8_t sign;
    };

    const AccelPose poses[6] = {
        {"NOSE UP",        0, +1},
        {"NOSE DOWN",      0, -1},
        {"LEFT side UP",   1, +1},
        {"RIGHT side UP",  1, -1},
        {"FLAT top up",    2, +1},
        {"UPSIDE DOWN",    2, -1}
    };

    float plusVal[3]  = {0.0f, 0.0f, 0.0f};
    float minusVal[3] = {0.0f, 0.0f, 0.0f};
    bool gotPlus[3]   = {false, false, false};
    bool gotMinus[3]  = {false, false, false};

    calLog("[CAL] 2/3 ACCEL — 6 physical positions, flip SWC UP to confirm each.");

    for (int p = 0; p < 6; p++) {
        if (swcIsUp()) {
            calLog("[CAL] Flip SWC DOWN first...");
            waitSwcDown();
        }

        calLogf("[CAL] Pos %d/6: Put drone: %s — Flip SWC UP when steady",
                p + 1, poses[p].physicalPose);

        uint32_t t0 = millis();
        while (!swcIsUp()) {
            if (millis() - t0 > ACCEL_WAIT_MAX_MS) {
                calLog("[CAL] Timeout — position skipped");
                break;
            }
            delay(20);
        }

        if (!swcIsUp()) continue;

        silentWait(ACCEL_HOLD_MS);

        ImuSensorData avg;
        imu.sampleAvg(ACCEL_HOLD_MS / 2, avg);

        float axisValue[3] = { avg.ax_g, avg.ay_g, avg.az_g };

        uint8_t axis = poses[p].axis;
        int8_t sign  = poses[p].sign;
        float value  = axisValue[axis];

        calLogf("[CAL] Got ax=%+.4f ay=%+.4f az=%+.4f g",
                avg.ax_g, avg.ay_g, avg.az_g);

        calLogf("[CAL] Routed %s to sensor %c%c = %+.4f g",
                poses[p].physicalPose,
                sign > 0 ? '+' : '-',
                axis == 0 ? 'X' : axis == 1 ? 'Y' : 'Z',
                value);

        if ((sign > 0 && value < 0.5f) || (sign < 0 && value > -0.5f)) {
            calLog("[CAL][WARN] Axis sign does not match expected pose. Check orientation.");
        }

        if (sign > 0) {
            plusVal[axis] = value;
            gotPlus[axis] = true;
        } else {
            minusVal[axis] = value;
            gotMinus[axis] = true;
        }

        waitSwcDown();
    }

    bool accelOk = true;
    for (int a = 0; a < 3; a++) {
        if (!gotPlus[a] || !gotMinus[a]) {
            accelOk = false;
        }
    }

    if (!accelOk) {
        calLog("[CAL][ERROR] Accel calibration incomplete — keeping old accel calibration.");
    } else {
        imu.cal.ax_b = (plusVal[0] + minusVal[0]) / 2.0f;
        imu.cal.ay_b = (plusVal[1] + minusVal[1]) / 2.0f;
        imu.cal.az_b = (plusVal[2] + minusVal[2]) / 2.0f;

        float hx = (plusVal[0] - minusVal[0]) / 2.0f;
        float hy = (plusVal[1] - minusVal[1]) / 2.0f;
        float hz = (plusVal[2] - minusVal[2]) / 2.0f;

        imu.cal.ax_s = fabsf(hx) > 0.01f ? 1.0f / hx : 1.0f;
        imu.cal.ay_s = fabsf(hy) > 0.01f ? 1.0f / hy : 1.0f;
        imu.cal.az_s = fabsf(hz) > 0.01f ? 1.0f / hz : 1.0f;

        calLogf("[CAL] Accel bias ax=%+.4f ay=%+.4f az=%+.4f g",
                imu.cal.ax_b, imu.cal.ay_b, imu.cal.az_b);

        calLogf("[CAL] Accel scale ax=%+.4f ay=%+.4f az=%+.4f",
                imu.cal.ax_s, imu.cal.ay_s, imu.cal.az_s);

        calLog("[CAL] ✓ ACCEL done.");
    }

    // ── Stage 3: Mag (skipped automatically if no AK8963) ────
    g_calibState = CalibState::RUNNING_MAG;
    if (imu.hasMag()) {
        calLogf("[CAL] 3/3 MAG — rotate figure-8 for %ds", MAG_DURATION_MS/1000);
        float xn=1e9f,yn=1e9f,zn=1e9f,xx=-1e9f,yx=-1e9f,zx=-1e9f;
        uint32_t end=millis()+MAG_DURATION_MS, lp=0;
        while (millis() < end) {
            ImuSensorData s;
            if (imu.readScaled(s)) {
                if (fabsf(s.mx_uT)>0.1f || fabsf(s.my_uT)>0.1f || fabsf(s.mz_uT)>0.1f) {
                    if(s.mx_uT<xn)xn=s.mx_uT; if(s.mx_uT>xx)xx=s.mx_uT;
                    if(s.my_uT<yn)yn=s.my_uT; if(s.my_uT>yx)yx=s.my_uT;
                    if(s.mz_uT<zn)zn=s.mz_uT; if(s.mz_uT>zx)zx=s.mz_uT;
                }
            }
            if (millis()-lp >= 10000) {
                calLogf("[CAL] Mag — %lus remaining", (unsigned long)((end-millis())/1000));
                lp=millis();
            }
            delay(10);
        }
        imu.cal.mx_b=(xx+xn)/2.0f; imu.cal.my_b=(yx+yn)/2.0f; imu.cal.mz_b=(zx+zn)/2.0f;
        float sp=((xx-xn)+(yx-yn)+(zx-zn))/3.0f;
        imu.cal.mx_s=(xx-xn)>0.1f?sp/(xx-xn):1.0f;
        imu.cal.my_s=(yx-yn)>0.1f?sp/(yx-yn):1.0f;
        imu.cal.mz_s=(zx-zn)>0.1f?sp/(zx-zn):1.0f;
        calLog("[CAL] ✓ MAG done.");
    } else {
        calLog("[CAL] 3/3 MAG — SKIPPED (no AK8963 on this board).");
        calLog("[CAL] AHRS running 6-DOF mode — roll+pitch accurate, yaw drifts.");
    }

    // ── Save to NVS flash ─────────────────────────────────────
    g_calibState = CalibState::SAVING;
    imu.cal.valid = true;
    imu.saveCalibration();
    calLog("[CAL] ✓ Calibration COMPLETE — saved to NVS.");
    g_calibState = CalibState::DONE;
}

// ═════════════════════════════════════════════════════════════
//  TASKS
// ═════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
//  taskGPS — Core 0, priority 1, 50 Hz
// ─────────────────────────────────────────────────────────────
static void taskGPS(void* /*pv*/)
{
    gpsServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskRC — Core 0, priority 3
//  Important: do NOT use vTaskDelayUntil(5 ms) here if the FreeRTOS tick
//  is coarse. On this build, one tick is behaving like ~40 ms, which can
//  starve the iBUS parser. Use micros() pacing + taskYIELD() instead.
// ─────────────────────────────────────────────────────────────
static void taskRC(void* /*pv*/)
{
    static ReceiverServiceTask receiverTask(
        rcReceiver, calManager, calLog,
        ESC_CALIB_VRB_THRESHOLD, TUNE_THROTTLE_CUT);
    receiverTask.run();
}

static void computeControlAttitude(const AttitudeEstimate& att,
                                   float& rollCtrl,
                                   float& pitchCtrl,
                                   float& yawCtrl)
{
    rollCtrl  = levelTrim.controlRollDeg(att.roll_deg);
    pitchCtrl = levelTrim.controlPitchDeg(att.pitch_deg);
    yawCtrl   = levelTrim.controlYawDeg(att.yaw_deg);
}

static LowPassFilter lpfGx, lpfGy, lpfGz;

// Diagnostic-only attitude estimates published to telemetry.
// These are not used for control; they help compare AHRS fusion vs raw accel
// tilt and gyro-only integration on the ground station.
static float g_gyroRollDeg  = 0.0f;
static float g_gyroPitchDeg = 0.0f;
static float g_gyroYawDeg   = 0.0f;
static bool  g_gyroAngleInit = false;

// Tracks whether we have already commanded the ESC outputs off.
// motorOff() is intentionally NOT called every 400 Hz cycle because the
// current MotorControl implementation appears to block for ~40 ms.
static bool  g_motorOutputsActive = false;

static void updateDynamicNotchFromFFT()
{
    if (!TUNE_DYNAMIC_NOTCH_ENABLE) return;

    static uint32_t lastUpdateMs = 0;
    uint32_t nowMs = millis();

    if (nowMs - lastUpdateMs < DYN_NOTCH_UPDATE_MS) {
        return;
    }

    lastUpdateMs = nowMs;

    // Snapshot flight state inside this function.
    // This avoids Arduino prototype issues with FlightState in the function argument.
    FlightState s;
    if (!flightStateRte.read(s, pdMS_TO_TICKS(5))) {
        return;
    }

    // Do not learn while disarmed, motors off, failsafe, or low throttle.
    if (!s.armed ||
        !g_motorOutputsActive ||
        !s.rc.valid ||
        s.rc.throttle < DYN_NOTCH_MIN_THROTTLE) {

        if (LOG_DYNAMIC_NOTCH_DEBUG) {
            static uint32_t lastGatePrintMs = 0;
            if (nowMs - lastGatePrintMs >= 1000) {
                lastGatePrintMs = nowMs;

                DBG_PRINTF("[DYN_NOTCH_GATE] blocked armed=%d motorOut=%d rcValid=%d throttle=%.3f minThrottle=%.3f\n",
                              s.armed ? 1 : 0,
                              g_motorOutputsActive ? 1 : 0,
                              s.rc.valid ? 1 : 0,
                              s.rc.throttle,
                              DYN_NOTCH_MIN_THROTTLE);
            }
        }

        return;
    }

    bool notchEnabled = false;
    float currentHz = TUNE_NOTCH_FREQ_HZ;

    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        notchEnabled = g_tuning.notch_enable;
        currentHz = g_tuning.notch_freq_hz;
        tuningRte.unlock();
    } else {
        return;
    }

    if (!notchEnabled) {
        return;
    }

    float peakHz = 0.0f;
    float peakScore = 0.0f;
    uint32_t seq = 0;

    bool ok = spectrumAnalyzer.findGyroPeak(DYN_NOTCH_MIN_HZ,
                                            DYN_NOTCH_MAX_HZ,
                                            DYN_NOTCH_MIN_SCORE,
                                            peakHz,
                                            peakScore,
                                            seq);

    if (!ok) {
        if (LOG_DYNAMIC_NOTCH_DEBUG) {
            static uint32_t lastNoPeakPrintMs = 0;
            if (nowMs - lastNoPeakPrintMs >= 1000) {
                lastNoPeakPrintMs = nowMs;

                DBG_PRINTF("[DYN_NOTCH_NO_PEAK] current=%.1fHz min=%.1f max=%.1f minScore=%.2f seq=%lu\n",
                              currentHz,
                              DYN_NOTCH_MIN_HZ,
                              DYN_NOTCH_MAX_HZ,
                              DYN_NOTCH_MIN_SCORE,
                              (unsigned long)seq);
            }
        }

        return;
    }

    // Smooth toward the detected FFT peak.
    float targetHz = currentHz + DYN_NOTCH_ALPHA * (peakHz - currentHz);

    // Limit movement per update so the notch does not chase noise.
    float stepHz = targetHz - currentHz;
    stepHz = constrain(stepHz, -DYN_NOTCH_MAX_STEP_HZ, DYN_NOTCH_MAX_STEP_HZ);

    float requestedHz = currentHz + stepHz;
    requestedHz = constrain(requestedHz, DYN_NOTCH_MIN_HZ, DYN_NOTCH_MAX_HZ);

    // Avoid tiny reconfigurations.
    if (fabsf(requestedHz - currentHz) < 0.5f) {
        return;
    }

    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        g_tuning.notch_freq_hz = requestedHz;
        g_tuning.dirty = true;   // taskControl applies notch safely
        tuningRte.unlock();
    }

    if (LOG_DYNAMIC_NOTCH_DEBUG) {
        DBG_PRINTF("[DYN_NOTCH] peak=%.1fHz score=%.3f current=%.1fHz -> request=%.1fHz seq=%lu\n",
                      peakHz,
                      peakScore,
                      currentHz,
                      requestedHz,
                      (unsigned long)seq);
    }
}

// ═════════════════════════════════════════════════════════════
//  HIGH-SPEED ON-BOARD FLIGHT LOG  (Test 7.1 / 7.3 / 7.4 capture)
//
//  Expanded debug row, logged at 100 Hz (every 4th 400 Hz cycle).
//  Workflow: POST /flightlog/reset  →  arm + fly  →  disarm  →
//            GET /flightlog/csv (freezes the buffer, streams it out).
//
//  FLIGHT_LOG_SIZE=300 → ~3.0 s at 100 Hz with extended fields. Raise only if
//  free heap allows (check the 'I'-style resource report first).
//
//  Concurrency: writer (taskControl, Core 1) and the freeze flag
//  share one portMUX spinlock. GET freezes logging first, so the
//  WiFi task (Core 0) reads a static buffer with no further writes
//  — no second copy buffer needed, and rows are streamed (chunked)
//  so no giant CSV String is ever built in heap.
// ═════════════════════════════════════════════════════════════

static void resetFlightLog() {
    flightLogger.reset();
}

static uint16_t flightLogCount() {
    return flightLogger.countAndFreeze();
}

static String flightLogHeader() {
    return flightLogger.csvHeader();
}

static String flightLogRowCsv(uint16_t i) {
    return flightLogger.csvRow(i);
}




// ─────────────────────────────────────────────────────────────
//  taskControl — Core 1, priority 5, 400 Hz  (2.5 ms period)
// ─────────────────────────────────────────────────────────────
// ============================================================================
// BLOCK B — Replace your taskControl() with this version
// ============================================================================

static void taskControl(void* /*pv*/)
{
    const uint32_t TARGET_US = TIMING_TARGET_US;   // 2500 us = 400 Hz

    // The loop is released by esp_timer every 2500 us.
    // Do not use vTaskDelay(1) here, and do not busy-wait: both caused
    // either 40 ms periods or watchdog resets on this build.
    uint32_t lastUs = 0;

    pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
    pidAngleRoll.reset(); pidAnglePitch.reset();
    pidAngleYaw.reset();
    attitudeEstimator.resetEkf();
    flightController.reset();

    for (;;) {
        if (g_tuning.dirty) applyTuningToObjects();

        if (g_calibState == CalibState::REQUESTED) {
            if (g_motorOutputsActive) {
                motorsOff();
                g_motorOutputsActive = false;
            }
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            pidAngleYaw.reset();
            attitudeEstimator.resetEkf();
            flightController.reset();
            runAutonomousCalibration();
            g_calibState = CalibState::IDLE;
            ulTaskNotifyTake(pdTRUE, 0);  // discard stale timer releases
            lastUs = 0;
            continue;
        }

        // Block until the high-resolution 400 Hz timer releases this task.
        // pdTRUE clears any accumulated notifications, so if a previous cycle
        // ran long we skip stale releases instead of trying to catch up.
        uint32_t releases = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (releases > 1) {
            g_execTiming.missedTimerReleases += (releases - 1);
        }

        uint32_t nowUs = micros();
        uint32_t periodUs = (lastUs == 0) ? TARGET_US : (nowUs - lastUs);
        lastUs = nowUs;

        // Execution timer starts AFTER the period wait.
        // This measures actual control work, not intentional wait time.
        uint32_t execStartUs = micros();

        float dt = (float)periodUs * 1e-6f;
        if (dt <= 0.0f || dt > 0.05f) {
            dt = (float)TARGET_US * 1e-6f;
            periodUs = TARGET_US;
        }

        batteryMonitor.update(millis());
        const BatteryStatus bat = batteryMonitor.status();
        const float batteryVoltageForRpm = bat.valid ? bat.batteryVoltage_v : BATTERY_NOMINAL_V;

        // ── Welford timing accumulator (Test 7.1) ────────────
        if (timingRte.lock(0)) {
            uint32_t n = ++g_timing.count;
            double delta  = (double)periodUs - g_timing.wMean;
            g_timing.wMean += delta / n;
            g_timing.wM2   += delta * ((double)periodUs - g_timing.wMean);

            uint32_t jit = (periodUs >= TARGET_US)
                         ? (periodUs - TARGET_US)
                         : (TARGET_US - periodUs);

            double jdelta  = (double)jit - g_timing.jMean;
            g_timing.jMean += jdelta / n;
            g_timing.jM2   += jdelta * ((double)jit - g_timing.jMean);

            if (jit > g_timing.jitterMax)  g_timing.jitterMax = jit;
            if (jit > JITTER_VIOLATION_US) g_timing.jitterViolations++;

            g_timing.buf[g_timing.bufHead] = (uint16_t)min(periodUs, (uint32_t)65535);
            g_timing.bufHead = (g_timing.bufHead + 1) % TIMING_BUF_SIZE;
            if (g_timing.bufHead == 0) g_timing.bufFull = true;

            timingRte.unlock();
        }

        // ── Calibration active: keep motors off and skip flight control ──
        if (FlightSafetyPolicy::controlMustStop(
                false, false, calManager.shouldBlockFlight())) {
            if (!calManager.ownsMotors() && g_motorOutputsActive) {
                motorsOff();
                g_motorOutputsActive = false;
            }

            pidRateRoll.reset();
            pidRatePitch.reset();
            pidRateYaw.reset();

            pidAngleRoll.reset();
            pidAnglePitch.reset();
            pidAngleYaw.reset();

            flightController.reset();

            updateExecTimingAndPrint(0, 0, periodUs, TARGET_US);

            continue;
        }

        // ── IMU read + EKF AHRS ───────────────────────────────
        ImuSensorData s;
        AttitudeEstimate   att;
        uint32_t phaseStartUs = micros();
        bool imuOk = imu.readScaled(s);
        g_execTiming.lastImuUs = micros() - phaseStartUs;

        float gxf = 0, gyf = 0, gzf = 0;
        float filtAx = 0.0f, filtAy = 0.0f, filtAz = 0.0f;
        float accelRollDeg = 0.0f, accelPitchDeg = 0.0f;
        float rollAngleErrDeg = 0.0f, pitchAngleErrDeg = 0.0f;
        if (imuOk) {
            if (g_notchResetPending) {
                notchAx.reset(s.ax_g);
                notchAy.reset(s.ay_g);
                notchAz.reset(s.az_g);
                notchGx.reset(s.gx_dps);
                notchGy.reset(s.gy_dps);
                notchGz.reset(s.gz_dps);
                g_notchResetPending = false;
            }

            // Apply tunable motor-vibration notch before EKF and before PID.
            // Keep original `s` for raw telemetry/logging; use `sf` for control.
            ImuSensorData sf = s;
            sf.ax_g   = notchAx.apply(s.ax_g);
            sf.ay_g   = notchAy.apply(s.ay_g);
            sf.az_g   = notchAz.apply(s.az_g);
            sf.gx_dps = notchGx.apply(s.gx_dps);
            sf.gy_dps = notchGy.apply(s.gy_dps);
            sf.gz_dps = notchGz.apply(s.gz_dps);
            filtAx = sf.ax_g; filtAy = sf.ay_g; filtAz = sf.az_g;

            // Feed onboard vibration spectrum with RAW pre-notch IMU data.
            // This lets you see the vibration peak even when the notch is enabled.
            spectrumAnalyzer.push(s.ax_g, s.ay_g, s.az_g,
                                  s.gx_dps, s.gy_dps, s.gz_dps,
                                  g_motorOutputsActive);

            if (LOG_DYNAMIC_NOTCH_DEBUG) {
                static uint32_t dynFftPushCount = 0;
                static uint32_t lastDynFftInputPrintMs = 0;

                dynFftPushCount++;

                uint32_t nowFftInputMs = millis();
                if (nowFftInputMs - lastDynFftInputPrintMs >= 1000) {
                    lastDynFftInputPrintMs = nowFftInputMs;

                    float gyroMag =
                        sqrtf(s.gx_dps * s.gx_dps +
                              s.gy_dps * s.gy_dps +
                              s.gz_dps * s.gz_dps);

                    float accelVib =
                        fabsf(sqrtf(s.ax_g * s.ax_g +
                                    s.ay_g * s.ay_g +
                                    s.az_g * s.az_g) - 1.0f);

                    DBG_PRINTF("[DYN_FFT_INPUT] pushes=%lu motorOut=%d gyroMag=%.3f dps accelVib=%.4f g\n",
                                  (unsigned long)dynFftPushCount,
                                  g_motorOutputsActive ? 1 : 0,
                                  gyroMag,
                                  accelVib);
                    DBG_PRINTF("[DYN_NOTCH_HOOK] motorOut=%d notchEnable=%d notchHz=%.1f Q=%.1f\n",
                                  g_motorOutputsActive ? 1 : 0,
                                  g_tuning.notch_enable ? 1 : 0,
                                  g_tuning.notch_freq_hz,
                                  g_tuning.notch_q);
                }
            }
            AHRSInput ekfIn;
            ekfIn.ax_g = sf.ax_g; ekfIn.ay_g = sf.ay_g; ekfIn.az_g = sf.az_g;
            ekfIn.gx_dps = sf.gx_dps; ekfIn.gy_dps = sf.gy_dps; ekfIn.gz_dps = sf.gz_dps;
            ekfIn.mx_uT = s.mx_uT; ekfIn.my_uT = s.my_uT; ekfIn.mz_uT = s.mz_uT;
            ekfIn.magValid = imu.isMagConnected();

            // Runtime-selectable attitude estimator SWC.
            uint8_t ahrsMode = 0;
            if (tuningRte.lock(0)) {
                ahrsMode = (uint8_t)constrain((int)roundf(g_tuning.ahrs_filter_mode), 0, 2);
                tuningRte.unlock();
            }
            uint32_t ahrsStartUs = micros();
            att = attitudeEstimator.update(ekfIn, dt, ahrsMode);
            g_ahrsFilterModeActive = attitudeEstimator.activeMode();
            g_execTiming.lastAhrsUs = micros() - ahrsStartUs;

            gxf = lpfGx.update(sf.gx_dps, dt, GYRO_LPF_HZ);
            gyf = lpfGy.update(sf.gy_dps, dt, GYRO_LPF_HZ);
            gzf = lpfGz.update(sf.gz_dps, dt, GYRO_LPF_HZ);

            // Accel-only tilt estimate. This is noisy under vibration/acceleration,
            // but useful as an independent reference for AHRS roll/pitch.
            const float RAD_TO_DEG_LOCAL = 57.29577951308232f;
            accelRollDeg  = atan2f(s.ay_g, s.az_g) * RAD_TO_DEG_LOCAL;
            accelPitchDeg = atan2f(-s.ax_g, sqrtf(s.ay_g*s.ay_g + s.az_g*s.az_g)) * RAD_TO_DEG_LOCAL;

            // Gyro-only integration. This will drift; that drift is exactly what
            // the AHRS correction is meant to control.
            if (!g_gyroAngleInit) {
                g_gyroRollDeg  = att.roll_deg;
                g_gyroPitchDeg = att.pitch_deg;
                g_gyroYawDeg   = att.yaw_deg;
                g_gyroAngleInit = true;
            } else {
                g_gyroRollDeg  += gxf * dt;
                g_gyroPitchDeg += gyf * dt;
                g_gyroYawDeg   += gzf * dt;
                while (g_gyroYawDeg >= 360.0f) g_gyroYawDeg -= 360.0f;
                while (g_gyroYawDeg <    0.0f) g_gyroYawDeg += 360.0f;
            }

            rollAngleErrDeg  = att.roll_deg  - accelRollDeg;
            pitchAngleErrDeg = att.pitch_deg - accelPitchDeg;
        }

        phaseStartUs = micros();
        RCCommand cmd = rcReceiver.getCommand();
        updateControlTransitionCounters(cmd);
        g_execTiming.lastRcUs = micros() - phaseStartUs;
 
        float rollCtrlDeg = 0.0f, pitchCtrlDeg = 0.0f, yawCtrlDeg = 0.0f;
        if (imuOk) computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);

        // Level-zero capture: DISARMED + SWB/ACRO high.
        // Uses raw CH8 because cmd.mode is DISARMED while disarmed, even if SWB is high.
        const bool swbAcroHigh = (cmd.valid && cmd.raw[RC_CH_AUX2] >= LEVEL_ZERO_SWB_THRESHOLD);
        const bool canCaptureLevelZero = (cmd.mode == FlightMode::DISARMED && imuOk && swbAcroHigh);
        const LevelTrimUpdate levelEvent = levelTrim.update(
            swbAcroHigh, canCaptureLevelZero, millis(), LEVEL_ZERO_SAMPLE_MS,
            att.roll_deg, att.pitch_deg, att.yaw_deg);
        if (levelEvent.captureStarted) {
            DBG_PRINTLN(F("[LEVEL] Capturing software zero — keep drone still..."));
            calLog("[LEVEL] Capturing software zero — keep drone still...");
        }
        if (levelEvent.captureCompleted) {
            computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);
            DBG_PRINTF("[LEVEL] Zero saved: rollOff=%+.2f pitchOff=%+.2f yawOff=%+.2f deg\n",
                       levelTrim.rollOffsetDeg(), levelTrim.pitchOffsetDeg(),
                       levelTrim.yawOffsetDeg());
            calLogf("[LEVEL] Zero saved: R=%+.2f P=%+.2f Y=%+.2f deg",
                    levelTrim.rollOffsetDeg(), levelTrim.pitchOffsetDeg(),
                    levelTrim.yawOffsetDeg());
        }

        // ── DISARMED / FAILSAFE ───────────────────────────────
        if (FlightSafetyPolicy::controlMustStop(
                cmd.mode == FlightMode::DISARMED,
                cmd.mode == FlightMode::FAILSAFE,
                false)) {
            phaseStartUs = micros();
            if (g_motorOutputsActive) {
                // One-shot stop only. Repeating motorOff() at 400 Hz was measured
                // as ~40 ms in your log, which destroys the loop timing.
                motorsOff();
                g_motorOutputsActive = false;
            }
            g_execTiming.lastMotorUs = micros() - phaseStartUs;
            uint32_t controlDoneUs = micros();

            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            lpfGx.reset(); lpfGy.reset(); lpfGz.reset();
            notchAx.reset(); notchAy.reset(); notchAz.reset();
            notchGx.reset(); notchGy.reset(); notchGz.reset();
            pidAngleYaw.reset();
            flightController.reset();

            phaseStartUs = micros();
            if (flightStateRte.lock(0)) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
                g_state.motorFLPreSat = g_state.motorFRPreSat = g_state.motorRLPreSat = g_state.motorRRPreSat = 0;
                g_state.batteryVoltage_v = batteryVoltageForRpm;
                g_state.batteryAdcVoltage_v = bat.adcVoltage_v;
                g_state.batteryCellVoltage_v = bat.cellVoltage_v;
                g_state.batteryPercent = bat.percent;
                g_state.batteryValid = bat.valid;
                g_state.batteryLow = bat.low;
                g_state.batteryCritical = bat.critical;
                g_state.pidRollOut = g_state.pidPitchOut = g_state.pidYawOut = 0;
                g_state.loopPeriod_us = periodUs;
                g_state.loopJitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
                g_state.imuRead_us = g_execTiming.lastImuUs;
                g_state.rcRead_us = g_execTiming.lastRcUs;
                g_state.ahrsUpdate_us = g_execTiming.lastAhrsUs;
                g_state.controlUpdate_us = controlDoneUs - execStartUs;
                g_state.motorWrite_us = g_execTiming.lastMotorUs;
                g_state.wifiService_us = g_lastWifiServiceUs;
                g_state.onboardLogWrite_us = g_execTiming.lastLogUs;
                g_state.missedLoopCount = g_execTiming.missedTimerReleases;
                g_state.rc      = cmd;
                g_state.angleModeActive = false;
                g_state.acroModeActive = false;
                g_state.modeSwitchRaw_us = cmd.raw[RC_CH_AUX2];
                g_state.armSwitchRaw_us = cmd.raw[RC_CH_FLIGHTMODE];
                g_state.auxTune1Raw_us = cmd.raw[RC_CH_AUX1];
                g_state.auxTune2Raw_us = cmd.raw[RC_CH_AUX5];
                g_state.rcFailsafeCount = rcReceiver.getFailsafeCount();
                g_state.angleLoopEnabled = false;
                g_state.rateLoopEnabled = false;
                g_state.actualRollRate_dps = imuOk ? gxf : 0.0f;
                g_state.actualPitchRate_dps = imuOk ? gyf : 0.0f;
                g_state.actualYawRate_dps = imuOk ? gzf : 0.0f;
                g_state.angleRollP = pidAngleRoll.lastP;
                g_state.angleRollI = pidAngleRoll.lastI;
                g_state.angleRollD = pidAngleRoll.lastD;
                g_state.anglePitchP = pidAnglePitch.lastP;
                g_state.anglePitchI = pidAnglePitch.lastI;
                g_state.anglePitchD = pidAnglePitch.lastD;
                g_state.rateRollP = pidRateRoll.lastP;
                g_state.rateRollI = pidRateRoll.lastI;
                g_state.rateRollD = pidRateRoll.lastD;
                g_state.ratePitchP = pidRatePitch.lastP;
                g_state.ratePitchI = pidRatePitch.lastI;
                g_state.ratePitchD = pidRatePitch.lastD;
                g_state.rateYawP = pidRateYaw.lastP;
                g_state.rateYawI = pidRateYaw.lastI;
                g_state.rateYawD = pidRateYaw.lastD;
                g_state.angleRollIterm = pidAngleRoll.integral;
                g_state.anglePitchIterm = pidAnglePitch.integral;
                g_state.pidResetCount = g_pidResetCount;
                g_state.modeTransitionCount = g_modeTransitionCount;
                g_state.lastModeChange_ms = g_lastModeChangeMs;
                g_state.armingTransitionCount = g_armingTransitionCount;
                g_state.lastArmChange_ms = g_lastArmChangeMs;
                g_state.throttleLow = true;
                g_state.controlAuthorityRemaining = 0.0f;
                g_state.rollOutputLimited = false;
                g_state.pitchOutputLimited = false;
                g_state.yawOutputLimited = false;
                g_state.rateOutputLimited = false;
                g_state.cmdRpmFL = g_state.cmdRpmFR = g_state.cmdRpmRL = g_state.cmdRpmRR = 0.0f;
                g_state.actualRpmFL = g_state.actualRpmFR = g_state.actualRpmRL = g_state.actualRpmRR = 0.0f;
                g_state.rpmActualValid = false;
                if (imuOk) {
                    g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                    g_state.yaw_deg  = att.yaw_deg;
                    g_state.q0 = att.q0; g_state.q1 = att.q1; g_state.q2 = att.q2; g_state.q3 = att.q3;
                    g_state.imuValid = imuOk;
                    g_state.magValid = imu.isMagConnected();
                    g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                    g_state.roll_ctrl_deg = rollCtrlDeg;
                    g_state.pitch_ctrl_deg = pitchCtrlDeg;
                    g_state.yaw_ctrl_deg = yawCtrlDeg;
                    g_state.roll_offset_deg = levelTrim.rollOffsetDeg();
                    g_state.pitch_offset_deg = levelTrim.pitchOffsetDeg();
                    g_state.yaw_offset_deg = levelTrim.yawOffsetDeg();
                    g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                    g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                    g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                    g_state.imuTemp_c=s.temp_c;
                    g_state.accelRoll_deg = accelRollDeg;
                    g_state.accelPitch_deg = accelPitchDeg;
                    g_state.gyroRoll_deg = g_gyroRollDeg;
                    g_state.gyroPitch_deg = g_gyroPitchDeg;
                    g_state.gyroYaw_deg = g_gyroYawDeg;
                    g_state.rollAngleError_deg = rollAngleErrDeg;
                    g_state.pitchAngleError_deg = pitchAngleErrDeg;
                    g_state.loopCount++;
                }
                flightStateRte.unlock();
            }
            g_execTiming.lastStateUs = micros() - phaseStartUs;

            uint32_t fullDoneUs = micros();
            updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                     fullDoneUs - execStartUs,
                                     periodUs,
                                     TARGET_US);
            continue;
        }

        // ── ARMED — cascaded PID ──────────────────────────────
        float roll  = imuOk ? rollCtrlDeg  : 0.0f;
        float pitch = imuOk ? pitchCtrlDeg : 0.0f;
        float gx    = imuOk ? gxf : 0.0f;
        float gy    = imuOk ? gyf : 0.0f;
        float gz    = imuOk ? gzf : 0.0f;

        TuningState tune;
        if (tuningRte.read(tune, 0)) {
        } else {
            // Extremely rare: keep flying with compile-time safe defaults for this cycle.
            memset(&tune, 0, sizeof(tune));
            tune.max_angle_deg              = TUNE_MAX_ANGLE_DEG;
            tune.max_rate_dps               = TUNE_MAX_RATE_DPS;
            tune.max_pitch_rate_dps         = TUNE_MAX_RATE_DPS;
            tune.roll_output_limit          = TUNE_ROLL_OUTPUT_LIMIT;
            tune.pitch_output_limit         = TUNE_PITCH_OUTPUT_LIMIT;
            tune.yaw_output_limit           = TUNE_YAW_OUTPUT_LIMIT;
            tune.throttle_expo              = TUNE_THROTTLE_EXPO;
            tune.throttle_up_rate_per_sec   = TUNE_THROTTLE_UP_RATE_PER_SEC;
            tune.throttle_down_rate_per_sec = TUNE_THROTTLE_DOWN_RATE_PER_SEC;
            tune.motor_idle                 = TUNE_MOTOR_IDLE;
            tune.motor_max                  = TUNE_MOTOR_MAX;
            tune.throttle_cut               = TUNE_THROTTLE_CUT;
            tune.idle_ramp_end              = TUNE_IDLE_RAMP_END;
            tune.yaw_deadband               = TUNE_YAW_DEADBAND;
            tune.yaw_max_rate_dps           = TUNE_YAW_MAX_RATE_DPS;
            tune.pid_roll_ff                = TUNE_RATE_ROLL_FF;
            tune.pid_pitch_ff               = TUNE_RATE_PITCH_FF;
            tune.pid_yaw_ff                 = TUNE_RATE_YAW_FF;
            tune.pid_roll_d_lpf_hz          = TUNE_RATE_ROLL_D_LPF_HZ;
            tune.pid_pitch_d_lpf_hz         = TUNE_RATE_PITCH_D_LPF_HZ;
            tune.pid_yaw_d_lpf_hz           = TUNE_RATE_YAW_D_LPF_HZ;
            tune.ahrs_filter_mode           = TUNE_AHRS_FILTER_MODE;
            tune.madgwick_beta              = TUNE_MADGWICK_BETA;
            tune.notch_enable               = TUNE_NOTCH_ENABLE;
            tune.notch_freq_hz              = TUNE_NOTCH_FREQ_HZ;
            tune.notch_q                    = TUNE_NOTCH_Q;
            tune.ekf_angle_q                = TUNE_EKF_ANGLE_Q;
            tune.ekf_bias_q                 = TUNE_EKF_BIAS_Q;
            tune.ekf_accel_r                = TUNE_EKF_ACCEL_R;
            tune.ekf_mag_r                  = TUNE_EKF_MAG_R;
            tune.ekf_mag_declination_deg    = TUNE_EKF_MAG_DECL_DEG;
            tune.ekf_mag_yaw_offset_deg     = TUNE_EKF_MAG_YAW_OFF_DEG;
            tune.ekf_mag_yaw_sign           = TUNE_EKF_MAG_YAW_SIGN;
        }

        const CascadedControlOutput control = flightController.update(
            {cmd.roll, cmd.pitch, cmd.yaw,
             roll, pitch, yawCtrlDeg,
             gx, gy, gz, dt, RC_LPF_HZ,
             cmd.mode == FlightMode::ANGLE, imuOk},
            tune);
        const float rollCmd = control.filteredRollCommand;
        const float pitchCmd = control.filteredPitchCommand;
        const float yawCmd = control.filteredYawCommand;
        const float targetRollDeg = control.targetRollDeg;
        const float targetPitchDeg = control.targetPitchDeg;
        const float targetYawDeg = control.targetYawDeg;
        const float targetRollRateDps = control.targetRollRateDps;
        const float targetPitchRateDps = control.targetPitchRateDps;
        const float targetYawRateDps = control.targetYawRateDps;
        const float angleErrRollDeg = control.rollAngleErrorDeg;
        const float angleErrPitchDeg = control.pitchAngleErrorDeg;
        const float yawErrDeg = control.yawErrorDeg;
        const float rateErrRollDps = control.rollRateErrorDps;
        const float rateErrPitchDps = control.pitchRateErrorDps;
        const float rateErrYawDps = control.yawRateErrorDps;
        const float rO = control.rollCorrection;
        const float pO = control.pitchCorrection;
        const float yO = control.yawCorrection;
        const bool rollOutputLimited = control.rollLimited;
        const bool pitchOutputLimited = control.pitchLimited;
        const bool yawOutputLimited = control.yawLimited;
        const bool rateOutputLimited = control.anyRateOutputLimited;

        // The application mixer owns throttle shaping and the Quad-X equations.
        const float MOTOR_MAX                  = tune.motor_max;
        const float THROTTLE_CUT               = tune.throttle_cut;
        const float thrRaw                     = constrain(cmd.throttle, 0.0f, 1.0f);
        const MotorMixerConfig mixerConfig{
            tune.throttle_expo,
            tune.throttle_up_rate_per_sec,
            tune.throttle_down_rate_per_sec,
            tune.motor_idle,
            tune.motor_max,
            tune.throttle_cut,
            tune.idle_ramp_end
        };
        const MotorMixerOutput mixed = motorMixer.update(
            {cmd.throttle, rO, pO, yO, dt}, mixerConfig);
        const float thr = mixed.shapedThrottle;
        const float flPre = mixed.frontLeftPreSaturation;
        const float frPre = mixed.frontRightPreSaturation;
        const float rlPre = mixed.rearLeftPreSaturation;
        const float rrPre = mixed.rearRightPreSaturation;
        float fl = mixed.frontLeft;
        float fr = mixed.frontRight;
        float rl = mixed.rearLeft;
        float rr = mixed.rearRight;
        const bool motorSaturated = mixed.saturated;
        const float maxMotor = mixed.maximumBeforeDesaturation;
        const float controlAuthorityRemaining = mixed.controlAuthorityRemaining;

        if (LOG_MOTOR_SATURATION && motorSaturated) {
            static uint32_t lastMotorSatPrintMs = 0;
            uint32_t motorSatNowMs = millis();
            if (motorSatNowMs - lastMotorSatPrintMs >= 250) {
                lastMotorSatPrintMs = motorSatNowMs;
                DBG_PRINTF("[MOTOR_SAT] thr=%.3f rO=%.3f pO=%.3f yO=%.3f "
                              "maxBefore=%.3f MOTOR_MAX=%.3f "
                              "FL=%.3f FR=%.3f RL=%.3f RR=%.3f\n",
                              thr, rO, pO, yO,
                              maxMotor, MOTOR_MAX,
                              fl, fr, rl, rr);
            }
        }

        // ── Motor PWM update ───────────────────────────────────
        // Runs every 400 Hz control cycle so motor output tracks PID immediately.
        static uint32_t motorWriteCount = 0;
        static uint32_t lastMotorRatePrintMs = 0;

        phaseStartUs = micros();
        writeMotors(fl, fr, rl, rr);
        g_motorOutputsActive = true;
        motorWriteCount++;

        g_execTiming.lastMotorUs = micros() - phaseStartUs;

        if (LOG_MOTOR_WRITE_RATE) {
            uint32_t motorRateNowMs = millis();
            if (motorRateNowMs - lastMotorRatePrintMs >= 1000) {
                lastMotorRatePrintMs = motorRateNowMs;

                DBG_PRINTF("[MOTOR_RATE] target=400Hz writes=%lu lastMotorUs=%lu\n",
                              (unsigned long)motorWriteCount,
                              (unsigned long)g_execTiming.lastMotorUs);

                motorWriteCount = 0;
            }
        } else {
            motorWriteCount = 0;
        }

        uint32_t controlDoneUs = micros();


        const float magNorm_uT = imuOk ? sqrtf(s.mx_uT*s.mx_uT + s.my_uT*s.my_uT + s.mz_uT*s.mz_uT) : 0.0f;
        const float rpmScaleLog = MOTOR_KV * batteryVoltageForRpm;
        const float cmdRpmFL = fl * rpmScaleLog;
        const float cmdRpmFR = fr * rpmScaleLog;
        const float cmdRpmRL = rl * rpmScaleLog;
        const float cmdRpmRR = rr * rpmScaleLog;

        // ── High-speed flight log @ 100 Hz (every 4th cycle) ──
        static uint8_t logDiv = 0;
        if (++logDiv >= 4) {
            logDiv = 0;
            LoggerRow row;
            memset(&row, 0, sizeof(row));
            row.t_us = nowUs;
            row.loop_count = g_state.loopCount;
            auto clampU16 = [](uint32_t v) -> uint16_t {
                return (uint16_t)((v > 65535UL) ? 65535UL : v);
            };
            row.period_us = clampU16(periodUs);
            row.jitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
            row.control_exec_us = clampU16(controlDoneUs - execStartUs);
            row.imu_read_us = clampU16(g_execTiming.lastImuUs);
            row.rc_read_us = clampU16(g_execTiming.lastRcUs);
            row.ahrs_exec_us = 0;
            row.pid_exec_us = 0;
            row.motor_exec_us = clampU16(g_execTiming.lastMotorUs);
            row.mode = (uint8_t)cmd.mode;
            row.flags = ((cmd.mode != FlightMode::DISARMED) ? 0x0001 : 0)
                      | (imuOk ? 0x0002 : 0)
                      | (cmd.valid ? 0x0004 : 0)
                      | ((imuOk && imu.isMagConnected()) ? 0x0008 : 0)
                      | (motorSaturated ? 0x0010 : 0)
                      | (g_state.rpmActualValid ? 0x0020 : 0);

            row.throttle = cmd.throttle;
            row.rc_roll = rollCmd;
            row.rc_pitch = pitchCmd;
            row.rc_yaw = yawCmd;
            row.target_roll_deg = targetRollDeg;
            row.target_pitch_deg = targetPitchDeg;
            row.target_yaw_deg = targetYawDeg;
            row.target_roll_rate_dps = targetRollRateDps;
            row.target_pitch_rate_dps = targetPitchRateDps;
            row.target_yaw_rate_dps = targetYawRateDps;

            row.ekf_roll_deg = imuOk ? att.roll_deg : 0.0f;
            row.ekf_pitch_deg = imuOk ? att.pitch_deg : 0.0f;
            row.ekf_yaw_deg = imuOk ? att.yaw_deg : 0.0f;
            row.ctrl_roll_deg = roll;
            row.ctrl_pitch_deg = pitch;
            row.ctrl_yaw_deg = imuOk ? yawCtrlDeg : 0.0f;
            row.zero_roll_deg = levelTrim.rollOffsetDeg();
            row.zero_pitch_deg = levelTrim.pitchOffsetDeg();
            row.zero_yaw_deg = levelTrim.yawOffsetDeg();

            row.ax_g = imuOk ? s.ax_g : 0.0f;
            row.ay_g = imuOk ? s.ay_g : 0.0f;
            row.az_g = imuOk ? s.az_g : 0.0f;
            row.gx_dps = imuOk ? s.gx_dps : 0.0f;
            row.gy_dps = imuOk ? s.gy_dps : 0.0f;
            row.gz_dps = imuOk ? s.gz_dps : 0.0f;
            row.mx_uT = imuOk ? s.mx_uT : 0.0f;
            row.my_uT = imuOk ? s.my_uT : 0.0f;
            row.mz_uT = imuOk ? s.mz_uT : 0.0f;
            row.accel_norm_g = sqrtf(row.ax_g*row.ax_g + row.ay_g*row.ay_g + row.az_g*row.az_g);
            row.gyro_norm_dps = sqrtf(row.gx_dps*row.gx_dps + row.gy_dps*row.gy_dps + row.gz_dps*row.gz_dps);
            row.mag_norm_uT = sqrtf(row.mx_uT*row.mx_uT + row.my_uT*row.my_uT + row.mz_uT*row.mz_uT);
            row.ekf_bgx_dps = attitudeEstimator.ekfRollBiasDps();
            row.ekf_bgy_dps = attitudeEstimator.ekfPitchBiasDps();
            row.ekf_bgz_dps = attitudeEstimator.ekfYawBiasDps();
            row.ahrs_mode = g_ahrsFilterModeActive;
            row.ekf_mag_used = attitudeEstimator.ekfMagAccepted() ? 1 : 0;

            row.angle_roll_error_deg = angleErrRollDeg;
            row.angle_pitch_error_deg = angleErrPitchDeg;
            row.yaw_error_deg = yawErrDeg;
            row.rate_roll_error_dps = rateErrRollDps;
            row.rate_pitch_error_dps = rateErrPitchDps;
            row.rate_yaw_error_dps = rateErrYawDps;

            row.angle_roll_p = pidAngleRoll.lastP;
            row.angle_roll_i = pidAngleRoll.lastI;
            row.angle_roll_d = pidAngleRoll.lastD;
            row.angle_roll_out = targetRollRateDps;
            row.angle_pitch_p = pidAnglePitch.lastP;
            row.angle_pitch_i = pidAnglePitch.lastI;
            row.angle_pitch_d = pidAnglePitch.lastD;
            row.angle_pitch_out = targetPitchRateDps;
            row.angle_yaw_p = pidAngleYaw.lastP;
            row.angle_yaw_i = pidAngleYaw.lastI;
            row.angle_yaw_d = pidAngleYaw.lastD;
            row.angle_yaw_out = targetYawRateDps;

            row.rate_roll_p = pidRateRoll.lastP;
            row.rate_roll_i = pidRateRoll.lastI;
            row.rate_roll_d = pidRateRoll.lastD;
            row.rate_roll_out = rO;
            row.rate_pitch_p = pidRatePitch.lastP;
            row.rate_pitch_i = pidRatePitch.lastI;
            row.rate_pitch_d = pidRatePitch.lastD;
            row.rate_pitch_out = pO;
            row.rate_yaw_p = pidRateYaw.lastP;
            row.rate_yaw_i = pidRateYaw.lastI;
            row.rate_yaw_d = pidRateYaw.lastD;
            row.rate_yaw_out = yO;

            row.motor_fl_pre = flPre;
            row.motor_fr_pre = frPre;
            row.motor_rl_pre = rlPre;
            row.motor_rr_pre = rrPre;
            row.motor_fl = fl;
            row.motor_fr = fr;
            row.motor_rl = rl;
            row.motor_rr = rr;
            row.motor_diag_a = fl + rr;
            row.motor_diag_b = fr + rl;
            row.motor_diag_diff = row.motor_diag_a - row.motor_diag_b;

            row.rpm_fl = cmdRpmFL;
            row.rpm_fr = cmdRpmFR;
            row.rpm_rl = cmdRpmRL;
            row.rpm_rr = cmdRpmRR;
            row.rpm_diag_a = cmdRpmFL + cmdRpmRR;
            row.rpm_diag_b = cmdRpmFR + cmdRpmRL;
            row.rpm_diag_diff = row.rpm_diag_a - row.rpm_diag_b;

            row.battery_v = batteryVoltageForRpm;
            row.cpu0_pct = g_state.cpuCore0_pct;
            row.cpu1_pct = g_state.cpuCore1_pct;
            row.notch_freq_hz = tune.notch_freq_hz;
            row.notch_q = tune.notch_q;
            row.notch_enable = tune.notch_enable ? 1 : 0;
            row.bmp_alt_m = g_state.bmpAltitude_m;
            row.bmp_vz_mps = g_state.bmpVerticalSpeed_mps;
            row.gps_valid = g_state.gps.valid ? 1 : 0;
            row.gps_sats = g_state.gps.satellites;
            row.gps_hdop = g_state.gps.hdop;

            uint32_t logStartUs = micros();
            flightLogger.push(row);
            g_execTiming.lastLogUs = micros() - logStartUs;
        }

        // ── Publish flight state (incl. true PID outputs) ─────
        phaseStartUs = micros();
        if (flightStateRte.lock(0)){
            if (imuOk) {
                g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                g_state.yaw_deg  = att.yaw_deg;
                g_state.q0=att.q0; g_state.q1=att.q1; g_state.q2=att.q2; g_state.q3=att.q3;
                g_state.imuValid = imuOk;
                g_state.magValid = imu.isMagConnected();
                g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                g_state.roll_ctrl_deg = rollCtrlDeg;
                g_state.pitch_ctrl_deg = pitchCtrlDeg;
                g_state.yaw_ctrl_deg = yawCtrlDeg;
                g_state.roll_offset_deg = levelTrim.rollOffsetDeg();
                g_state.pitch_offset_deg = levelTrim.pitchOffsetDeg();
                g_state.yaw_offset_deg = levelTrim.yawOffsetDeg();
                g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                g_state.imuTemp_c=s.temp_c;
                g_state.rawAx_g=s.ax_g; g_state.rawAy_g=s.ay_g; g_state.rawAz_g=s.az_g;
                g_state.rawGx_dps=s.gx_dps; g_state.rawGy_dps=s.gy_dps; g_state.rawGz_dps=s.gz_dps;
                g_state.filtAx_g=filtAx; g_state.filtAy_g=filtAy; g_state.filtAz_g=filtAz;
                g_state.filtGx_dps=gx; g_state.filtGy_dps=gy; g_state.filtGz_dps=gz;
                g_state.magNorm_uT=magNorm_uT;
                g_state.ekfMagUsed=attitudeEstimator.ekfMagAccepted();
                g_state.ekfBgx_dps=attitudeEstimator.ekfRollBiasDps();
                g_state.ekfBgy_dps=attitudeEstimator.ekfPitchBiasDps();
                g_state.ekfBgz_dps=attitudeEstimator.ekfYawBiasDps();
                g_state.targetRollDeg=targetRollDeg; g_state.targetPitchDeg=targetPitchDeg; g_state.targetYawDeg=targetYawDeg;
                g_state.targetRollRateDps=targetRollRateDps; g_state.targetPitchRateDps=targetPitchRateDps; g_state.targetYawRateDps=targetYawRateDps;
                g_state.rollRateError_dps=rateErrRollDps; g_state.pitchRateError_dps=rateErrPitchDps; g_state.yawRateError_dps=rateErrYawDps;
                g_state.yawError_deg=yawErrDeg;
                g_state.motorSaturated=motorSaturated;
                g_state.cmdRpmFL=cmdRpmFL; g_state.cmdRpmFR=cmdRpmFR; g_state.cmdRpmRL=cmdRpmRL; g_state.cmdRpmRR=cmdRpmRR;
                g_state.actualRpmFL=0.0f; g_state.actualRpmFR=0.0f; g_state.actualRpmRL=0.0f; g_state.actualRpmRR=0.0f; g_state.rpmActualValid=false;
                g_state.accelRoll_deg = accelRollDeg;
                g_state.accelPitch_deg = accelPitchDeg;
                g_state.gyroRoll_deg = g_gyroRollDeg;
                g_state.gyroPitch_deg = g_gyroPitchDeg;
                g_state.gyroYaw_deg = g_gyroYawDeg;
                g_state.rollAngleError_deg = rollAngleErrDeg;
                g_state.pitchAngleError_deg = pitchAngleErrDeg;
                g_state.loopCount++;
            }
            g_state.batteryVoltage_v = batteryVoltageForRpm;
            g_state.batteryAdcVoltage_v = bat.adcVoltage_v;
            g_state.batteryCellVoltage_v = bat.cellVoltage_v;
            g_state.batteryPercent = bat.percent;
            g_state.batteryValid = bat.valid;
            g_state.batteryLow = bat.low;
            g_state.batteryCritical = bat.critical;
            g_state.motorFL=fl; g_state.motorFR=fr;
            g_state.motorRL=rl; g_state.motorRR=rr;
            g_state.motorFLPreSat=flPre; g_state.motorFRPreSat=frPre;
            g_state.motorRLPreSat=rlPre; g_state.motorRRPreSat=rrPre;
            g_state.pidRollOut=rO; g_state.pidPitchOut=pO; g_state.pidYawOut=yO;
            g_state.loopPeriod_us = periodUs;
            g_state.loopJitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
            g_state.imuRead_us = g_execTiming.lastImuUs;
            g_state.rcRead_us = g_execTiming.lastRcUs;
            g_state.ahrsUpdate_us = g_execTiming.lastAhrsUs;
            g_state.controlUpdate_us = controlDoneUs - execStartUs;
            g_state.motorWrite_us = g_execTiming.lastMotorUs;
            g_state.wifiService_us = g_lastWifiServiceUs;
            g_state.onboardLogWrite_us = g_execTiming.lastLogUs;
            g_state.missedLoopCount = g_execTiming.missedTimerReleases;
            g_state.angleModeActive = (cmd.mode == FlightMode::ANGLE);
            g_state.acroModeActive = (cmd.mode == FlightMode::ACRO);
            g_state.modeSwitchRaw_us = cmd.raw[RC_CH_AUX2];
            g_state.armSwitchRaw_us = cmd.raw[RC_CH_FLIGHTMODE];
            g_state.auxTune1Raw_us = cmd.raw[RC_CH_AUX1];
            g_state.auxTune2Raw_us = cmd.raw[RC_CH_AUX5];
            g_state.rcFailsafeCount = rcReceiver.getFailsafeCount();
            g_state.angleLoopEnabled = (cmd.mode == FlightMode::ANGLE) && imuOk;
            g_state.rateLoopEnabled = imuOk && (cmd.mode == FlightMode::ANGLE || cmd.mode == FlightMode::ACRO);
            g_state.actualRollRate_dps = gx;
            g_state.actualPitchRate_dps = gy;
            g_state.actualYawRate_dps = gz;
            g_state.angleRollP = pidAngleRoll.lastP;
            g_state.angleRollI = pidAngleRoll.lastI;
            g_state.angleRollD = pidAngleRoll.lastD;
            g_state.anglePitchP = pidAnglePitch.lastP;
            g_state.anglePitchI = pidAnglePitch.lastI;
            g_state.anglePitchD = pidAnglePitch.lastD;
            g_state.rateRollP = pidRateRoll.lastP;
            g_state.rateRollI = pidRateRoll.lastI;
            g_state.rateRollD = pidRateRoll.lastD;
            g_state.ratePitchP = pidRatePitch.lastP;
            g_state.ratePitchI = pidRatePitch.lastI;
            g_state.ratePitchD = pidRatePitch.lastD;
            g_state.rateYawP = pidRateYaw.lastP;
            g_state.rateYawI = pidRateYaw.lastI;
            g_state.rateYawD = pidRateYaw.lastD;
            g_state.angleRollIterm = pidAngleRoll.integral;
            g_state.anglePitchIterm = pidAnglePitch.integral;
            g_state.pidResetCount = g_pidResetCount;
            g_state.modeTransitionCount = g_modeTransitionCount;
            g_state.lastModeChange_ms = g_lastModeChangeMs;
            g_state.armingTransitionCount = g_armingTransitionCount;
            g_state.lastArmChange_ms = g_lastArmChangeMs;
            g_state.throttleLow = (thrRaw <= THROTTLE_CUT);
            g_state.controlAuthorityRemaining = controlAuthorityRemaining;
            g_state.rollOutputLimited = rollOutputLimited;
            g_state.pitchOutputLimited = pitchOutputLimited;
            g_state.yawOutputLimited = yawOutputLimited;
            g_state.rateOutputLimited = rateOutputLimited;
            g_state.armed=true; g_state.rc=cmd;
            flightStateRte.unlock();
        }
        g_execTiming.lastStateUs = micros() - phaseStartUs;

        uint32_t fullDoneUs = micros();
        updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                 fullDoneUs - execStartUs,
                                 periodUs,
                                 TARGET_US);
    }
}


// ─────────────────────────────────────────────────────────────
//  taskSerial — Core 0, priority 1, 20 Hz
//  1 Hz status line; optional ~4 Hz [PID] trace (Serial 'p' toggles).
// ─────────────────────────────────────────────────────────────
static void taskSerial(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t tick = 0;

    DBG_PRINTLN(F("\n╔══════════════════════════════════════════════════════╗"));
    DBG_PRINTLN(F("  ║  FlySky iBUS + MPU-9250/6500 + BMP280 + GPS  v6.1.0  ║"));
    DBG_PRINTLN(F("  ║  Wi-Fi: ESP32-DRONE / 12345678 → 192.168.4.1         ║"));
    DBG_PRINTLN(F("  ║  taskControl: timer 400 Hz, original RC              ║"));
    DBG_PRINTLN(F("  ║  Type 'p' to toggle the [PID] tuning trace.          ║"));
    DBG_PRINTLN(F("  ╚══════════════════════════════════════════════════════╝"));

    for (;;) {

        RCCommand calCmd = rcReceiver.getCommand();

        bool safeForCalibration =
            calCmd.valid &&
            calCmd.mode == FlightMode::DISARMED;

        calManager.setSafety(safeForCalibration);

        // SWC / CH9 confirms the current calibration step
        static bool swcPrevForCal = false;
        bool swcNowForCal = calCmd.raw[RC_CH_AUX3] >= SWC_THRESHOLD;

        if (swcNowForCal && !swcPrevForCal) {
            calManager.confirmStep();
        }

        swcPrevForCal = swcNowForCal;

        calManager.update();
        // Print CalibrationManager status when active or when state changes
        static CalibrationState lastCalState = CalibrationState::IDLE;
        static uint32_t lastCalPrintMs = 0;

        CalibrationStatus calStatus = calManager.status();

        if (calStatus.state != lastCalState ||
            (calStatus.active && millis() - lastCalPrintMs >= 1000)) {

            lastCalState = calStatus.state;
            lastCalPrintMs = millis();

            calLogf("[CAL-MGR] run=%lu mode=%s src=%s state=%s active=%d safe=%d confirm=%d progress=%.0f%% msg=%s",
                    (unsigned long)calStatus.runId,
                    calModeName(calStatus.mode),
                    calSourceName(calStatus.source),
                    calStateName(calStatus.state),
                    calStatus.active ? 1 : 0,
                    calStatus.safeToRun ? 1 : 0,
                    calStatus.requiresUserConfirm ? 1 : 0,
                    (double)(calStatus.progress * 100.0f),
                    calStatus.message);
            if (calStatus.error[0]) {
                calLogf("[CAL-MGR][ERROR] run=%lu %s",
                        (unsigned long)calStatus.runId,
                        calStatus.error);
            }
        }

        // Handle inbound Serial commands (non-blocking)
        while (Serial.available() > 0) {
            int c = Serial.read();
            if (c == 'p' || c == 'P') {
                g_pidTrace = !g_pidTrace;
                DBG_PRINTF("[CMD] PID trace %s\n", g_pidTrace ? "ON" : "OFF");
            }
        }

        if (g_calibState != CalibState::IDLE && g_calibState != CalibState::DONE) {
            tick++; vTaskDelayUntil(&lastWake, period); continue;
        }

        // Snapshot once per loop; both blocks below use it.
        FlightState s;
        flightStateRte.read(s, pdMS_TO_TICKS(5));
        updateDynamicNotchFromFFT();
        const char* ms = "???";
        switch(s.rc.mode) {
            case FlightMode::DISARMED: ms="DISARMD"; break;
            case FlightMode::ANGLE:    ms="ANGLE  "; break;
            case FlightMode::ACRO:     ms="ACRO   "; break;
            case FlightMode::FAILSAFE: ms="FAILSFE"; break;
        }

        // 1 Hz status line
        if (tick % 20 == 0) {
            DBG_PRINTF("[%6lu] %s | RAW R=%+6.1f P=%+6.1f Y=%6.1f | "
                          "CTRL R=%+6.1f P=%+6.1f Y=%+6.1f | "
                          "OFF R=%+5.2f P=%+5.2f Y=%+5.2f | "
                          "MOT %.2f %.2f %.2f %.2f | "
                          "BMP %.1fC %.1fhPa %.1fm | "
                          "ToF %s %.3fm obj=%u %lums | "
                          "GPS %s Sats=%d | "
                          "Mag:%s Est:%s mKp=%.2f\n",
                          (unsigned long)tick, ms,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.roll_offset_deg, s.pitch_offset_deg, s.yaw_offset_deg,
                          s.motorFL, s.motorFR, s.motorRL, s.motorRR,
                          s.bmpTemp_c, s.bmpPressure_hpa, s.bmpAltitude_m,
                          s.tofValid ? "OK" : (s.tofReady ? "---" : "OFF"),
                          s.tofDistance_m,
                          (unsigned int)s.tofObjectCount,
                          (unsigned long)((s.tofAge_ms == UINT32_MAX) ? 0 : s.tofAge_ms),
                          s.gps.valid ? "FIX" : "---", s.gps.satellites,
                          imu.isMagConnected() ? "9DOF" : "6DOF",
                          (s.ahrsFilterMode == 1) ? "Mahony" : ((s.ahrsFilterMode == 2) ? "Madgwick" : "EKF"),
                          mahony.kp());
            DBG_PRINTF("          CH: 1=%4u 2=%4u 3=%4u 4=%4u 5=%4u 6=%4u 7=%4u 8=%4u 9=%4u 10=%4u\n",
                          s.rc.raw[0], s.rc.raw[1], s.rc.raw[2], s.rc.raw[3], s.rc.raw[4],
                          s.rc.raw[5], s.rc.raw[6], s.rc.raw[7], s.rc.raw[8], s.rc.raw[9]);
            
            float magNorm = sqrtf(s.mx_uT * s.mx_uT +
                      s.my_uT * s.my_uT +
                      s.mz_uT * s.mz_uT);

            DBG_PRINTF("          SENS: acc=%+.4f %+.4f %+.4f g | "
                        "gyro=%+.4f %+.4f %+.4f dps | "
                        "mag=%+.2f %+.2f %+.2f uT | norm=%.2f uT\n",
                        s.ax_g, s.ay_g, s.az_g,
                        s.gx_dps, s.gy_dps, s.gz_dps,
                        s.mx_uT, s.my_uT, s.mz_uT,
                        magNorm);
        }

        // ~4 Hz PID tuning trace (sibling of the 1 Hz block; gated by 'p')
        if (g_pidTrace && s.armed && (tick % 5 == 0)) {
            DBG_PRINTF("[PID] %s | cmd r=%+.2f p=%+.2f y=%+.2f | "
                          "ctrlAtt R=%+6.2f P=%+6.2f Y=%+6.2f | "
                          "gyro gx=%+7.1f gy=%+7.1f gz=%+7.1f | "
                          "out rO=%+.4f pO=%+.4f yO=%+.4f | "
                          "yawSP=%+6.1f hold=%d\n",
                          ms, s.rc.roll, s.rc.pitch, s.rc.yaw,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.gx_dps, s.gy_dps, s.gz_dps,
                          s.pidRollOut, s.pidPitchOut, s.pidYawOut,
                          flightController.yawSetpointDeg(),
                          (int)flightController.yawHoldActive());
        }

        tick++;
        vTaskDelayUntil(&lastWake, period);
    }
}

// ─────────────────────────────────────────────────────────────
//  taskToF — Core 0, priority 1, 40 Hz
// ─────────────────────────────────────────────────────────────
static void taskToF(void* /*pv*/)
{
    tofServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskBMP — Core 0, priority 1, 20 Hz
// ─────────────────────────────────────────────────────────────
static void taskBMP(void* /*pv*/)
{
    barometerServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskCPU — Core 0, priority 1, 2 Hz
// ─────────────────────────────────────────────────────────────
static void taskCPU(void* /*pv*/)
{
    cpuServiceTask.run();
}


// ─────────────────────────────────────────────────────────────
//  Wi-Fi spectrum provider — /spectrum endpoint
// ─────────────────────────────────────────────────────────────
static String provideSpectrumJson()
{
    TuningState t;
    memset(&t, 0, sizeof(t));
    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        t = g_tuning;
        tuningRte.unlock();
    } else {
        t.notch_enable  = TUNE_NOTCH_ENABLE;
        t.notch_freq_hz = TUNE_NOTCH_FREQ_HZ;
        t.notch_q       = TUNE_NOTCH_Q;
    }
    return spectrumAnalyzer.toJson(t.notch_freq_hz, t.notch_q, t.notch_enable);
}

// ─────────────────────────────────────────────────────────────
//  OTA safety gate — Web firmware update is bench-only.
//  OTA is allowed only when DISARMED, throttle low, and motors inactive.
// ─────────────────────────────────────────────────────────────
static bool otaIsSafeToStart()
{
    FlightState s;
    memset(&s, 0, sizeof(s));

    if (!flightStateRte.read(s, pdMS_TO_TICKS(5))) return false;

    return FlightSafetyPolicy::otaAllowed(
        {s.armed, s.rc.throttle, s.motorFL, s.motorFR, s.motorRL, s.motorRR});
}

// ─────────────────────────────────────────────────────────────
//  taskWiFi — Core 0, priority 1, event-driven
// ─────────────────────────────────────────────────────────────
static void taskWiFi(void* /*pv*/)
{
    const WifiServiceBindings bindings{
        provideTelemetry, handleTune, otaIsSafeToStart,
        provideTimingJson, provideTimingCsv, resetTimingStats,
        provideSpectrumJson, provideIdentityJson,
        flightLogCount, flightLogHeader, flightLogRowCsv, resetFlightLog
    };
    wifiServiceTask.run(bindings, "ESP32-DRONE", "12345678");
}

// ═════════════════════════════════════════════════════════════
//  setup()
// ═════════════════════════════════════════════════════════════
void setup()
{
    Serial.begin(115200);
    delay(600);

    configASSERT(flightStateRte.begin());
    configASSERT(tuningRte.begin());
    configASSERT(timingRte.begin());
    g_i2cMutex    = xSemaphoreCreateMutex();
    configASSERT(g_i2cMutex);
    if (!flightLogger.begin()) {
        DBG_PRINTLN(F("[BOOT][WARN] FlightLogger allocation failed; /flightlog/csv disabled."));
    }

    memset(&g_state,    0, sizeof(g_state));
    memset(&g_timing,   0, sizeof(g_timing));

    batteryMonitor.begin(PIN_BATTERY_ADC,
                         BATTERY_CELL_COUNT,
                         BATTERY_RTOP_OHMS,
                         BATTERY_RBOTTOM_OHMS,
                         BATTERY_NOMINAL_V,
                         BATTERY_ADC_CAL_SCALE);
    BatteryStatus bootBattery = batteryMonitor.forceRead();
    g_state.batteryVoltage_v = bootBattery.batteryVoltage_v;
    g_state.batteryAdcVoltage_v = bootBattery.adcVoltage_v;
    g_state.batteryCellVoltage_v = bootBattery.cellVoltage_v;
    g_state.batteryPercent = bootBattery.percent;
    g_state.batteryValid = bootBattery.valid;
    g_state.batteryLow = bootBattery.low;
    g_state.batteryCritical = bootBattery.critical;
    DBG_PRINTF("[BOOT] Battery ADC pin=%u scale=%.4f adc=%.3fV batt=%.3fV cell=%.3fV valid=%d\n",
               (unsigned)PIN_BATTERY_ADC,
               batteryMonitor.scale(),
               bootBattery.adcVoltage_v,
               bootBattery.batteryVoltage_v,
               bootBattery.cellVoltage_v,
               bootBattery.valid ? 1 : 0);
    DBG_PRINTLN(F("[BOOT] Battery sanity: 12.2V pack should read about 1.58V at ADC."));

    motorsBegin();
    motorEscArm();   // sends 1000 µs for 3 s; comment out after ESC calibration done
    g_motorOutputsActive = false;

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_MPU_CS);
    delay(10);

    DBG_PRINTF("[BOOT] IMU backend: %s... ", SelectedImu::backendName());
    if (!imu.begin()) {
        DBG_PRINTLN(F("FAILED. Check SPI wiring. Halting."));
        while (true) delay(1000);
    }
    DBG_PRINTLN(F("OK"));
#if FLIGHT_IMU_BACKEND == FLIGHT_IMU_BACKEND_MPU9250
    calManager.begin(imu);
#endif
    calManager.attachMotorOutputs(writeMotors, motorsOff);
    DBG_PRINTLN(F("[BOOT] CalibrationManager ready."));

    if (imu.hasMag()) {
        DBG_PRINTLN(F("[BOOT] AK8963 magnetometer: DETECTED — 9-DOF AHRS"));
    } else {
        DBG_PRINTLN(F("[BOOT] AK8963 magnetometer: NOT FOUND — 6-DOF AHRS"));
        DBG_PRINTLN(F("[BOOT] Roll and pitch accurate. Yaw will drift."));
    }

    bmp280.scanI2C(PIN_BMP_SDA, PIN_BMP_SCL, 100000);
    DBG_PRINT(F("[BOOT] BMP280... "));
    bmp280.beginAuto(PIN_BMP_SDA, PIN_BMP_SCL, 100000)
        ? DBG_PRINTLN(F("OK")) : DBG_PRINTLN(F("not found."));

    DBG_PRINT(F("[BOOT] VL53L4CX ToF... "));
    Wire.setClock(TOF_I2C_HZ);
    bool tofOk = tofSensor.begin(TOF_ST_ADDRESS,
                                 FlightToF_VL53L4CX::DISTANCE_MEDIUM,
                                 TOF_TIMING_BUDGET_US,
                                 TOF_INTER_MEASUREMENT_MS);
    g_state.tofReady = tofOk;
    if (tofOk) {
        DBG_PRINTLN(F("OK"));
    } else {
        DBG_PRINTLN(F("disabled/not found."));
    }

    DBG_PRINT(F("[BOOT] CPU monitor... "));
    cpuUtilization.begin(1000)
        ? DBG_PRINTLN(F("OK")) : DBG_PRINTLN(F("idle-hook failed."));

    DBG_PRINTLN(F("[BOOT] GPS (GY-GPS6MV2)..."));
    gps.begin(PIN_GPS_RX, PIN_GPS_TX, 9600);

    DBG_PRINT(F("[BOOT] NVS calibration... "));
    if (imu.loadCalibration()) {
        DBG_PRINTLN(F("loaded."));
        imu.printCalibration();
    } else {
        DBG_PRINTLN(F("none — flip SWD UP (disarmed) to calibrate."));
    }

    rcReceiver.begin(PIN_IBUS_RX, PIN_IBUS_TX, 2);
    DBG_PRINTLN(F("[BOOT] iBUS ready."));

    syncTuningFromObjects();
    DBG_PRINT(F("[BOOT] NVS tuning... "));
    if (g_tuningStore.load(g_tuning)) {
        applyTuningToObjects();
        DBG_PRINTLN(F("loaded and applied."));
    } else {
        DBG_PRINTLN(F("none/invalid — using firmware defaults."));
    }

    const Esp32TaskDefinition serviceTasks[] = {
        {taskRC,     "RC",     6144,  3, 0},
        {taskSerial, "Serial", 4096,  1, 0},
        {taskWiFi,   "WiFi",   12288, 1, 0},
        {taskBMP,    "BMP280", 3072,  1, 0},
        {taskToF,    "ToF",    4096,  1, 0},
        {taskCPU,    "CPU",    3072,  1, 0},
        {taskGPS,    "GPS",    4096,  1, 0}
    };
    const Esp32TaskDefinition controlTask{taskControl, "Ctrl", 10240, 5, 1};
    if (!flightScheduler.start(
            serviceTasks, sizeof(serviceTasks) / sizeof(serviceTasks[0]),
            controlTask, TIMING_TARGET_US)) {
        DBG_PRINTLN(F("[BOOT][ERROR] Failed to start flight scheduler."));
        while (true) delay(1000);
    }

    DBG_PRINTLN(F("[BOOT] All tasks running."));
    DBG_PRINTLN(F("[BOOT] Timing target: 2500 us (400 Hz control loop)."));
    DBG_PRINTLN(F("[BOOT] Ground station: http://192.168.4.1"));
    DBG_PRINTLN(F("[BOOT] GPS fix takes 30-90 s outdoors."));
    DBG_PRINTF("[BOOT] Free heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(portMAX_DELAY); }
