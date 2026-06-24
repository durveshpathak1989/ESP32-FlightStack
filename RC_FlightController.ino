/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v4.0.0                                ║
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
#include "MotorControl.h"
#include "MPU9250.h"
#include "FlySkyiBUS.h"
#include "TelemetryWiFi.h"
#include "BMP280Sensor.h"
#include "AttitudeEKF.h"
#include "ToFSensor.h"
#include "NotchFilter.h"
#include "SpectrumAnalyzer.h"
#include "MadgwickAHRS.h"
#include "MahonyAHRS.h"
#include "CPUUtilization.h"
#include "GPSSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "CalibrationManager.h"

// ─────────────────────────────────────────────────────────────
//  Pin assignments
// ─────────────────────────────────────────────────────────────
#define PIN_SPI_SCK   5
#define PIN_SPI_MISO  19
#define PIN_SPI_MOSI  18
#define PIN_MPU_CS    33
#define PIN_MPU_INT   27   // optional, not driven by firmware

#define PIN_MOTOR_FL  25
#define PIN_MOTOR_FR  15
#define PIN_MOTOR_RL  14
#define PIN_MOTOR_RR  32
#define PIN_IBUS_RX   16
#define PIN_IBUS_TX   4    // spare GPIO, not connected — avoids GPIO17 conflict with GPS TX
#define PIN_BMP_SDA   21
#define PIN_BMP_SCL   22
#define PIN_GPS_RX    13
#define PIN_GPS_TX    17   // GPS module RXD (optional for read-only operation)

// ═════════════════════════════════════════════════════════════
//  TUNING DASHBOARD — edit flight behavior here first
// ═════════════════════════════════════════════════════════════

// ── Loop timing + software filtering ────────────────────────
#define TIMING_BUF_SIZE       800     // ring buffer depth (samples)
#define TIMING_TARGET_US      2500     // nominal control period (400 Hz = 2500 µs)
#define JITTER_VIOLATION_US    100     // threshold: counts as a violation
#define GYRO_LPF_HZ           50.0f    // lower = smoother but more lag
#define RC_LPF_HZ             50.0f    // stick setpoint smoothing

// ── Pilot command limits ────────────────────────────────────
static constexpr float TUNE_MAX_ANGLE_DEG = 25.0f;
static constexpr float TUNE_MAX_RATE_DPS  = 150.0f;

// ── PID output authority limits before motor mixing ─────────
static constexpr float TUNE_ROLL_OUTPUT_LIMIT  = 0.500f;
static constexpr float TUNE_PITCH_OUTPUT_LIMIT = 0.500f;
static constexpr float TUNE_YAW_OUTPUT_LIMIT   = 0.200f;

// ── Throttle shaping + motor output limits ──────────────────
static constexpr float TUNE_THROTTLE_EXPO              = 0.60f;
static constexpr float TUNE_THROTTLE_UP_RATE_PER_SEC   = 0.70f;
static constexpr float TUNE_THROTTLE_DOWN_RATE_PER_SEC = 1.00f;
static constexpr float TUNE_MOTOR_IDLE                 = 0.08f;
static constexpr float TUNE_MOTOR_MAX                  = 0.80f;
static constexpr float TUNE_THROTTLE_CUT               = 0.03f;
static constexpr float TUNE_IDLE_RAMP_END              = 0.15f;

// ── Initial PID gains loaded at boot ────────────────────────
// Inner Loop
static constexpr float TUNE_RATE_ROLL_KP   = 0.00015f;
static constexpr float TUNE_RATE_ROLL_KI   = 0.00000f;
static constexpr float TUNE_RATE_ROLL_KD   = 0.00001f;
static constexpr float TUNE_RATE_PITCH_KP  = 0.00015f;
static constexpr float TUNE_RATE_PITCH_KI  = 0.00000f;
static constexpr float TUNE_RATE_PITCH_KD  = 0.00001f;
static constexpr float TUNE_RATE_YAW_KP    = 0.00015f;
static constexpr float TUNE_RATE_YAW_KI    = 0.0000000f;
static constexpr float TUNE_RATE_YAW_KD    = 0.00001f;
// Outer Loop
static constexpr float TUNE_ANGLE_ROLL_KP  = 8.00f;
static constexpr float TUNE_ANGLE_ROLL_KI  = 0.000f;
static constexpr float TUNE_ANGLE_ROLL_KD  = 0.010f;
static constexpr float TUNE_ANGLE_PITCH_KP = 8.00f;
static constexpr float TUNE_ANGLE_PITCH_KI = 0.000f;
static constexpr float TUNE_ANGLE_PITCH_KD = 0.010f;
// Outer Loop — Yaw heading hold
static constexpr float TUNE_ANGLE_YAW_KP     = 6.00f;   // heading-hold Kp (tune up if soft)
static constexpr float TUNE_YAW_DEADBAND     = 0.05f;   // |yaw stick| below this = hold
static constexpr float TUNE_YAW_MAX_RATE_DPS = 90.0f;   // cap on commanded yaw rate

// ── Motor vibration notch filter ────────────────────────────
// Runs before EKF and before rate PID. Keep center below 0.45*sample rate.
static constexpr bool  TUNE_NOTCH_ENABLE    = true;
static constexpr float TUNE_NOTCH_FREQ_HZ   = 90.0f;   // start point; tune from motor log/FFT
static constexpr float TUNE_NOTCH_Q         = 8.0f;    // higher = narrower notch
static constexpr float NOTCH_SAMPLE_HZ      = 400.0f;  // control loop sample rate
// ── Dynamic FFT-driven notch tracking ───────────────────────
// Start conservative. The notch should track slowly, not chase noise.
static constexpr bool     TUNE_DYNAMIC_NOTCH_ENABLE = true;
static constexpr float    DYN_NOTCH_MIN_HZ          = 45.0f;
static constexpr float    DYN_NOTCH_MAX_HZ          = 170.0f;  // below 0.45 * 400 Hz
static constexpr uint32_t DYN_NOTCH_UPDATE_MS       = 250;     // 10 Hz update rate
static constexpr float    DYN_NOTCH_ALPHA           = 0.08f;   // smoothing
static constexpr float    DYN_NOTCH_MAX_STEP_HZ     = 2.0f;    // max movement per update
static constexpr float    DYN_NOTCH_MIN_THROTTLE    = 0.15f;
static constexpr float    DYN_NOTCH_MIN_SCORE       = 3.5f;    // adjust based on your FFT output
//static constexpr float DYN_NOTCH_MIN_GYRO_MAG    = 0.30f;
//static constexpr float DYN_NOTCH_SMOOTH_ALPHA    = 0.20f;
// ── EKF tuning knobs ────────────────────────────────────────
// These are runtime-tunable from /tune while DISARMED.
// Higher R = trust that sensor less. Higher Q = allow faster EKF state motion.
static constexpr float TUNE_EKF_ANGLE_Q        = 0.0008f;
static constexpr float TUNE_EKF_BIAS_Q         = 0.000001f;
static constexpr float TUNE_EKF_ACCEL_R        = 0.060f;
static constexpr float TUNE_EKF_MAG_R          = 0.200f;
static constexpr float TUNE_EKF_MAG_DECL_DEG   = 0.0f;
static constexpr float TUNE_EKF_MAG_YAW_OFF_DEG= 0.0f;
static constexpr float TUNE_EKF_MAG_YAW_SIGN   = 1.0f;
static constexpr float TUNE_AHRS_FILTER_MODE    = 0.0f;   // 0=EKF, 1=Mahony, 2=Madgwick
static constexpr float TUNE_MADGWICK_BETA       = 0.080f;

// ── Software level-zero trim ───────────────────────────────
// DISARMED + SWB/ACRO high captures the current AHRS attitude
// as the control zero. This is applied AFTER AHRS; it does not
// modify MPU calibration or the Mahony quaternion state.
static constexpr uint32_t LEVEL_ZERO_SAMPLE_MS = 1200;
static constexpr uint16_t LEVEL_ZERO_SWB_THRESHOLD = 1700;

struct FlightLogRow {
    uint32_t t_us;
    uint32_t loop_count;
    uint16_t period_us;
    int16_t  jitter_us;
    uint16_t control_exec_us;
    uint16_t imu_read_us;
    uint16_t rc_read_us;
    uint16_t motor_exec_us;
    uint8_t  mode;        // 0=DISARM 1=ANGLE 2=ACRO 3=FAILSAFE
    uint8_t  flags;       // bit0 armed, bit1 imuOk, bit2 rcValid, bit3 magValid, bit4 bmpValid, bit5 gpsValid, bit6 motorSaturated, bit7 rpmActualValid

    uint16_t ch[IBUS_CHANNELS];
    float rcThrottle, thrRaw, thrShaped;
    float rcRoll, rcPitch, rcYaw;

    float estRoll, estPitch, estYaw;
    float ctrlRoll, ctrlPitch, ctrlYaw;
    float offRoll, offPitch, offYaw;

    float rawAx, rawAy, rawAz;
    float rawGx, rawGy, rawGz;
    float filtAx, filtAy, filtAz;
    float filtGx, filtGy, filtGz;
    float mx, my, mz;
    float magNorm;

    float ekfBgx, ekfBgy, ekfBgz;
    uint8_t ahrsMode;
    bool    ekfMagUsed;

    float targetRollDeg, targetPitchDeg, targetYawDeg;
    float targetRollRateDps, targetPitchRateDps, targetYawRateDps;
    float angleErrRollDeg, angleErrPitchDeg, yawErrDeg;
    float rateErrRollDps, rateErrPitchDps, rateErrYawDps;
    float rO, pO, yO;

    float mFL, mFR, mRL, mRR;
    float cmdRpmFL, cmdRpmFR, cmdRpmRL, cmdRpmRR;
    float actualRpmFL, actualRpmFR, actualRpmRL, actualRpmRR;

    float notchFreqHz, notchQ;
    bool  notchEnable;

    float bmpAltM;
    float bmpVzMps;
    float cpu0Pct, cpu1Pct;
};


