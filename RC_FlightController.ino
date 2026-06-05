/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v2.3.2                                  ║
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
#include "CPUUtilization.h"
#include "GPSSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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
#define TIMING_BUF_SIZE       3000     // ring buffer depth (samples)
#define TIMING_TARGET_US      2500     // nominal control period (400 Hz = 2500 µs)
#define JITTER_VIOLATION_US    100     // threshold: counts as a violation
#define GYRO_LPF_HZ           40.0f    // lower = smoother but more lag
#define RC_LPF_HZ             60.0f    // stick setpoint smoothing

// ── Pilot command limits ────────────────────────────────────
static constexpr float TUNE_MAX_ANGLE_DEG = 12.0f;
static constexpr float TUNE_MAX_RATE_DPS  = 120.0f;

// ── PID output authority limits before motor mixing ─────────
static constexpr float TUNE_ROLL_OUTPUT_LIMIT  = 0.150f;
static constexpr float TUNE_PITCH_OUTPUT_LIMIT = 0.150f;
static constexpr float TUNE_YAW_OUTPUT_LIMIT   = 0.050f;

// ── Throttle shaping + motor output limits ──────────────────
static constexpr float TUNE_THROTTLE_EXPO              = 0.35f;
static constexpr float TUNE_THROTTLE_UP_RATE_PER_SEC   = 0.50f;
static constexpr float TUNE_THROTTLE_DOWN_RATE_PER_SEC = 1.00f;
static constexpr float TUNE_MOTOR_IDLE                 = 0.08f;
static constexpr float TUNE_MOTOR_MAX                  = 0.75f;
static constexpr float TUNE_THROTTLE_CUT               = 0.03f;
static constexpr float TUNE_IDLE_RAMP_END              = 0.15f;

// ── Initial PID gains loaded at boot ────────────────────────
// Inner Loop
static constexpr float TUNE_RATE_ROLL_KP   = 0.0030f;
static constexpr float TUNE_RATE_ROLL_KI   = 0.00000f;
static constexpr float TUNE_RATE_ROLL_KD   = 0.00000f;
static constexpr float TUNE_RATE_PITCH_KP  = 0.0030f;
static constexpr float TUNE_RATE_PITCH_KI  = 0.00000f;
static constexpr float TUNE_RATE_PITCH_KD  = 0.00000f;
static constexpr float TUNE_RATE_YAW_KP    = 0.00025f;
static constexpr float TUNE_RATE_YAW_KI    = 0.0000000f;
static constexpr float TUNE_RATE_YAW_KD    = 0.0000000f;
// Outer Loop
static constexpr float TUNE_ANGLE_ROLL_KP  = 0.50f;
static constexpr float TUNE_ANGLE_ROLL_KI  = 0.000f;
static constexpr float TUNE_ANGLE_ROLL_KD  = 0.0000f;
static constexpr float TUNE_ANGLE_PITCH_KP = 0.50f;
static constexpr float TUNE_ANGLE_PITCH_KI = 0.000f;
static constexpr float TUNE_ANGLE_PITCH_KD = 0.0000f;
// Outer Loop — Yaw heading hold
static constexpr float TUNE_ANGLE_YAW_KP     = 2.00f;   // heading-hold Kp (tune up if soft)
static constexpr float TUNE_YAW_DEADBAND     = 0.05f;   // |yaw stick| below this = hold
static constexpr float TUNE_YAW_MAX_RATE_DPS = 90.0f;   // cap on commanded yaw rate

struct FlightLogRow {
    uint32_t t_us;
    uint16_t period_us;
    uint8_t  mode;        // 0=DISARM 1=ANGLE 2=ACRO 3=FAILSAFE
    uint8_t  flags;       // bit0 armed, bit1 imuOk
    float rcThrottle, thr;            // raw stick throttle, final mixed
    float rcRoll, rcPitch, rcYaw;     // smoothed setpoints (post-LPF)
    float roll, pitch, yaw;           // attitude (deg)
    float gxRaw, gyRaw, gzRaw;        // unfiltered gyro (dps)
    float gxFilt, gyFilt, gzFilt;     // filtered gyro fed to PID
    float rO, pO, yO;                 // clamped PID outputs
    float mFL, mFR, mRL, mRR;         // motor commands 0..1
};