// ─────────────────────────────────────────────────────────────
//  Calibration timing
// ─────────────────────────────────────────────────────────────
#define SWC_THRESHOLD      1700
#define ESC_CALIB_VRB_THRESHOLD 1900   // CH6 / VrB almost full-right = ESC calibration request
#define GYRO_SETTLE_MS     3000
#define GYRO_SAMPLE_MS     5000
#define ACCEL_HOLD_MS      3000
#define ACCEL_WAIT_MAX_MS 30000
#define MAG_DURATION_MS   30000

// ─────────────────────────────────────────────────────────────
//  Motor RPM estimation constants
// ─────────────────────────────────────────────────────────────
#define MOTOR_KV         920.0f
#define BATTERY_VOLTAGE    11.1f

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
    uint32_t lastMotorUs;
    uint32_t lastStateUs;
    uint32_t missedTimerReleases;
};

static volatile ExecTimingStats g_execTiming = {0,0,0,0,0,0,0,0,0,0,0};

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

    // Print only once per second so Serial does not dominate the control loop.
    static uint32_t lastPrintMs = 0;
    uint32_t nowMs = millis();

    if (nowMs - lastPrintMs >= 1000) {
        lastPrintMs = nowMs;

        int32_t jitterSigned  = (int32_t)periodUs - (int32_t)targetUs;
        int32_t headroomFull  = (int32_t)targetUs - (int32_t)fullUs;

        Serial.printf("[TIME] period=%luus jitter=%+ldus ctrl=%luus full=%luus "
                      "headroom=%+ldus maxCtrl=%luus maxFull=%luus "
                      "overCtrl=%lu overFull=%lu missed=%lu "
                      "phase imu=%luus rc=%luus motor=%luus state=%luus\n",
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
                      (unsigned long)g_execTiming.lastMotorUs,
                      (unsigned long)g_execTiming.lastStateUs);
    }
}

static TimingStats       g_timing;
static SemaphoreHandle_t g_timingMutex;

static void resetTimingStats()
{
    if (xSemaphoreTake(g_timingMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        memset(&g_timing, 0, sizeof(g_timing));
        xSemaphoreGive(g_timingMutex);
    }
}

// ─────────────────────────────────────────────────────────────
//  IMU + attitude estimator objects
// ─────────────────────────────────────────────────────────────
MPU9250 imu(PIN_MPU_CS);
CalibrationManager calManager;
MahonyAHRS mahony;
AttitudeEstimate mahonyAtt;
AttitudeEKF attitudeEKF;
MadgwickAHRS madgwickAHRS;
NotchFilter notchAx, notchAy, notchAz;
NotchFilter notchGx, notchGy, notchGz;
static float    g_dynamicNotchHz = TUNE_NOTCH_FREQ_HZ;
static bool     g_dynamicNotchTracking = false;
static uint32_t g_lastDynamicNotchUpdateMs = 0;

SpectrumAnalyzer spectrumAnalyzer(NOTCH_SAMPLE_HZ);

#define FLIGHT_LOG_SIZE 100   // expanded debug row: 300 samples @ 100 Hz = 3 s



// ─────────────────────────────────────────────────────────────
//  Shared flight state
// ─────────────────────────────────────────────────────────────
struct FlightState {
    // Raw estimator attitude from EKF/Mahony/Madgwick. Keep untouched for telemetry/debug.
    float roll_deg, pitch_deg, yaw_deg;
    bool imuValid;
    bool magValid;
    uint8_t ahrsFilterMode;

    // Post-AHRS control attitude. PID uses these values after level-zero offset.
    float roll_ctrl_deg, pitch_ctrl_deg, yaw_ctrl_deg;

    // Captured software level-zero offsets subtracted from raw AHRS.
    float roll_offset_deg, pitch_offset_deg, yaw_offset_deg;
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float mx_uT, my_uT, mz_uT;
    float bmpTemp_c, bmpPressure_hpa, bmpAltitude_m;
    bool  bmpValid;
    float cpuCore0_pct, cpuCore1_pct;
    bool  cpuValid;
    float motorFL, motorFR, motorRL, motorRR;
    float pidRollOut, pidPitchOut, pidYawOut;   // true PID outputs for tuning trace

    // Extended flight-log / telemetry debug fields
    float rawAx_g, rawAy_g, rawAz_g;
    float rawGx_dps, rawGy_dps, rawGz_dps;
    float filtAx_g, filtAy_g, filtAz_g;
    float filtGx_dps, filtGy_dps, filtGz_dps;
    float magNorm_uT;
    bool  ekfMagUsed;
    float ekfBgx_dps, ekfBgy_dps, ekfBgz_dps;
    float targetRollDeg, targetPitchDeg, targetYawDeg;
    float targetRollRateDps, targetPitchRateDps, targetYawRateDps;
    float rollRateError_dps, pitchRateError_dps, yawRateError_dps;
    float yawError_deg;
    bool  motorSaturated;
    float cmdRpmFL, cmdRpmFR, cmdRpmRL, cmdRpmRR;
    float actualRpmFL, actualRpmFR, actualRpmRL, actualRpmRR;
    bool  rpmActualValid;
    float bmpVerticalSpeed_mps;

    // Diagnostic attitude sources for PID/AHRS review
    float accelRoll_deg, accelPitch_deg;        // accel-only tilt estimate
    float gyroRoll_deg,  gyroPitch_deg, gyroYaw_deg; // integrated gyro-only estimate
    float rollAngleError_deg, pitchAngleError_deg;   // AHRS - accel estimate

    RCCommand rc;
    bool  armed;
    uint32_t loopCount;
    GPSData gps;
};
static FlightState       g_state;
static SemaphoreHandle_t g_flightMutex;

// Software level-zero trim state. Offset is post-AHRS only.
static float g_levelRollOffsetDeg  = 0.0f;
static float g_levelPitchOffsetDeg = 0.0f;
static float g_levelYawOffsetDeg   = 0.0f;

// PID-trace toggle (Serial 'p'). Off at boot so the log stays clean.
static volatile bool g_pidTrace = false;

// ─────────────────────────────────────────────────────────────
//  Tuning state
// ─────────────────────────────────────────────────────────────
struct TuningState {
    // Pilot command limits
    float max_angle_deg;
    float max_rate_dps;
    float max_pitch_rate_dps;

    // PID output authority limits before motor mixing
    float roll_output_limit;
    float pitch_output_limit;
    float yaw_output_limit;

    // Throttle shaping + motor output limits
    float throttle_expo;
    float throttle_up_rate_per_sec;
    float throttle_down_rate_per_sec;
    float motor_idle;
    float motor_max;
    float throttle_cut;
    float idle_ramp_end;
    float pid_ilimit;

    // Inner rate loop PID
    float pid_roll_kp,  pid_roll_ki,  pid_roll_kd;
    float pid_pitch_kp, pid_pitch_ki, pid_pitch_kd;
    float pid_yaw_kp,   pid_yaw_ki,   pid_yaw_kd;

    // Outer angle loop PID
    float pid_angle_roll_kp,  pid_angle_roll_ki,  pid_angle_roll_kd;
    float pid_angle_pitch_kp, pid_angle_pitch_ki, pid_angle_pitch_kd;

    // Outer yaw heading-hold loop
    float pid_angle_yaw_kp;
    float yaw_deadband;
    float yaw_max_rate_dps;

    // AHRS / estimator selection
    float mahony_kp, mahony_ki;
    float ahrs_filter_mode;
    float madgwick_beta;

    // Motor vibration notch filter
    bool  notch_enable;
    float notch_freq_hz;
    float notch_q;

    // Attitude EKF tuning
    float ekf_angle_q;
    float ekf_bias_q;
    float ekf_accel_r;
    float ekf_mag_r;
    float ekf_mag_declination_deg;
    float ekf_mag_yaw_offset_deg;
    float ekf_mag_yaw_sign;

    volatile bool dirty;
};
static TuningState       g_tuning;
static SemaphoreHandle_t g_tuneMutex;

// Tune transaction diagnostics for GCS verification.
// request_seq increments when /tune accepts a payload while disarmed.
// apply_seq follows request_seq after taskControl applies the values to PID/AHRS objects.
// reject_seq increments when /tune is rejected, typically because the drone is armed.
static volatile uint32_t g_tuneRequestSeq = 0;
static volatile uint32_t g_tuneApplySeq   = 0;
static volatile uint32_t g_tuneRejectSeq  = 0;
static volatile uint8_t  g_ahrsFilterModeActive = 0;

// ─────────────────────────────────────────────────────────────
//  Calibration state machine
// ─────────────────────────────────────────────────────────────
enum class CalibState : uint8_t {
    IDLE=0, REQUESTED, RUNNING_GYRO, RUNNING_ACCEL, RUNNING_MAG, SAVING, DONE
};
static volatile CalibState g_calibState = CalibState::IDLE;

// ─────────────────────────────────────────────────────────────
//  Task handles
// ─────────────────────────────────────────────────────────────
static TaskHandle_t hTaskIMU    = nullptr;
static TaskHandle_t hTaskControl= nullptr;
static TaskHandle_t hTaskRC     = nullptr;
static TaskHandle_t hTaskSerial = nullptr;
static TaskHandle_t hTaskWiFi   = nullptr;
static TaskHandle_t hTaskBMP    = nullptr;
static TaskHandle_t hTaskCPU    = nullptr;
static TaskHandle_t hTaskGPS    = nullptr;

// High-resolution 400 Hz release timer.
// This is better than vTaskDelayUntil() when the FreeRTOS tick is coarse,
// and better than a busy-wait because the control task blocks between releases
// so the watchdog/idle task still gets CPU time.
static esp_timer_handle_t g_controlTimer = nullptr;

static void controlTimerCallback(void* /*arg*/)
{
    if (hTaskControl != nullptr) {
        xTaskNotifyGive(hTaskControl);
    }
}

// ─────────────────────────────────────────────────────────────
//  PID controller
// ─────────────────────────────────────────────────────────────
struct PID {
    float kp, ki, kd, integral=0, prevError=0, iLimit=50.0f;
    PID(float p,float i,float d,float il=50.0f):kp(p),ki(i),kd(d),iLimit(il){}
    float update(float err,float dt){
        integral=constrain(integral+err*dt,-iLimit,iLimit);
        float d2=(err-prevError)/dt; prevError=err;
        return kp*err+ki*integral+kd*d2;
    }
    void reset(){integral=0;prevError=0;}
};

static PID pidRateRoll  (TUNE_RATE_ROLL_KP,   TUNE_RATE_ROLL_KI,   TUNE_RATE_ROLL_KD);
static PID pidRatePitch (TUNE_RATE_PITCH_KP,  TUNE_RATE_PITCH_KI,  TUNE_RATE_PITCH_KD);
static PID pidRateYaw   (TUNE_RATE_YAW_KP,    TUNE_RATE_YAW_KI,    TUNE_RATE_YAW_KD);

static PID pidAngleRoll (TUNE_ANGLE_ROLL_KP,  TUNE_ANGLE_ROLL_KI,  TUNE_ANGLE_ROLL_KD);
static PID pidAnglePitch(TUNE_ANGLE_PITCH_KP, TUNE_ANGLE_PITCH_KI, TUNE_ANGLE_PITCH_KD);
// Yaw Control
static PID pidAngleYaw  (TUNE_ANGLE_YAW_KP,   0.0f,                0.0f);

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
// Copy g_tuning into live PID/AHRS objects. Caller already holds g_tuneMutex.
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
    if (xSemaphoreTake(g_tuneMutex, 0) != pdTRUE) return;
    applyTuningToObjectsLocked();
    xSemaphoreGive(g_tuneMutex);
    calLog("[TUNE] Runtime tune values applied.");
}
// ─────────────────────────────────────────────────────────────
//  Log helpers (push to WiFi ring buffer + Serial)
// ─────────────────────────────────────────────────────────────
static void calLog(const char* msg)  { telemetryWiFi.pushLog(msg); }
static void calLogf(const char* fmt, ...) {
    char buf[160];
    va_list args; va_start(args,fmt); vsnprintf(buf,sizeof(buf),fmt,args); va_end(args);
    telemetryWiFi.pushLog(buf);
}

// ─────────────────────────────────────────────────────────────
//  Timing JSON provider — /timing endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingJson()
{
    if (xSemaphoreTake(g_timingMutex, pdMS_TO_TICKS(20)) != pdTRUE) {
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
    xSemaphoreGive(g_timingMutex);

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

// ─────────────────────────────────────────────────────────────
//  Timing CSV provider — /timing/csv endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingCsv()
{
    if (xSemaphoreTake(g_timingMutex, pdMS_TO_TICKS(20)) != pdTRUE) {
        return F("error,mutex_timeout\n");
    }
    uint16_t n    = g_timing.bufFull ? TIMING_BUF_SIZE : g_timing.bufHead;
    uint16_t head = g_timing.bufFull ? g_timing.bufHead : 0;

    static uint16_t tmp[TIMING_BUF_SIZE];
    for (uint16_t i = 0; i < n; i++) {
        tmp[i] = g_timing.buf[(head + i) % TIMING_BUF_SIZE];
    }
    xSemaphoreGive(g_timingMutex);

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

static bool provideTelemetry(TelemetryPacket& out)
{
    FlightState s;
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(15)) != pdTRUE) return false;
    s = g_state;
    xSemaphoreGive(g_flightMutex);

    TuningState t;
    memset(&t, 0, sizeof(t));
    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        t = g_tuning; xSemaphoreGive(g_tuneMutex);
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
    out.roll_ctrl_deg  = s.roll_ctrl_deg;
    out.pitch_ctrl_deg = s.pitch_ctrl_deg;
    out.yaw_ctrl_deg   = s.yaw_ctrl_deg;
    out.roll_offset_deg  = s.roll_offset_deg;
    out.pitch_offset_deg = s.pitch_offset_deg;
    out.yaw_offset_deg   = s.yaw_offset_deg;
    out.ax_g=s.ax_g; out.ay_g=s.ay_g; out.az_g=s.az_g;
    out.gx_dps=s.gx_dps; out.gy_dps=s.gy_dps; out.gz_dps=s.gz_dps;
    out.mx_uT=s.mx_uT;   out.my_uT=s.my_uT;   out.mz_uT=s.mz_uT;
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
    out.motor_fl=s.motorFL; out.motor_fr=s.motorFR;
    out.motor_rl=s.motorRL; out.motor_rr=s.motorRR;
    float rpmScale = MOTOR_KV * BATTERY_VOLTAGE;
    out.cmd_rpm_fl=s.motorFL*rpmScale; out.cmd_rpm_fr=s.motorFR*rpmScale;
    out.cmd_rpm_rl=s.motorRL*rpmScale; out.cmd_rpm_rr=s.motorRR*rpmScale;
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
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        armed = g_state.armed; xSemaphoreGive(g_flightMutex);
    }
    if (armed) {
        g_tuneRejectSeq++;
        calLog("[TUNE] REJECTED — disarm first.");
        return false;
    }

    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(10)) != pdTRUE) {
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
    uint32_t appliedSeq = g_tuneApplySeq;
    xSemaphoreGive(g_tuneMutex);

    Serial.printf("[TUNE] Applied immediately seq=%lu.\n", (unsigned long)appliedSeq);
    calLogf("[TUNE] Applied seq=%lu.", (unsigned long)appliedSeq);
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

    // ── Stage 1: Gyro ────────────────────────────────────────
    g_calibState = CalibState::RUNNING_GYRO;
    calLogf("[CAL] 1/3 GYRO — flat + still. Settle %ds, sample %ds...",
            GYRO_SETTLE_MS/1000, GYRO_SAMPLE_MS/1000);
    silentWait(GYRO_SETTLE_MS);
    {
        MPU_SensorData avg;
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

        MPU_SensorData avg;
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
            MPU_SensorData s;
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
    const TickType_t period = pdMS_TO_TICKS(20);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastPrintMs = 0;

    for (;;) {
        gps.update();
        GPSData d = gps.get();
        if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
            g_state.gps = d; xSemaphoreGive(g_flightMutex);
        }
        if (millis() - lastPrintMs >= 5000) {
            if (d.valid)
                Serial.printf("[GPS] Fix  Lat=%.6f  Lon=%.6f  Alt=%.1fm  Sats=%d"
                              "  HDOP=%.1f  Speed=%.1fkm/h  UTC=%02d:%02d:%02d\n",
                              d.latitude, d.longitude, d.altitude_m,
                              d.satellites, d.hdop, d.speed_kmh,
                              d.hour, d.minute, d.second);
            else
                Serial.printf("[GPS] No fix  Sats=%d  Sentences=%lu\n",
                              d.satellites, (unsigned long)d.sentenceCount);
            lastPrintMs = millis();
        }
        vTaskDelayUntil(&lastWake, period);
    }
}

// ─────────────────────────────────────────────────────────────
//  taskRC — Core 0, priority 3
//  Important: do NOT use vTaskDelayUntil(5 ms) here if the FreeRTOS tick
//  is coarse. On this build, one tick is behaving like ~40 ms, which can
//  starve the iBUS parser. Use micros() pacing + taskYIELD() instead.
// ─────────────────────────────────────────────────────────────
static void taskRC(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();
    bool swdPrev = false;
    bool vrbEscPrev = false;


    for (;;) {
        rcReceiver.update();
        RCCommand cmd = rcReceiver.getCommand();

        if (cmd.swdHigh && !swdPrev) {
            if (cmd.mode == FlightMode::DISARMED) {
                calManager.request(CalibrationMode::IMU_ALL_GUIDED,CalibrationSource::RC);        
                calLog("[RC] Calibration Manager request — IMU All GUIDED.");
            } else if (cmd.mode != FlightMode::DISARMED) {
                calLog("[RC] Cannot calibrate while armed.");
            }
        }
        swdPrev = cmd.swdHigh;
        // CH6 / VrB rising edge requests ESC calibration.
        // SWC still confirms the dangerous step inside CalibrationManager.
        const bool vrbEscHigh = cmd.valid &&
                                (cmd.raw[RC_CH_AUX5] >= ESC_CALIB_VRB_THRESHOLD);

        if (vrbEscHigh && !vrbEscPrev) {
            if (cmd.mode == FlightMode::DISARMED && cmd.throttle <= TUNE_THROTTLE_CUT) {
                calManager.request(CalibrationMode::ESC, CalibrationSource::RC);
                calLog("[RC] ESC calibration requested by VrB. Use SWC to confirm.");
            } else {
                calLog("[RC] ESC calibration rejected — disarm and set throttle low first.");
            }
        }

        vrbEscPrev = vrbEscHigh;

        // Optional safety cancel: lowering VrB cancels ESC calibration.
        CalibrationStatus rcCalStatus = calManager.status();
        if (rcCalStatus.active &&
            rcCalStatus.mode == CalibrationMode::ESC &&
            !vrbEscHigh) {
            calManager.cancel();
            calLog("[RC] ESC calibration cancelled — VrB lowered.");
        }
        // ── Per-second iBUS health: good rate vs checksum failures ──
        static uint32_t lastReportMs = 0;
        static uint32_t lastCsumFail = 0;
        uint32_t nowMs = millis();
        if (nowMs - lastReportMs >= 1000) {
            uint32_t fails    = rcReceiver.getChecksumFailCount();
            uint32_t failPerS = fails - lastCsumFail;
            lastCsumFail = fails;
            lastReportMs = nowMs;
            Serial.printf("[iBUS] %.0f Hz good | %lu bad/s | %lu bad total\n",
                          rcReceiver.getFrameRate(),
                          (unsigned long)failPerS,
                          (unsigned long)fails);
        }
        vTaskDelayUntil(&lastWake, period);
    }
}

// throttle smoothening
static float throttleExpo(float x, float expo)
{
    x = constrain(x, 0.0f, 1.0f);
    expo = constrain(expo, 0.0f, 1.0f);
    return expo * x * x * x +(1.0f - expo) * x;
}