// ─────────────────────────────────────────────────────────────
//  Calibration timing
// ─────────────────────────────────────────────────────────────
#define SWC_THRESHOLD      1700
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
//  IMU object
// ─────────────────────────────────────────────────────────────
MPU9250 imu(PIN_MPU_CS);

#define FLIGHT_LOG_SIZE 400   // 400 samples @ 100 Hz = 4 s



// ─────────────────────────────────────────────────────────────
//  Shared flight state
// ─────────────────────────────────────────────────────────────
struct FlightState {
    float roll_deg, pitch_deg, yaw_deg;
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float mx_uT, my_uT, mz_uT;
    float bmpTemp_c, bmpPressure_hpa, bmpAltitude_m;
    bool  bmpValid;
    float cpuCore0_pct, cpuCore1_pct;
    bool  cpuValid;
    float motorFL, motorFR, motorRL, motorRR;
    float pidRollOut, pidPitchOut, pidYawOut;   // NEW — true PID outputs for tuning trace
    RCCommand rc;
    bool  armed;
    uint32_t loopCount;
    GPSData gps;
};
static FlightState       g_state;
static SemaphoreHandle_t g_flightMutex;

// PID-trace toggle (Serial 'p'). Off at boot so the log stays clean.
static volatile bool g_pidTrace = false;

// ─────────────────────────────────────────────────────────────
//  Tuning state
// ─────────────────────────────────────────────────────────────
struct TuningState {
    float pid_roll_kp,  pid_roll_ki,  pid_roll_kd;
    float pid_pitch_kp, pid_pitch_ki, pid_pitch_kd;
    float pid_yaw_kp,   pid_yaw_ki,   pid_yaw_kd;
    float pid_angle_roll_kp, pid_angle_pitch_kp;
    float mahony_kp, mahony_ki;
    volatile bool dirty;
};
static TuningState       g_tuning;
static SemaphoreHandle_t g_tuneMutex;

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
    g_tuning.pid_roll_kp        = pidRateRoll.kp;
    g_tuning.pid_roll_ki        = pidRateRoll.ki;
    g_tuning.pid_roll_kd        = pidRateRoll.kd;
    g_tuning.pid_pitch_kp       = pidRatePitch.kp;
    g_tuning.pid_pitch_ki       = pidRatePitch.ki;
    g_tuning.pid_pitch_kd       = pidRatePitch.kd;
    g_tuning.pid_yaw_kp         = pidRateYaw.kp;
    g_tuning.pid_yaw_ki         = pidRateYaw.ki;
    g_tuning.pid_yaw_kd         = pidRateYaw.kd;
    g_tuning.pid_angle_roll_kp  = pidAngleRoll.kp;
    g_tuning.pid_angle_pitch_kp = pidAnglePitch.kp;
    g_tuning.mahony_kp          = imu.mahonyKp;
    g_tuning.mahony_ki          = imu.mahonyKi;
    g_tuning.dirty              = false;
}