static float smoothStep01(float x)
{
    x = constrain(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

static float wrapDeg180(float a)
{
    while (a >  180.0f) a -= 360.0f;
    while (a < -180.0f) a += 360.0f;
    return a;
}

static void computeControlAttitude(const AttitudeEstimate& att,
                                   float& rollCtrl,
                                   float& pitchCtrl,
                                   float& yawCtrl)
{
    rollCtrl  = att.roll_deg  - g_levelRollOffsetDeg;
    pitchCtrl = att.pitch_deg - g_levelPitchOffsetDeg;
    yawCtrl   = wrapDeg180(att.yaw_deg - g_levelYawOffsetDeg);
}

// First-order gyro/setpoint low-pass (one state per axis)
struct LPF {
    float y = 0.0f;
    bool initialized = false;

    float apply(float x, float dt, float fc) {
        if (dt <= 0.0f || fc <= 0.0f) return x;
        if (!initialized) {
            y = x;
            initialized = true;
            return y;
        }
        float rc = 1.0f / (2.0f * 3.14159265f * fc);
        float a  = dt / (dt + rc);
        y += a * (x - y);
        return y;
    }

    void reset() {
        y = 0.0f;
        initialized = false;
    }
};
static LPF lpfGx, lpfGy, lpfGz, lpfSpRoll, lpfSpPitch, lpfSpYaw;

static float g_yawSetpoint   = 0.0f;
static bool  g_yawHoldActive = false;

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
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        s = g_state;
        xSemaphoreGive(g_flightMutex);
    } else {
        return;
    }

    // Do not learn while disarmed, motors off, failsafe, or low throttle.
    if (!s.armed ||
        !g_motorOutputsActive ||
        !s.rc.valid ||
        s.rc.throttle < DYN_NOTCH_MIN_THROTTLE) {

        static uint32_t lastGatePrintMs = 0;
        if (nowMs - lastGatePrintMs >= 1000) {
            lastGatePrintMs = nowMs;

            Serial.printf("[DYN_NOTCH_GATE] blocked armed=%d motorOut=%d rcValid=%d throttle=%.3f minThrottle=%.3f\n",
                          s.armed ? 1 : 0,
                          g_motorOutputsActive ? 1 : 0,
                          s.rc.valid ? 1 : 0,
                          s.rc.throttle,
                          DYN_NOTCH_MIN_THROTTLE);
        }

        return;
    }

    bool notchEnabled = false;
    float currentHz = TUNE_NOTCH_FREQ_HZ;

    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        notchEnabled = g_tuning.notch_enable;
        currentHz = g_tuning.notch_freq_hz;
        xSemaphoreGive(g_tuneMutex);
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
        static uint32_t lastNoPeakPrintMs = 0;
        if (nowMs - lastNoPeakPrintMs >= 1000) {
            lastNoPeakPrintMs = nowMs;

            Serial.printf("[DYN_NOTCH_NO_PEAK] current=%.1fHz min=%.1f max=%.1f minScore=%.2f seq=%lu\n",
                          currentHz,
                          DYN_NOTCH_MIN_HZ,
                          DYN_NOTCH_MAX_HZ,
                          DYN_NOTCH_MIN_SCORE,
                          (unsigned long)seq);
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

    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        g_tuning.notch_freq_hz = requestedHz;
        g_tuning.dirty = true;   // taskControl applies notch safely
        xSemaphoreGive(g_tuneMutex);
    }

    Serial.printf("[DYN_NOTCH] peak=%.1fHz score=%.3f current=%.1fHz -> request=%.1fHz seq=%lu\n",
                  peakHz,
                  peakScore,
                  currentHz,
                  requestedHz,
                  (unsigned long)seq);
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

static FlightLogRow  g_log[FLIGHT_LOG_SIZE];
static volatile uint16_t g_logHead   = 0;
static volatile bool     g_logFull   = false;
static volatile bool     g_logActive = true;   // writes enabled; GET freezes it
static portMUX_TYPE      g_logMux    = portMUX_INITIALIZER_UNLOCKED;

static void pushFlightLog(const FlightLogRow& row)
{
    portENTER_CRITICAL(&g_logMux);
    if (g_logActive) {
        g_log[g_logHead] = row;
        if (++g_logHead >= FLIGHT_LOG_SIZE) { g_logHead = 0; g_logFull = true; }
    }
    portEXIT_CRITICAL(&g_logMux);
}

static void resetFlightLog()    // POST /flightlog/reset — clear + re-arm logging
{
    portENTER_CRITICAL(&g_logMux);
    g_logHead = 0; g_logFull = false; g_logActive = true;
    portEXIT_CRITICAL(&g_logMux);
}

// Called by the WiFi CSV handler. Freezes the buffer so the read is
// race-free, then reports how many rows are available.
static uint16_t flightLogCount()
{
    portENTER_CRITICAL(&g_logMux);
    g_logActive = false;
    uint16_t n = g_logFull ? FLIGHT_LOG_SIZE : g_logHead;
    portEXIT_CRITICAL(&g_logMux);
    return n;
}

static String flightLogHeader()
{
    return F("t_us,loop_count,period_us,jitter_us,control_exec_us,imu_read_us,rc_read_us,motor_exec_us,"
             "mode,armed,imuOk,rcValid,magValid,bmpValid,gpsValid,motorSaturated,rpmActualValid,"
             "ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8,ch9,ch10,"
             "rcThrottle,thrRaw,thrShaped,rcRoll,rcPitch,rcYaw,"
             "estRoll,estPitch,estYaw,ctrlRoll,ctrlPitch,ctrlYaw,offRoll,offPitch,offYaw,"
             "rawAx,rawAy,rawAz,rawGx,rawGy,rawGz,filtAx,filtAy,filtAz,filtGx,filtGy,filtGz,"
             "mx,my,mz,magNorm,ahrsMode,ekfMagUsed,ekfBgx,ekfBgy,ekfBgz,"
             "targetRollDeg,targetPitchDeg,targetYawDeg,targetRollRateDps,targetPitchRateDps,targetYawRateDps,"
             "angleErrRollDeg,angleErrPitchDeg,yawErrDeg,rateErrRollDps,rateErrPitchDps,rateErrYawDps,"
             "rollRateOut,pitchRateOut,yawRateOut,"
             "motFL,motFR,motRL,motRR,cmdRpmFL,cmdRpmFR,cmdRpmRL,cmdRpmRR,actualRpmFL,actualRpmFR,actualRpmRL,actualRpmRR,"
             "notchEnable,notchFreqHz,notchQ,bmpAltM,bmpVzMps,cpu0Pct,cpu1Pct\n");
}

// Row i in chronological order. Buffer is frozen during download.
static String flightLogRowCsv(uint16_t i)
{
    uint16_t head, n; bool full;
    portENTER_CRITICAL(&g_logMux);
    full = g_logFull; head = g_logHead;
    portEXIT_CRITICAL(&g_logMux);
    n = full ? FLIGHT_LOG_SIZE : head;
    if (i >= n) return String();

    const FlightLogRow& r = g_log[full ? ((head + i) % FLIGHT_LOG_SIZE) : i];

    String s; s.reserve(900);
    s += String(r.t_us);                 s += ',';
    s += String(r.loop_count);           s += ',';
    s += String(r.period_us);            s += ',';
    s += String(r.jitter_us);            s += ',';
    s += String(r.control_exec_us);      s += ',';
    s += String(r.imu_read_us);          s += ',';
    s += String(r.rc_read_us);           s += ',';
    s += String(r.motor_exec_us);        s += ',';
    s += String(r.mode);                 s += ',';
    s += String((r.flags & 0x01) ? 1 : 0); s += ',';
    s += String((r.flags & 0x02) ? 1 : 0); s += ',';
    s += String((r.flags & 0x04) ? 1 : 0); s += ',';
    s += String((r.flags & 0x08) ? 1 : 0); s += ',';
    s += String((r.flags & 0x10) ? 1 : 0); s += ',';
    s += String((r.flags & 0x20) ? 1 : 0); s += ',';
    s += String((r.flags & 0x40) ? 1 : 0); s += ',';
    s += String((r.flags & 0x80) ? 1 : 0);
    for (uint8_t k = 0; k < IBUS_CHANNELS; k++) { s += ','; s += String(r.ch[k]); }
    #define CSVF(v,dp) do { s += ','; s += String((v), (dp)); } while(0)
    CSVF(r.rcThrottle,4); CSVF(r.thrRaw,4); CSVF(r.thrShaped,4); CSVF(r.rcRoll,4); CSVF(r.rcPitch,4); CSVF(r.rcYaw,4);
    CSVF(r.estRoll,3); CSVF(r.estPitch,3); CSVF(r.estYaw,3); CSVF(r.ctrlRoll,3); CSVF(r.ctrlPitch,3); CSVF(r.ctrlYaw,3); CSVF(r.offRoll,3); CSVF(r.offPitch,3); CSVF(r.offYaw,3);
    CSVF(r.rawAx,5); CSVF(r.rawAy,5); CSVF(r.rawAz,5); CSVF(r.rawGx,3); CSVF(r.rawGy,3); CSVF(r.rawGz,3); CSVF(r.filtAx,5); CSVF(r.filtAy,5); CSVF(r.filtAz,5); CSVF(r.filtGx,3); CSVF(r.filtGy,3); CSVF(r.filtGz,3);
    CSVF(r.mx,3); CSVF(r.my,3); CSVF(r.mz,3); CSVF(r.magNorm,3); s += ','; s += String(r.ahrsMode); s += ','; s += String(r.ekfMagUsed ? 1 : 0); CSVF(r.ekfBgx,5); CSVF(r.ekfBgy,5); CSVF(r.ekfBgz,5);
    CSVF(r.targetRollDeg,3); CSVF(r.targetPitchDeg,3); CSVF(r.targetYawDeg,3); CSVF(r.targetRollRateDps,3); CSVF(r.targetPitchRateDps,3); CSVF(r.targetYawRateDps,3);
    CSVF(r.angleErrRollDeg,3); CSVF(r.angleErrPitchDeg,3); CSVF(r.yawErrDeg,3); CSVF(r.rateErrRollDps,3); CSVF(r.rateErrPitchDps,3); CSVF(r.rateErrYawDps,3);
    CSVF(r.rO,5); CSVF(r.pO,5); CSVF(r.yO,5);
    CSVF(r.mFL,4); CSVF(r.mFR,4); CSVF(r.mRL,4); CSVF(r.mRR,4); CSVF(r.cmdRpmFL,0); CSVF(r.cmdRpmFR,0); CSVF(r.cmdRpmRL,0); CSVF(r.cmdRpmRR,0); CSVF(r.actualRpmFL,0); CSVF(r.actualRpmFR,0); CSVF(r.actualRpmRL,0); CSVF(r.actualRpmRR,0);
    s += ','; s += String(r.notchEnable ? 1 : 0); CSVF(r.notchFreqHz,2); CSVF(r.notchQ,2); CSVF(r.bmpAltM,3); CSVF(r.bmpVzMps,3); CSVF(r.cpu0Pct,1); CSVF(r.cpu1Pct,1);
    #undef CSVF
    s += '\n';
    return s;
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
    attitudeEKF.reset();
    g_yawHoldActive = false;

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
            attitudeEKF.reset();
            g_yawHoldActive = false;
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

        // ── Welford timing accumulator (Test 7.1) ────────────
        if (xSemaphoreTake(g_timingMutex, 0) == pdTRUE) {
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

            xSemaphoreGive(g_timingMutex);
        }

        // ── Calibration active: keep motors off and skip flight control ──
        if (calManager.shouldBlockFlight()) {
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

            g_yawHoldActive = false;

            updateExecTimingAndPrint(0, 0, periodUs, TARGET_US);

            continue;
        }

        // ── IMU read + EKF AHRS ───────────────────────────────
        MPU_SensorData s;
        AttitudeEstimate   att;
        uint32_t phaseStartUs = micros();
        bool imuOk = imu.readScaled(s);
        g_execTiming.lastImuUs = micros() - phaseStartUs;

        float gxf = 0, gyf = 0, gzf = 0;
        float filtAx = 0.0f, filtAy = 0.0f, filtAz = 0.0f;
        float accelRollDeg = 0.0f, accelPitchDeg = 0.0f;
        float rollAngleErrDeg = 0.0f, pitchAngleErrDeg = 0.0f;
        if (imuOk) {
            // Apply tunable motor-vibration notch before EKF and before PID.
            // Keep original `s` for raw telemetry/logging; use `sf` for control.
            MPU_SensorData sf = s;
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

                Serial.printf("[DYN_FFT_INPUT] pushes=%lu motorOut=%d gyroMag=%.3f dps accelVib=%.4f g\n",
                            (unsigned long)dynFftPushCount,
                            g_motorOutputsActive ? 1 : 0,
                            gyroMag,
                            accelVib);
            }
            // TEMP DEBUG: prove dynamic notch / spectrum hook is running.
            // Prints only once per second so it does not destroy 400 Hz timing.
            static uint32_t lastDynNotchPrintMs = 0;
            uint32_t nowDynNotchMs = millis();

            if (nowDynNotchMs - lastDynNotchPrintMs >= 1000) {
                lastDynNotchPrintMs = nowDynNotchMs;

    Serial.printf("[DYN_NOTCH_HOOK] motorOut=%d notchEnable=%d notchHz=%.1f Q=%.1f\n",
                  g_motorOutputsActive ? 1 : 0,
                  g_tuning.notch_enable ? 1 : 0,
                  g_tuning.notch_freq_hz,
                  g_tuning.notch_q);
}
            AHRSInput ekfIn;
            ekfIn.ax_g = sf.ax_g; ekfIn.ay_g = sf.ay_g; ekfIn.az_g = sf.az_g;
            ekfIn.gx_dps = sf.gx_dps; ekfIn.gy_dps = sf.gy_dps; ekfIn.gz_dps = sf.gz_dps;
            ekfIn.mx_uT = s.mx_uT; ekfIn.my_uT = s.my_uT; ekfIn.mz_uT = s.mz_uT;
            ekfIn.magValid = imu.isMagConnected();

            // Runtime-selectable attitude estimator.
            // 0 = EKF, 1 = Mahony, 2 = Madgwick.
            static uint8_t lastAhrsMode = 255;
            uint8_t ahrsMode = 0;
            if (xSemaphoreTake(g_tuneMutex, 0) == pdTRUE) {
                ahrsMode = (uint8_t)constrain((int)roundf(g_tuning.ahrs_filter_mode), 0, 2);
                xSemaphoreGive(g_tuneMutex);
            }
            if (ahrsMode != lastAhrsMode) {
                attitudeEKF.reset();
                madgwickAHRS.reset();
                lastAhrsMode = ahrsMode;
            }
            g_ahrsFilterModeActive = ahrsMode;

            if (ahrsMode == 1) {
                mahony.update(sf, dt, att);
            } else if (ahrsMode == 2) {
                AttitudeEstimate madOut;
                madgwickAHRS.update(ekfIn, dt, madOut);
                att.roll_deg = madOut.roll_deg; att.pitch_deg = madOut.pitch_deg; att.yaw_deg = madOut.yaw_deg;
                att.q0 = madOut.q0; att.q1 = madOut.q1; att.q2 = madOut.q2; att.q3 = madOut.q3;
            } else {
                AttitudeEstimate ekfOut;
                attitudeEKF.update(ekfIn, dt, ekfOut);
                att.roll_deg = ekfOut.roll_deg; att.pitch_deg = ekfOut.pitch_deg; att.yaw_deg = ekfOut.yaw_deg;
                att.q0 = ekfOut.q0; att.q1 = ekfOut.q1; att.q2 = ekfOut.q2; att.q3 = ekfOut.q3;
            }

            gxf = lpfGx.apply(sf.gx_dps, dt, GYRO_LPF_HZ);
            gyf = lpfGy.apply(sf.gy_dps, dt, GYRO_LPF_HZ);
            gzf = lpfGz.apply(sf.gz_dps, dt, GYRO_LPF_HZ);

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
        g_execTiming.lastRcUs = micros() - phaseStartUs;
 
        float rollCtrlDeg = 0.0f, pitchCtrlDeg = 0.0f, yawCtrlDeg = 0.0f;
        if (imuOk) computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);

        // Level-zero capture: DISARMED + SWB/ACRO high.
        // Uses raw CH8 because cmd.mode is DISARMED while disarmed, even if SWB is high.
        static bool levelZeroWasHigh = false;
        static bool levelZeroCapturing = false;
        static uint16_t levelZeroCount = 0;
        static uint32_t levelZeroStartMs = 0;
        static double levelRollSum = 0.0, levelPitchSum = 0.0, levelYawSum = 0.0;
        const bool swbAcroHigh = (cmd.valid && cmd.raw[RC_CH_AUX2] >= LEVEL_ZERO_SWB_THRESHOLD);
        const bool canCaptureLevelZero = (cmd.mode == FlightMode::DISARMED && imuOk && swbAcroHigh);

        if (!swbAcroHigh) {
            levelZeroWasHigh = false;
            levelZeroCapturing = false;
            levelZeroCount = 0;
            levelZeroStartMs = 0;
            levelRollSum = levelPitchSum = levelYawSum = 0.0;
        } else if (canCaptureLevelZero && !levelZeroWasHigh) {
            if (!levelZeroCapturing) {
                levelZeroCapturing = true;
                levelZeroCount = 0;
                levelZeroStartMs = millis();
                levelRollSum = levelPitchSum = levelYawSum = 0.0;
                Serial.println(F("[LEVEL] Capturing software zero — keep drone still..."));
                calLog("[LEVEL] Capturing software zero — keep drone still...");
            }

            levelRollSum  += att.roll_deg;
            levelPitchSum += att.pitch_deg;
            levelYawSum   += att.yaw_deg;
            levelZeroCount++;

            if ((millis() - levelZeroStartMs) >= LEVEL_ZERO_SAMPLE_MS && levelZeroCount > 0) {
                g_levelRollOffsetDeg  = (float)(levelRollSum  / (double)levelZeroCount);
                g_levelPitchOffsetDeg = (float)(levelPitchSum / (double)levelZeroCount);
                g_levelYawOffsetDeg   = (float)(levelYawSum   / (double)levelZeroCount);
                computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);

                Serial.printf("[LEVEL] Zero saved: rollOff=%+.2f pitchOff=%+.2f yawOff=%+.2f deg\n",
                              g_levelRollOffsetDeg, g_levelPitchOffsetDeg, g_levelYawOffsetDeg);
                calLogf("[LEVEL] Zero saved: R=%+.2f P=%+.2f Y=%+.2f deg",
                        g_levelRollOffsetDeg, g_levelPitchOffsetDeg, g_levelYawOffsetDeg);

                levelZeroWasHigh = true;
                levelZeroCapturing = false;
                levelZeroCount = 0;
                levelZeroStartMs = 0;
                levelRollSum = levelPitchSum = levelYawSum = 0.0;
            }
        }

        // ── DISARMED / FAILSAFE ───────────────────────────────
        if (cmd.mode == FlightMode::DISARMED || cmd.mode == FlightMode::FAILSAFE) {
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
            lpfSpRoll.reset(); lpfSpPitch.reset(); lpfSpYaw.reset();
            pidAngleYaw.reset();
            g_yawHoldActive = false;

            phaseStartUs = micros();
            if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
                g_state.pidRollOut = g_state.pidPitchOut = g_state.pidYawOut = 0;
                g_state.rc      = cmd;
                if (imuOk) {
                    g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                    g_state.yaw_deg  = att.yaw_deg;
                    g_state.imuValid = imuOk;
                    g_state.magValid = imu.isMagConnected();
                    g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                    g_state.roll_ctrl_deg = rollCtrlDeg;
                    g_state.pitch_ctrl_deg = pitchCtrlDeg;
                    g_state.yaw_ctrl_deg = yawCtrlDeg;
                    g_state.roll_offset_deg = g_levelRollOffsetDeg;
                    g_state.pitch_offset_deg = g_levelPitchOffsetDeg;
                    g_state.yaw_offset_deg = g_levelYawOffsetDeg;
                    g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                    g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                    g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                    g_state.accelRoll_deg = accelRollDeg;
                    g_state.accelPitch_deg = accelPitchDeg;
                    g_state.gyroRoll_deg = g_gyroRollDeg;
                    g_state.gyroPitch_deg = g_gyroPitchDeg;
                    g_state.gyroYaw_deg = g_gyroYawDeg;
                    g_state.rollAngleError_deg = rollAngleErrDeg;
                    g_state.pitchAngleError_deg = pitchAngleErrDeg;
                    g_state.loopCount++;
                }
                xSemaphoreGive(g_flightMutex);
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
        if (xSemaphoreTake(g_tuneMutex, 0) == pdTRUE) {
            tune = g_tuning;
            xSemaphoreGive(g_tuneMutex);
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

        const float MAX_ANGLE_DEG      = tune.max_angle_deg;
        const float MAX_RATE_DPS       = tune.max_rate_dps;
        const float MAX_PITCH_RATE_DPS = tune.max_pitch_rate_dps;
        float rO=0, pO=0, yO=0;
        float targetRollDeg = 0.0f, targetPitchDeg = 0.0f, targetYawDeg = 0.0f;
        float targetRollRateDps = 0.0f, targetPitchRateDps = 0.0f, targetYawRateDps = 0.0f;
        float angleErrRollDeg = 0.0f, angleErrPitchDeg = 0.0f, yawErrDeg = 0.0f;
        float rateErrRollDps = 0.0f, rateErrPitchDps = 0.0f, rateErrYawDps = 0.0f;

        float rollCmd  = lpfSpRoll .apply(cmd.roll,  dt, RC_LPF_HZ);
        float pitchCmd = lpfSpPitch.apply(cmd.pitch, dt, RC_LPF_HZ);
        float yawCmd   = lpfSpYaw  .apply(cmd.yaw,   dt, RC_LPF_HZ);

        if (cmd.mode == FlightMode::ANGLE) {
            targetRollDeg = rollCmd * MAX_ANGLE_DEG;
            targetPitchDeg = pitchCmd * MAX_ANGLE_DEG;
            angleErrRollDeg = targetRollDeg - roll;
            angleErrPitchDeg = targetPitchDeg - pitch;
            targetRollRateDps = pidAngleRoll.update(angleErrRollDeg, dt);
            targetPitchRateDps = pidAnglePitch.update(angleErrPitchDeg, dt);
            rateErrRollDps = targetRollRateDps - gx;
            rateErrPitchDps = targetPitchRateDps - gy;
            rO = pidRateRoll.update(rateErrRollDps, dt);
            pO = pidRatePitch.update(rateErrPitchDps, dt);
        } else {   // ACRO
            targetRollRateDps = rollCmd * MAX_RATE_DPS;
            targetPitchRateDps = pitchCmd * MAX_PITCH_RATE_DPS;
            rateErrRollDps = targetRollRateDps - gx;
            rateErrPitchDps = targetPitchRateDps - gy;
            rO = pidRateRoll.update(rateErrRollDps, dt);
            pO = pidRatePitch.update(rateErrPitchDps, dt);
        }

        // Yaw: heading-hold when stick centered, rate when moving.
        if (imuOk && fabsf(yawCmd) < tune.yaw_deadband) {
            if (!g_yawHoldActive) {
                g_yawSetpoint   = yawCtrlDeg;
                g_yawHoldActive = true;
                pidAngleYaw.reset();
            }
            targetYawDeg = g_yawSetpoint;
            yawErrDeg = wrapDeg180(g_yawSetpoint - yawCtrlDeg);
            targetYawRateDps = pidAngleYaw.update(yawErrDeg, dt);
            targetYawRateDps = constrain(targetYawRateDps, -tune.yaw_max_rate_dps, tune.yaw_max_rate_dps);
            rateErrYawDps = targetYawRateDps - gz;
            yO = pidRateYaw.update(rateErrYawDps, dt);
        } else {
            g_yawHoldActive = false;
            targetYawDeg = yawCtrlDeg;
            targetYawRateDps = -yawCmd * tune.yaw_max_rate_dps;
            rateErrYawDps = targetYawRateDps - gz;
            yO = pidRateYaw.update(rateErrYawDps, dt);
        }

        rO = constrain(rO, -tune.roll_output_limit,  tune.roll_output_limit);
        pO = constrain(pO, -tune.pitch_output_limit, tune.pitch_output_limit);
        yO = constrain(yO, -tune.yaw_output_limit,   tune.yaw_output_limit);

        // ── Throttle expo + smoothing ─────────────────────────
        static float thrSmooth = 0.0f;

        const float THROTTLE_EXPO              = tune.throttle_expo;
        const float THROTTLE_UP_RATE_PER_SEC   = tune.throttle_up_rate_per_sec;
        const float THROTTLE_DOWN_RATE_PER_SEC = tune.throttle_down_rate_per_sec;
        const float MOTOR_IDLE                 = tune.motor_idle;
        const float MOTOR_MAX                  = tune.motor_max;
        const float THROTTLE_CUT               = tune.throttle_cut;
        const float IDLE_RAMP_END              = tune.idle_ramp_end;

        float thrRaw = constrain(cmd.throttle, 0.0f, 1.0f);

        float thrTarget = 0.0f;
        if (thrRaw > THROTTLE_CUT) {
            thrTarget = throttleExpo(thrRaw, THROTTLE_EXPO);
        }

        float maxStepUp   = THROTTLE_UP_RATE_PER_SEC * dt;
        float maxStepDown = THROTTLE_DOWN_RATE_PER_SEC * dt;

        if (thrTarget > thrSmooth) {
            thrSmooth += min(thrTarget - thrSmooth, maxStepUp);
        } else {
            thrSmooth -= min(thrSmooth - thrTarget, maxStepDown);
        }

        float thr = constrain(thrSmooth, 0.0f, 1.0f);

        float fl = thr + rO - pO - yO;
        float fr = thr - rO - pO + yO;
        float rl = thr + rO + pO + yO;
        float rr = thr - rO + pO - yO;

        // Desaturate high side first
        bool motorSaturated = false;
        float maxMotor = max(max(fl, fr), max(rl, rr));
        if (maxMotor > MOTOR_MAX) {
            motorSaturated = true;
            float excess = maxMotor - MOTOR_MAX;
            fl -= excess; fr -= excess; rl -= excess; rr -= excess;
        }

            Serial.printf("[MOTOR_SAT] thr=%.3f rO=%.3f pO=%.3f yO=%.3f "
                  "maxBefore=%.3f MOTOR_MAX=%.3f "
                  "FL=%.3f FR=%.3f RL=%.3f RR=%.3f\n",
                  thr, rO, pO, yO,
                  maxMotor, MOTOR_MAX,
                  fl, fr, rl, rr);

        float idleBlend = smoothStep01((thr - THROTTLE_CUT) / (IDLE_RAMP_END - THROTTLE_CUT));
        float motorMin = MOTOR_IDLE * idleBlend;

        if (thr > THROTTLE_CUT) {
            fl = constrain(fl, motorMin, MOTOR_MAX);
            fr = constrain(fr, motorMin, MOTOR_MAX);
            rl = constrain(rl, motorMin, MOTOR_MAX);
            rr = constrain(rr, motorMin, MOTOR_MAX);
        } else {
            fl = fr = rl = rr = 0.0f;
        }

        // phaseStartUs = micros();
        // writeMotors(fl, fr, rl, rr);
        // g_motorOutputsActive = true;
        // g_execTiming.lastMotorUs = micros() - phaseStartUs;
        // uint32_t controlDoneUs = micros();

                // ── Motor PWM update rate limiter ───────────────────────────
        // Control loop still runs at 400 Hz.
        // ESC/motor PWM writes run at 200 Hz to avoid blocking every cycle.
        static uint32_t lastMotorWriteUs = 0;
        static uint32_t motorWriteCount = 0;
        static uint32_t motorSkipCount = 0;
        static uint32_t lastMotorRatePrintMs = 0;

        constexpr uint32_t MOTOR_UPDATE_PERIOD_US = 5000; // 200 Hz

        bool motorWriteDue =
            (lastMotorWriteUs == 0) ||
            ((uint32_t)(nowUs - lastMotorWriteUs) >= MOTOR_UPDATE_PERIOD_US);

        phaseStartUs = micros();

        if (motorWriteDue) {
            writeMotors(fl, fr, rl, rr);
            g_motorOutputsActive = true;
            lastMotorWriteUs = nowUs;
            motorWriteCount++;
        } else {
            motorSkipCount++;
        }

        g_execTiming.lastMotorUs = micros() - phaseStartUs;

        uint32_t motorRateNowMs = millis();
        if (motorRateNowMs - lastMotorRatePrintMs >= 1000) {
            lastMotorRatePrintMs = motorRateNowMs;

            Serial.printf("[MOTOR_RATE] target=200Hz writes=%lu skips=%lu lastMotorUs=%lu\n",
                        (unsigned long)motorWriteCount,
                        (unsigned long)motorSkipCount,
                        (unsigned long)g_execTiming.lastMotorUs);

            motorWriteCount = 0;
            motorSkipCount = 0;
        }

        uint32_t controlDoneUs = micros();


        const float magNorm_uT = imuOk ? sqrtf(s.mx_uT*s.mx_uT + s.my_uT*s.my_uT + s.mz_uT*s.mz_uT) : 0.0f;
        const float rpmScaleLog = MOTOR_KV * BATTERY_VOLTAGE;
        const float cmdRpmFL = fl * rpmScaleLog;
        const float cmdRpmFR = fr * rpmScaleLog;
        const float cmdRpmRL = rl * rpmScaleLog;
        const float cmdRpmRR = rr * rpmScaleLog;

        // ── High-speed flight log @ 100 Hz (every 4th cycle) ──
        static uint8_t logDiv = 0;
        if (++logDiv >= 4) {
            logDiv = 0;
            FlightLogRow row;
            memset(&row, 0, sizeof(row));
            row.t_us       = nowUs;
            row.loop_count = g_state.loopCount;
            auto clampU16 = [](uint32_t v) -> uint16_t {
                return (uint16_t)((v > 65535UL) ? 65535UL : v);
            };
            row.period_us  = clampU16(periodUs);
            int32_t jitSigned = (int32_t)periodUs - (int32_t)TARGET_US;
            row.jitter_us  = (int16_t)constrain(jitSigned, -32768, 32767);
            row.control_exec_us = clampU16(controlDoneUs - execStartUs);

            // Copy volatile timing values into non-volatile locals before clamping.
            // This avoids C++ template deduction errors from min(volatile uint32_t&, uint32_t).
            uint32_t imuReadUs   = g_execTiming.lastImuUs;
            uint32_t rcReadUs    = g_execTiming.lastRcUs;
            uint32_t motorExecUs = g_execTiming.lastMotorUs;
            row.imu_read_us    = clampU16(imuReadUs);
            row.rc_read_us     = clampU16(rcReadUs);
            row.motor_exec_us  = clampU16(motorExecUs);
            row.mode      = (uint8_t)cmd.mode;
            bool magOkNow = imuOk && imu.isMagConnected();
            row.flags     = 0x01
                          | (imuOk ? 0x02 : 0x00)
                          | (cmd.valid ? 0x04 : 0x00)
                          | (magOkNow ? 0x08 : 0x00)
                          | (g_state.bmpValid ? 0x10 : 0x00)
                          | (g_state.gps.valid ? 0x20 : 0x00)
                          | (motorSaturated ? 0x40 : 0x00);
            for (uint8_t k = 0; k < IBUS_CHANNELS; k++) row.ch[k] = cmd.raw[k];
            row.rcThrottle = cmd.throttle;
            row.thrRaw = thrRaw;
            row.thrShaped = thr;
            row.rcRoll = rollCmd; row.rcPitch = pitchCmd; row.rcYaw = yawCmd;
            row.estRoll = imuOk ? att.roll_deg : 0.0f; row.estPitch = imuOk ? att.pitch_deg : 0.0f; row.estYaw = imuOk ? att.yaw_deg : 0.0f;
            row.ctrlRoll = roll; row.ctrlPitch = pitch; row.ctrlYaw = imuOk ? yawCtrlDeg : 0.0f;
            row.offRoll = g_levelRollOffsetDeg; row.offPitch = g_levelPitchOffsetDeg; row.offYaw = g_levelYawOffsetDeg;
            row.rawAx = imuOk ? s.ax_g : 0.0f; row.rawAy = imuOk ? s.ay_g : 0.0f; row.rawAz = imuOk ? s.az_g : 0.0f;
            row.rawGx = imuOk ? s.gx_dps : 0.0f; row.rawGy = imuOk ? s.gy_dps : 0.0f; row.rawGz = imuOk ? s.gz_dps : 0.0f;
            row.filtAx = filtAx; row.filtAy = filtAy; row.filtAz = filtAz;
            row.filtGx = gx; row.filtGy = gy; row.filtGz = gz;
            row.mx = imuOk ? s.mx_uT : 0.0f; row.my = imuOk ? s.my_uT : 0.0f; row.mz = imuOk ? s.mz_uT : 0.0f; row.magNorm = magNorm_uT;
            row.ahrsMode = g_ahrsFilterModeActive;
            row.ekfMagUsed = attitudeEKF.lastMagAccepted();
            row.ekfBgx = attitudeEKF.rollBiasDps(); row.ekfBgy = attitudeEKF.pitchBiasDps(); row.ekfBgz = attitudeEKF.yawBiasDps();
            row.targetRollDeg = targetRollDeg; row.targetPitchDeg = targetPitchDeg; row.targetYawDeg = targetYawDeg;
            row.targetRollRateDps = targetRollRateDps; row.targetPitchRateDps = targetPitchRateDps; row.targetYawRateDps = targetYawRateDps;
            row.angleErrRollDeg = angleErrRollDeg; row.angleErrPitchDeg = angleErrPitchDeg; row.yawErrDeg = yawErrDeg;
            row.rateErrRollDps = rateErrRollDps; row.rateErrPitchDps = rateErrPitchDps; row.rateErrYawDps = rateErrYawDps;
            row.rO = rO; row.pO = pO; row.yO = yO;
            row.mFL = fl; row.mFR = fr; row.mRL = rl; row.mRR = rr;
            row.cmdRpmFL = cmdRpmFL; row.cmdRpmFR = cmdRpmFR; row.cmdRpmRL = cmdRpmRL; row.cmdRpmRR = cmdRpmRR;
            row.actualRpmFL = 0.0f; row.actualRpmFR = 0.0f; row.actualRpmRL = 0.0f; row.actualRpmRR = 0.0f;
            row.notchEnable = tune.notch_enable; row.notchFreqHz = tune.notch_freq_hz; row.notchQ = tune.notch_q;
            row.bmpAltM = g_state.bmpAltitude_m; row.bmpVzMps = g_state.bmpVerticalSpeed_mps;
            row.cpu0Pct = g_state.cpuCore0_pct; row.cpu1Pct = g_state.cpuCore1_pct;
            pushFlightLog(row);
        }

        // ── Publish flight state (incl. true PID outputs) ─────
        phaseStartUs = micros();
        if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE){
            if (imuOk) {
                g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                g_state.yaw_deg  = att.yaw_deg;
                g_state.imuValid = imuOk;
                g_state.magValid = imu.isMagConnected();
                g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                g_state.roll_ctrl_deg = rollCtrlDeg;
                g_state.pitch_ctrl_deg = pitchCtrlDeg;
                g_state.yaw_ctrl_deg = yawCtrlDeg;
                g_state.roll_offset_deg = g_levelRollOffsetDeg;
                g_state.pitch_offset_deg = g_levelPitchOffsetDeg;
                g_state.yaw_offset_deg = g_levelYawOffsetDeg;
                g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                g_state.rawAx_g=s.ax_g; g_state.rawAy_g=s.ay_g; g_state.rawAz_g=s.az_g;
                g_state.rawGx_dps=s.gx_dps; g_state.rawGy_dps=s.gy_dps; g_state.rawGz_dps=s.gz_dps;
                g_state.filtAx_g=filtAx; g_state.filtAy_g=filtAy; g_state.filtAz_g=filtAz;
                g_state.filtGx_dps=gx; g_state.filtGy_dps=gy; g_state.filtGz_dps=gz;
                g_state.magNorm_uT=magNorm_uT;
                g_state.ekfMagUsed=attitudeEKF.lastMagAccepted();
                g_state.ekfBgx_dps=attitudeEKF.rollBiasDps();
                g_state.ekfBgy_dps=attitudeEKF.pitchBiasDps();
                g_state.ekfBgz_dps=attitudeEKF.yawBiasDps();
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
            g_state.motorFL=fl; g_state.motorFR=fr;
            g_state.motorRL=rl; g_state.motorRR=rr;
            g_state.pidRollOut=rO; g_state.pidPitchOut=pO; g_state.pidYawOut=yO;
            g_state.armed=true; g_state.rc=cmd;
            xSemaphoreGive(g_flightMutex);
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

    Serial.println(F("\n╔══════════════════════════════════════════════════════╗"));
    Serial.println(F("  ║  FlySky iBUS + MPU-9250/6500 + BMP280 + GPS  v4.0.0  ║"));
    Serial.println(F("  ║  Wi-Fi: ESP32-DRONE / 12345678 → 192.168.4.1         ║"));
    Serial.println(F("  ║  taskControl: timer 400 Hz, original RC              ║"));
    Serial.println(F("  ║  Type 'p' to toggle the [PID] tuning trace.          ║"));
    Serial.println(F("  ╚══════════════════════════════════════════════════════╝"));

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

            Serial.printf("[CAL-MGR] run=%lu state=%u active=%d safe=%d confirm=%d progress=%.2f msg=%s\n",
                        (unsigned long)calStatus.runId,
                        (unsigned)calStatus.state,
                        calStatus.active ? 1 : 0,
                        calStatus.safeToRun ? 1 : 0,
                        calStatus.requiresUserConfirm ? 1 : 0,
                        calStatus.progress,
                        calStatus.message);
        }

        // Handle inbound Serial commands (non-blocking)
        while (Serial.available() > 0) {
            int c = Serial.read();
            if (c == 'p' || c == 'P') {
                g_pidTrace = !g_pidTrace;
                Serial.printf("[CMD] PID trace %s\n", g_pidTrace ? "ON" : "OFF");
            }
        }

        if (g_calibState != CalibState::IDLE && g_calibState != CalibState::DONE) {
            tick++; vTaskDelayUntil(&lastWake, period); continue;
        }

        // Snapshot once per loop; both blocks below use it.
        FlightState s;
        if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            s = g_state; xSemaphoreGive(g_flightMutex);
        }
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
            Serial.printf("[%6lu] %s | RAW R=%+6.1f P=%+6.1f Y=%6.1f | "
                          "CTRL R=%+6.1f P=%+6.1f Y=%+6.1f | "
                          "OFF R=%+5.2f P=%+5.2f Y=%+5.2f | "
                          "MOT %.2f %.2f %.2f %.2f | "
                          "BMP %.1fC %.1fhPa %.1fm | "
                          "GPS %s Sats=%d | "
                          "Mag:%s Est:%s mKp=%.2f\n",
                          (unsigned long)tick, ms,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.roll_offset_deg, s.pitch_offset_deg, s.yaw_offset_deg,
                          s.motorFL, s.motorFR, s.motorRL, s.motorRR,
                          s.bmpTemp_c, s.bmpPressure_hpa, s.bmpAltitude_m,
                          s.gps.valid ? "FIX" : "---", s.gps.satellites,
                          imu.isMagConnected() ? "9DOF" : "6DOF",
                          (s.ahrsFilterMode == 1) ? "Mahony" : ((s.ahrsFilterMode == 2) ? "Madgwick" : "EKF"),
                          mahony.kp());
            Serial.printf("          CH: 1=%4u 2=%4u 3=%4u 4=%4u 5=%4u 6=%4u 7=%4u 8=%4u 9=%4u 10=%4u\n",
                          s.rc.raw[0], s.rc.raw[1], s.rc.raw[2], s.rc.raw[3], s.rc.raw[4],
                          s.rc.raw[5], s.rc.raw[6], s.rc.raw[7], s.rc.raw[8], s.rc.raw[9]);
            
            float magNorm = sqrtf(s.mx_uT * s.mx_uT +
                      s.my_uT * s.my_uT +
                      s.mz_uT * s.mz_uT);

            Serial.printf("          SENS: acc=%+.4f %+.4f %+.4f g | "
                        "gyro=%+.4f %+.4f %+.4f dps | "
                        "mag=%+.2f %+.2f %+.2f uT | norm=%.2f uT\n",
                        s.ax_g, s.ay_g, s.az_g,
                        s.gx_dps, s.gy_dps, s.gz_dps,
                        s.mx_uT, s.my_uT, s.mz_uT,
                        magNorm);
        }

        // ~4 Hz PID tuning trace (sibling of the 1 Hz block; gated by 'p')
        if (g_pidTrace && s.armed && (tick % 5 == 0)) {
            Serial.printf("[PID] %s | cmd r=%+.2f p=%+.2f y=%+.2f | "
                          "ctrlAtt R=%+6.2f P=%+6.2f Y=%+6.2f | "
                          "gyro gx=%+7.1f gy=%+7.1f gz=%+7.1f | "
                          "out rO=%+.4f pO=%+.4f yO=%+.4f | "
                          "yawSP=%+6.1f hold=%d\n",
                          ms, s.rc.roll, s.rc.pitch, s.rc.yaw,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.gx_dps, s.gy_dps, s.gz_dps,
                          s.pidRollOut, s.pidPitchOut, s.pidYawOut,
                          g_yawSetpoint, (int)g_yawHoldActive);
        }

        tick++;
        vTaskDelayUntil(&lastWake, period);
    }
}