static void applyTuningToObjects()
{
    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) != pdTRUE) return;
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
    pidAnglePitch.kp       = g_tuning.pid_angle_pitch_kp;
    imu.mahonyKp           = g_tuning.mahony_kp;
    imu.mahonyKi           = g_tuning.mahony_ki;
    g_tuning.dirty         = false;
    xSemaphoreGive(g_tuneMutex);
    calLog("[TUNE] New gains applied.");
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

    String j;
    j.reserve(320);
    j += F("{\"ok\":true");
    j += F(",\"count\":");          j += String(count);
    j += F(",\"period_mean_us\":"); j += String((float)periodMean, 2);
    j += F(",\"period_std_us\":");  j += String((float)periodStd,  2);
    j += F(",\"jitter_mean_us\":"); j += String((float)jitterMean, 2);
    j += F(",\"jitter_std_us\":");  j += String((float)jitterStd,  2);
    j += F(",\"jitter_max_us\":");  j += String(jitterMax);
    j += F(",\"jitter_p99_us\":");  j += String(p99, 1);
    j += F(",\"violations\":");     j += String(violations);
    j += F(",\"buf_samples\":");    j += String(n);
    j += F(",\"target_us\":");      j += String(TIMING_TARGET_US);
    j += F(",\"violation_thresh_us\":"); j += String(JITTER_VIOLATION_US);
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
    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        t = g_tuning; xSemaphoreGive(g_tuneMutex);
    }

    out.tick      = s.loopCount;
    out.mode      = flightModeToStr(s.rc.mode);
    out.armed     = s.armed;
    out.rc_valid  = s.rc.valid;
    out.roll_deg  = s.roll_deg;  out.pitch_deg = s.pitch_deg; out.yaw_deg = s.yaw_deg;
    out.ax_g=s.ax_g; out.ay_g=s.ay_g; out.az_g=s.az_g;
    out.gx_dps=s.gx_dps; out.gy_dps=s.gy_dps; out.gz_dps=s.gz_dps;
    out.mx_uT=s.mx_uT;   out.my_uT=s.my_uT;   out.mz_uT=s.mz_uT;
    out.throttle=s.rc.throttle; out.rc_roll=s.rc.roll;
    out.rc_pitch=s.rc.pitch;    out.rc_yaw=s.rc.yaw;
    out.rc_hz=rcReceiver.getFrameRate();
    out.motor_fl=s.motorFL; out.motor_fr=s.motorFR;
    out.motor_rl=s.motorRL; out.motor_rr=s.motorRR;
    float rpmScale = MOTOR_KV * BATTERY_VOLTAGE;
    out.rpm_fl=s.motorFL*rpmScale; out.rpm_fr=s.motorFR*rpmScale;
    out.rpm_rl=s.motorRL*rpmScale; out.rpm_rr=s.motorRR*rpmScale;
    out.bmp_temp_c=s.bmpTemp_c; out.bmp_pressure_hpa=s.bmpPressure_hpa;
    out.bmp_altitude_m=s.bmpAltitude_m; out.bmp_valid=s.bmpValid;
    out.cpu_core0_pct=s.cpuCore0_pct; out.cpu_core1_pct=s.cpuCore1_pct;
    out.cpu_valid=s.cpuValid;
    out.pid_roll_kp=t.pid_roll_kp;   out.pid_roll_ki=t.pid_roll_ki;   out.pid_roll_kd=t.pid_roll_kd;
    out.pid_pitch_kp=t.pid_pitch_kp; out.pid_pitch_ki=t.pid_pitch_ki; out.pid_pitch_kd=t.pid_pitch_kd;
    out.pid_yaw_kp=t.pid_yaw_kp;     out.pid_yaw_ki=t.pid_yaw_ki;     out.pid_yaw_kd=t.pid_yaw_kd;
    out.pid_angle_roll_kp=t.pid_angle_roll_kp;
    out.pid_angle_pitch_kp=t.pid_angle_pitch_kp;
    out.mahony_kp=t.mahony_kp; out.mahony_ki=t.mahony_ki;
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
static void handleTune(const TunePacket& in)
{
    bool armed = false;
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        armed = g_state.armed; xSemaphoreGive(g_flightMutex);
    }
    if (armed) { calLog("[TUNE] REJECTED — disarm first."); return; }

    if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(10)) != pdTRUE) return;
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
    if (in.has_pid_angle_pitch_kp) g_tuning.pid_angle_pitch_kp = in.pid_angle_pitch_kp;
    if (in.has_mahony_kp)          g_tuning.mahony_kp          = in.mahony_kp;
    if (in.has_mahony_ki)          g_tuning.mahony_ki          = in.mahony_ki;
    g_tuning.dirty = true;
    xSemaphoreGive(g_tuneMutex);
    Serial.println(F("[TUNE] Gains queued — will apply on next IMU cycle."));
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
//  taskRC — Core 0, priority 3, 200 Hz
// ─────────────────────────────────────────────────────────────
static void taskRC(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();
    bool swdPrev = false;

    for (;;) {
        rcReceiver.update();
        RCCommand cmd = rcReceiver.getCommand();

        if (cmd.swdHigh && !swdPrev) {
            if (cmd.mode == FlightMode::DISARMED &&
                (g_calibState == CalibState::IDLE || g_calibState == CalibState::DONE)) {
                g_calibState = CalibState::REQUESTED;
                calLog("[RC] Calibration requested — SWD flipped UP.");
            } else if (cmd.mode != FlightMode::DISARMED) {
                calLog("[RC] Cannot calibrate while armed.");
            }
        }
        swdPrev = cmd.swdHigh;
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

// ═════════════════════════════════════════════════════════════
//  HIGH-SPEED ON-BOARD FLIGHT LOG  (Test 7.1 / 7.3 / 7.4 capture)
//
//  Compact 92-byte row, logged at 100 Hz (every 4th 400 Hz cycle).
//  Workflow: POST /flightlog/reset  →  arm + fly  →  disarm  →
//            GET /flightlog/csv (freezes the buffer, streams it out).
//
//  FLIGHT_LOG_SIZE=800 → 8.0 s at 100 Hz → ~74 KB. Raise only if
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
    return F("t_us,period_us,mode,armed,imuOk,"
             "rcThrottle,thr,rcRoll,rcPitch,rcYaw,"
             "roll,pitch,yaw,"
             "gxRaw,gyRaw,gzRaw,gxFilt,gyFilt,gzFilt,"
             "rO,pO,yO,mFL,mFR,mRL,mRR\n");
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

    String s; s.reserve(160);
    s += String(r.t_us);                 s += ',';
    s += String(r.period_us);            s += ',';
    s += String(r.mode);                 s += ',';
    s += String((r.flags & 0x01) ? 1 : 0); s += ',';
    s += String((r.flags & 0x02) ? 1 : 0); s += ',';
    s += String(r.rcThrottle, 4);        s += ',';
    s += String(r.thr, 4);               s += ',';
    s += String(r.rcRoll, 4);            s += ',';
    s += String(r.rcPitch, 4);           s += ',';
    s += String(r.rcYaw, 4);             s += ',';
    s += String(r.roll, 3);              s += ',';
    s += String(r.pitch, 3);             s += ',';
    s += String(r.yaw, 3);               s += ',';
    s += String(r.gxRaw, 3);             s += ',';
    s += String(r.gyRaw, 3);             s += ',';
    s += String(r.gzRaw, 3);             s += ',';
    s += String(r.gxFilt, 3);            s += ',';
    s += String(r.gyFilt, 3);            s += ',';
    s += String(r.gzFilt, 3);            s += ',';
    s += String(r.rO, 4);                s += ',';
    s += String(r.pO, 4);                s += ',';
    s += String(r.yO, 4);                s += ',';
    s += String(r.mFL, 4);               s += ',';
    s += String(r.mFR, 4);               s += ',';
    s += String(r.mRL, 4);               s += ',';
    s += String(r.mRR, 4);               s += '\n';
    return s;
}
// ─────────────────────────────────────────────────────────────
//  taskControl — Core 1, priority 5, 400 Hz  (2.5 ms period)
// ─────────────────────────────────────────────────────────────
static void taskControl(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(3);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastUs = micros();
    const uint32_t TARGET_US = TIMING_TARGET_US;

    pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
    pidAngleRoll.reset(); pidAnglePitch.reset();
    pidAngleYaw.reset();
    g_yawHoldActive = false;

    for (;;) {
        if (g_tuning.dirty) applyTuningToObjects();

        if (g_calibState == CalibState::REQUESTED) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            pidAngleYaw.reset();
            g_yawHoldActive = false;
            runAutonomousCalibration();
            g_calibState = CalibState::IDLE;
            lastUs   = micros();
            lastWake = xTaskGetTickCount();
            continue;
        }

        vTaskDelay(1);   // yield — allows WiFi, BMP, GPS, Serial to run
        while ((micros() - lastUs) < TARGET_US) { }   // busy-wait remainder

        uint32_t nowUs    = micros();
        uint32_t periodUs = nowUs - lastUs;
        lastUs = nowUs;
        lastWake = xTaskGetTickCount();

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

        // ── IMU read + Mahony AHRS ────────────────────────────
        MPU_SensorData s;
        MPU_Attitude   att;
        bool imuOk = imu.readScaled(s);
        float gxf = 0, gyf = 0, gzf = 0;
        if (imuOk) {
            imu.mahonyUpdate(s, dt, att);
            gxf = lpfGx.apply(s.gx_dps, dt, GYRO_LPF_HZ);
            gyf = lpfGy.apply(s.gy_dps, dt, GYRO_LPF_HZ);
            gzf = lpfGz.apply(s.gz_dps, dt, GYRO_LPF_HZ);
        }

        RCCommand cmd = rcReceiver.getCommand();

        // ── DISARMED / FAILSAFE ───────────────────────────────
        if (cmd.mode == FlightMode::DISARMED || cmd.mode == FlightMode::FAILSAFE) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            lpfGx.reset(); lpfGy.reset(); lpfGz.reset();
            lpfSpRoll.reset(); lpfSpPitch.reset(); lpfSpYaw.reset();
            pidAngleYaw.reset();
            g_yawHoldActive = false;

            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
                g_state.pidRollOut = g_state.pidPitchOut = g_state.pidYawOut = 0;
                g_state.rc      = cmd;
                if (imuOk) {
                    g_state.roll_deg = att.roll;  g_state.pitch_deg = att.pitch;
                    g_state.yaw_deg  = att.yaw;
                    g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                    g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                    g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                    g_state.loopCount++;
                }
                xSemaphoreGive(g_flightMutex);
            }
            continue;
        }

        // ── ARMED — cascaded PID ──────────────────────────────
        float roll  = imuOk ? att.roll  : 0.0f;
        float pitch = imuOk ? att.pitch : 0.0f;
        float gx    = imuOk ? gxf : 0.0f;
        float gy    = imuOk ? gyf : 0.0f;
        float gz    = imuOk ? gzf : 0.0f;

        const float MAX_ANGLE_DEG = TUNE_MAX_ANGLE_DEG;
        const float MAX_RATE_DPS  = TUNE_MAX_RATE_DPS;
        float rO=0, pO=0, yO=0;

        float rollCmd  = lpfSpRoll .apply(cmd.roll,  dt, RC_LPF_HZ);
        float pitchCmd = lpfSpPitch.apply(cmd.pitch, dt, RC_LPF_HZ);
        float yawCmd   = lpfSpYaw  .apply(cmd.yaw,   dt, RC_LPF_HZ);

        if (cmd.mode == FlightMode::ANGLE) {
            float rSP = pidAngleRoll .update(rollCmd *MAX_ANGLE_DEG - roll,  dt);
            float pSP = pidAnglePitch.update(pitchCmd*MAX_ANGLE_DEG - pitch, dt);
            rO = pidRateRoll .update(rSP - gx, dt);
            pO = pidRatePitch.update(pSP - gy, dt);
        } else {   // ACRO
            rO = pidRateRoll .update(rollCmd *MAX_RATE_DPS - gx, dt);
            pO = pidRatePitch.update(pitchCmd*MAX_RATE_DPS - gy, dt);
        }

        // ── Yaw: heading-hold when stick centered, rate when moving ──
        if (imuOk && fabsf(yawCmd) < TUNE_YAW_DEADBAND) {
            if (!g_yawHoldActive) {
                g_yawSetpoint   = att.yaw;
                g_yawHoldActive = true;
                pidAngleYaw.reset();
            }
            float yawErr = g_yawSetpoint - att.yaw;
            while (yawErr >  180.0f) yawErr -= 360.0f;
            while (yawErr < -180.0f) yawErr += 360.0f;
            float yawRateSP = pidAngleYaw.update(yawErr, dt);
            yawRateSP = constrain(yawRateSP, -TUNE_YAW_MAX_RATE_DPS, TUNE_YAW_MAX_RATE_DPS);
            yO = pidRateYaw.update(yawRateSP - gz, dt);
        } else {
            g_yawHoldActive = false;
            yO = pidRateYaw.update(-yawCmd*MAX_RATE_DPS - gz, dt);
        }

        rO = constrain(rO, -TUNE_ROLL_OUTPUT_LIMIT,  TUNE_ROLL_OUTPUT_LIMIT);
        pO = constrain(pO, -TUNE_PITCH_OUTPUT_LIMIT, TUNE_PITCH_OUTPUT_LIMIT);
        yO = constrain(yO, -TUNE_YAW_OUTPUT_LIMIT,   TUNE_YAW_OUTPUT_LIMIT);

        // ── Throttle expo + smoothing ─────────────────────────
        static float thrSmooth = 0.0f;

        const float THROTTLE_EXPO              = TUNE_THROTTLE_EXPO;
        const float THROTTLE_UP_RATE_PER_SEC   = TUNE_THROTTLE_UP_RATE_PER_SEC;
        const float THROTTLE_DOWN_RATE_PER_SEC = TUNE_THROTTLE_DOWN_RATE_PER_SEC;
        const float MOTOR_IDLE                 = TUNE_MOTOR_IDLE;
        const float MOTOR_MAX                  = TUNE_MOTOR_MAX;
        const float THROTTLE_CUT               = TUNE_THROTTLE_CUT;
        const float IDLE_RAMP_END              = TUNE_IDLE_RAMP_END;

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
        float maxMotor = max(max(fl, fr), max(rl, rr));
        if (maxMotor > MOTOR_MAX) {
            float excess = maxMotor - MOTOR_MAX;
            fl -= excess; fr -= excess; rl -= excess; rr -= excess;
        }

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

        writeMotors(fl, fr, rl, rr);

        // ── High-speed flight log @ 100 Hz (every 4th cycle) ──
        static uint8_t logDiv = 0;
        if (++logDiv >= 4) {
            logDiv = 0;
            FlightLogRow row;
            row.t_us      = nowUs;
            row.period_us = (uint16_t)min(periodUs, (uint32_t)65535);
            row.mode      = (uint8_t)cmd.mode;
            row.flags     = 0x01 | (imuOk ? 0x02 : 0x00);   // armed here by definition
            row.rcThrottle = thrRaw;
            row.thr        = thr;
            row.rcRoll = rollCmd; row.rcPitch = pitchCmd; row.rcYaw = yawCmd;
            row.roll = roll; row.pitch = pitch; row.yaw = imuOk ? att.yaw : 0.0f;
            row.gxRaw = imuOk ? s.gx_dps : 0.0f;
            row.gyRaw = imuOk ? s.gy_dps : 0.0f;
            row.gzRaw = imuOk ? s.gz_dps : 0.0f;
            row.gxFilt = gx; row.gyFilt = gy; row.gzFilt = gz;
            row.rO = rO; row.pO = pO; row.yO = yO;
            row.mFL = fl; row.mFR = fr; row.mRL = rl; row.mRR = rr;
            pushFlightLog(row);
        }

        // ── Publish flight state (incl. true PID outputs) ─────
        if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE){
            if (imuOk) {
                g_state.roll_deg = att.roll;  g_state.pitch_deg = att.pitch;
                g_state.yaw_deg  = att.yaw;
                g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                g_state.loopCount++;
            }
            g_state.motorFL=fl; g_state.motorFR=fr;
            g_state.motorRL=rl; g_state.motorRR=rr;
            g_state.pidRollOut=rO; g_state.pidPitchOut=pO; g_state.pidYawOut=yO;
            g_state.armed=true; g_state.rc=cmd;
            xSemaphoreGive(g_flightMutex);
        }
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
    Serial.println(F("║  FlySky iBUS + MPU-9250/6500 + BMP280 + GPS  v2.3.2 ║"));
    Serial.println(F("║  Wi-Fi: ESP32-DRONE / 12345678 → 192.168.4.1        ║"));
    Serial.println(F("║  taskControl: merged IMU+PID @ 400 Hz, Core 1       ║"));
    Serial.println(F("║  Type 'p' to toggle the [PID] tuning trace.         ║"));
    Serial.println(F("╚══════════════════════════════════════════════════════╝"));

    for (;;) {
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
        const char* ms = "???";
        switch(s.rc.mode) {
            case FlightMode::DISARMED: ms="DISARMD"; break;
            case FlightMode::ANGLE:    ms="ANGLE  "; break;
            case FlightMode::ACRO:     ms="ACRO   "; break;
            case FlightMode::FAILSAFE: ms="FAILSFE"; break;
        }

        // 1 Hz status line
        if (tick % 20 == 0) {
            Serial.printf("[%6lu] %s | R=%+6.1f P=%+6.1f Y=%6.1f | "
                          "MOT %.2f %.2f %.2f %.2f | "
                          "BMP %.1fC %.1fhPa %.1fm | "
                          "GPS %s Sats=%d | "
                          "Mag:%s mKp=%.2f\n",
                          (unsigned long)tick, ms,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
                          s.motorFL, s.motorFR, s.motorRL, s.motorRR,
                          s.bmpTemp_c, s.bmpPressure_hpa, s.bmpAltitude_m,
                          s.gps.valid ? "FIX" : "---", s.gps.satellites,
                          imu.hasMag() ? "9DOF" : "6DOF",
                          imu.mahonyKp);
        }

        // ~4 Hz PID tuning trace (sibling of the 1 Hz block; gated by 'p')
        if (g_pidTrace && s.armed && (tick % 5 == 0)) {
            Serial.printf("[PID] %s | cmd r=%+.2f p=%+.2f y=%+.2f | "
                          "att R=%+6.2f P=%+6.2f Y=%+6.2f | "
                          "gyro gx=%+7.1f gy=%+7.1f gz=%+7.1f | "
                          "out rO=%+.4f pO=%+.4f yO=%+.4f | "
                          "yawSP=%+6.1f hold=%d\n",
                          ms, s.rc.roll, s.rc.pitch, s.rc.yaw,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
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

    for (;;) {
        BMP280Data b;
        if (bmp280.read(b)) {
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpTemp_c      = b.temperature_c;
                g_state.bmpPressure_hpa = b.pressure_hpa;
                g_state.bmpAltitude_m  = b.altitude_m;
                g_state.bmpValid       = b.valid;
                xSemaphoreGive(g_flightMutex);
            }
        } else {
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpValid = false; xSemaphoreGive(g_flightMutex);
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
//  taskWiFi — Core 0, priority 1, event-driven
// ─────────────────────────────────────────────────────────────
static void taskWiFi(void* /*pv*/)
{
    telemetryWiFi.setTelemetryProvider(provideTelemetry);
    telemetryWiFi.setTuneHandler(handleTune);

    telemetryWiFi.setTimingProvider(provideTimingJson);
    telemetryWiFi.setTimingCsvProvider(provideTimingCsv);
    telemetryWiFi.setTimingResetHandler(resetTimingStats);

// High-speed flight log (chunked streaming — no giant String)
    telemetryWiFi.setFlightLogCountProvider(flightLogCount);
    telemetryWiFi.setFlightLogHeaderProvider(flightLogHeader);
    telemetryWiFi.setFlightLogRowProvider(flightLogRowCsv);
    telemetryWiFi.setFlightLogResetHandler(resetFlightLog);

    telemetryWiFi.begin("ESP32-DRONE", "12345678");

    for (;;) {
        telemetryWiFi.update();
        vTaskDelay(pdMS_TO_TICKS(2));
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

    memset(&g_state,  0, sizeof(g_state));
    memset(&g_timing, 0, sizeof(g_timing));

    motorsBegin();
    motorEscArm();   // sends 1000 µs for 3 s; comment out after ESC calibration done

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_MPU_CS);
    delay(10);

    Serial.print(F("[BOOT] IMU... "));
    if (!imu.begin()) {
        Serial.println(F("FAILED. Check SPI wiring. Halting."));
        while (true) delay(1000);
    }
    Serial.println(F("OK"));

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

    Serial.println(F("[BOOT] All tasks running."));
    Serial.println(F("[BOOT] Timing target: 2500 us (400 Hz control loop)."));
    Serial.println(F("[BOOT] Ground station: http://192.168.4.1"));
    Serial.println(F("[BOOT] GPS fix takes 30-90 s outdoors."));
    Serial.printf("[BOOT] Free heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(portMAX_DELAY); }