// ─────────────────────────────────────────────────────────────
//  taskBMP — Core 0, priority 1, 20 Hz
// ─────────────────────────────────────────────────────────────
static void taskBMP(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();
    bool bmpVzInit = false;
    float prevBmpAltM = 0.0f;
    uint32_t prevBmpMs = 0;

    for (;;) {
        BMP280Data b;
        if (bmp280.read(b)) {
            uint32_t nowMsBmp = millis();
            float bmpVz = 0.0f;
            if (b.valid && bmpVzInit && nowMsBmp > prevBmpMs) {
                float dtBmp = (nowMsBmp - prevBmpMs) * 0.001f;
                bmpVz = (b.altitude_m - prevBmpAltM) / dtBmp;
            }
            if (b.valid) { prevBmpAltM = b.altitude_m; prevBmpMs = nowMsBmp; bmpVzInit = true; }

            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpTemp_c      = b.temperature_c;
                g_state.bmpPressure_hpa = b.pressure_hpa;
                g_state.bmpAltitude_m  = b.altitude_m;
                g_state.bmpVerticalSpeed_mps = bmpVz;
                g_state.bmpValid       = b.valid;
                xSemaphoreGive(g_flightMutex);
            }
        } else {
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpValid = false;
                g_state.bmpVerticalSpeed_mps = 0.0f;
                xSemaphoreGive(g_flightMutex);
            }
        }
        vTaskDelayUntil(&lastWake, period);
    }
}

// ─────────────────────────────────────────────────────────────
//  taskCPU — Core 0, priority 1, 2 Hz
// ─────────────────────────────────────────────────────────────
static void taskCPU(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(500);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        cpuUtilization.update();
        CPUUtilizationData c = cpuUtilization.get();
        if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
            g_state.cpuCore0_pct = c.core0_pct;
            g_state.cpuCore1_pct = c.core1_pct;
            g_state.cpuValid     = c.valid;
            xSemaphoreGive(g_flightMutex);
        }
        vTaskDelayUntil(&lastWake, period);
    }
}


// ─────────────────────────────────────────────────────────────
//  Wi-Fi spectrum provider — /spectrum endpoint
// ─────────────────────────────────────────────────────────────
static String provideSpectrumJson()
{
    TuningState t;
    memset(&t, 0, sizeof(t));
    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        t = g_tuning;
        xSemaphoreGive(g_tuneMutex);
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

    if (!g_flightMutex) return false;
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) != pdTRUE) return false;
    s = g_state;
    xSemaphoreGive(g_flightMutex);

    const float motorMax = max(max(s.motorFL, s.motorFR), max(s.motorRL, s.motorRR));
    const bool throttleLow = s.rc.throttle <= 0.03f;
    const bool motorsOff   = motorMax <= 0.001f;

    return (!s.armed) && throttleLow && motorsOff;
}

// ─────────────────────────────────────────────────────────────
//  taskWiFi — Core 0, priority 1, event-driven
// ─────────────────────────────────────────────────────────────
static void taskWiFi(void* /*pv*/)
{
    telemetryWiFi.setTelemetryProvider(provideTelemetry);
    telemetryWiFi.setTuneHandler(handleTune);
    telemetryWiFi.setOtaAllowedProvider(otaIsSafeToStart);

    telemetryWiFi.setTimingProvider(provideTimingJson);
    telemetryWiFi.setTimingCsvProvider(provideTimingCsv);
    telemetryWiFi.setTimingResetHandler(resetTimingStats);
    telemetryWiFi.setSpectrumProvider(provideSpectrumJson);

// High-speed flight log (chunked streaming — no giant String)
    telemetryWiFi.setFlightLogCountProvider(flightLogCount);
    telemetryWiFi.setFlightLogHeaderProvider(flightLogHeader);
    telemetryWiFi.setFlightLogRowProvider(flightLogRowCsv);
    telemetryWiFi.setFlightLogResetHandler(resetFlightLog);

    telemetryWiFi.begin("ESP32-DRONE", "12345678");

    for (;;) {
        telemetryWiFi.update();
        // Give Core 0 idle time. 10 ms still supports 10–50 Hz GCS polling
        // but avoids pegging Core 0 when Wi-Fi + RC are active.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ═════════════════════════════════════════════════════════════
//  setup()
// ═════════════════════════════════════════════════════════════
void setup()
{
    Serial.begin(115200);
    delay(600);

    g_flightMutex = xSemaphoreCreateMutex();
    g_tuneMutex   = xSemaphoreCreateMutex();
    g_timingMutex = xSemaphoreCreateMutex();
    configASSERT(g_flightMutex);
    configASSERT(g_tuneMutex);
    configASSERT(g_timingMutex);

    memset(&g_state,    0, sizeof(g_state));
    memset(&g_timing,   0, sizeof(g_timing));


    motorsBegin();
    motorEscArm();   // sends 1000 µs for 3 s; comment out after ESC calibration done
    g_motorOutputsActive = false;

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_MPU_CS);
    delay(10);

    Serial.print(F("[BOOT] IMU... "));
    if (!imu.begin()) {
        Serial.println(F("FAILED. Check SPI wiring. Halting."));
        while (true) delay(1000);
    }
    Serial.println(F("OK"));
    calManager.begin(imu);
    calManager.attachMotorOutputs(writeMotors, motorsOff);
    Serial.println(F("[BOOT] CalibrationManager ready."));

    if (imu.hasMag()) {
        Serial.println(F("[BOOT] AK8963 magnetometer: DETECTED — 9-DOF AHRS"));
    } else {
        Serial.println(F("[BOOT] AK8963 magnetometer: NOT FOUND — 6-DOF AHRS"));
        Serial.println(F("[BOOT] Roll and pitch accurate. Yaw will drift."));
    }

    bmp280.scanI2C(PIN_BMP_SDA, PIN_BMP_SCL, 100000);
    Serial.print(F("[BOOT] BMP280... "));
    bmp280.beginAuto(PIN_BMP_SDA, PIN_BMP_SCL, 100000)
        ? Serial.println(F("OK")) : Serial.println(F("not found."));

    Serial.print(F("[BOOT] VL53L4CX ToF... "));
    tofSensor.begin(PIN_BMP_SDA, PIN_BMP_SCL, 400000)
        ? Serial.println(F("OK")) : Serial.println(F("disabled/not found."));

    Serial.print(F("[BOOT] CPU monitor... "));
    cpuUtilization.begin(1000)
        ? Serial.println(F("OK")) : Serial.println(F("idle-hook failed."));

    Serial.println(F("[BOOT] GPS (GY-GPS6MV2)..."));
    gps.begin(PIN_GPS_RX, PIN_GPS_TX, 9600);

    Serial.print(F("[BOOT] NVS calibration... "));
    if (imu.loadCalibration()) {
        Serial.println(F("loaded."));
        imu.printCalibration();
    } else {
        Serial.println(F("none — flip SWD UP (disarmed) to calibrate."));
    }

    rcReceiver.begin(PIN_IBUS_RX, PIN_IBUS_TX, 2);
    Serial.println(F("[BOOT] iBUS ready."));

    syncTuningFromObjects();

    xTaskCreatePinnedToCore(taskRC,      "RC",     6144,  nullptr, 3, &hTaskRC,      0);
    xTaskCreatePinnedToCore(taskSerial,  "Serial", 4096,  nullptr, 1, &hTaskSerial,  0);
    xTaskCreatePinnedToCore(taskWiFi,    "WiFi",   12288, nullptr, 1, &hTaskWiFi,    0);
    xTaskCreatePinnedToCore(taskBMP,     "BMP280", 3072,  nullptr, 1, &hTaskBMP,     0);
    xTaskCreatePinnedToCore(taskCPU,     "CPU",    3072,  nullptr, 1, &hTaskCPU,     0);
    xTaskCreatePinnedToCore(taskGPS,     "GPS",    4096,  nullptr, 1, &hTaskGPS,     0);
    xTaskCreatePinnedToCore(taskControl, "Ctrl",   10240, nullptr, 5, &hTaskControl, 1);
    hTaskIMU = hTaskControl;

    esp_timer_create_args_t controlTimerArgs;
    memset(&controlTimerArgs, 0, sizeof(controlTimerArgs));
    controlTimerArgs.callback = &controlTimerCallback;
    controlTimerArgs.arg = nullptr;
    controlTimerArgs.dispatch_method = ESP_TIMER_TASK;
    controlTimerArgs.name = "ctrl400";

    if (esp_timer_create(&controlTimerArgs, &g_controlTimer) != ESP_OK ||
        esp_timer_start_periodic(g_controlTimer, TIMING_TARGET_US) != ESP_OK) {
        Serial.println(F("[BOOT][ERROR] Failed to start 400 Hz control timer."));
        while (true) delay(1000);
    }

    Serial.println(F("[BOOT] All tasks running."));
    Serial.println(F("[BOOT] Timing target: 2500 us (400 Hz control loop)."));
    Serial.println(F("[BOOT] Ground station: http://192.168.4.1"));
    Serial.println(F("[BOOT] GPS fix takes 30-90 s outdoors."));
    Serial.printf("[BOOT] Free heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(portMAX_DELAY); }
