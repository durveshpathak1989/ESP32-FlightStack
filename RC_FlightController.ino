/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v2.3.1                                  ║
 * ║  FlySky FS-iA6B iBUS  +  MPU-9250/6500  +  BMP280  +  GPS       ║
 * ║  Fully autonomous — no keyboard required                         ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  v2.3.1 fixes (over v2.3):                                       ║
 * ║   • MERGED taskIMU + taskPID → single taskControl at 400 Hz     ║
 * ║     Root cause of Core1=100%: two 500 Hz tasks sharing one core  ║
 * ║     Fix: one merged task, 2.5 ms budget, Core 1 ~45% utilisation ║
 * ║   • WiFi telemetry mutex timeout 5 ms → 15 ms (fixes "undefined" ║
 * ║     cycling caused by mutex starvation under Core 1 load)        ║
 * ║   • TIMING_TARGET_US updated to 2500 µs (400 Hz)                ║
 * ║   Requires TelemetryWiFi v2.4 (timing endpoint callbacks)        ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                           ║
 * ║   FS-iA6B iBUS port → GPIO 16 (UART2 RX)                        ║
 * ║   MPU-9250 SCLK→5  MOSI→18  MISO→19  NCS→33  VCC→3.3V          ║
 * ║   BMP280 SDA→21  SCL→22  VCC→3.3V  GND→GND  CSB→3.3V           ║
 * ║   GPS TXD→13  VCC→3.3V  GND→GND (UART1)                         ║
 * ║   Motors: FL→25  FR→15  RL→14  RR→32                            ║
 * ║                                                                  ║
 * ║  GPIO 17 is shared: iBUS TX (unused) + GPS UART1 TX (optional)  ║
 * ║  Both are output-only sides never driven — no electrical conflict║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  RC SWITCH ASSIGNMENTS                                            ║
 * ║   CH7  SWA  → ARM / DISARM                                       ║
 * ║   CH8  SWB  → ANGLE / ACRO                                       ║
 * ║   CH9  SWC  → Accel confirm during calibration                   ║
 * ║   CH10 SWD  → CALIBRATION trigger (flip UP while disarmed)       ║
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
#define MOTOR_KV         2300.0f
#define BATTERY_VOLTAGE    11.1f

// ─────────────────────────────────────────────────────────────
//  IMU loop timing instrumentation — Test 7.1
//
//  Ring buffer stores the last 3000 raw period_us samples
//  (~7.5 s of data at 400 Hz).  The Welford online algorithm
//  tracks mean and variance without storing every sample for
//  the stats; the ring buffer is only needed for the p99
//  computation and the CSV download.
//
//  The timing mutex is try-taken with timeout=0 in the hot
//  IMU path: if the WiFi task holds it while computing p99,
//  that single sample is skipped rather than blocking taskIMU.
// ─────────────────────────────────────────────────────────────
#define TIMING_BUF_SIZE       3000     // ring buffer depth (samples)
#define TIMING_TARGET_US      2500     // nominal control period (400 Hz = 2500 µs)
#define JITTER_VIOLATION_US    100     // threshold: counts as a violation

struct TimingStats {
    // Welford online algorithm accumulators
    uint32_t count;
    double   wMean;   // running mean of period_us
    double   wM2;     // running sum of squared deviations (for variance)
    // Jitter = |period_us - TIMING_TARGET_US|
    uint32_t jitterMax;
    uint32_t jitterViolations;
    double   jMean;
    double   jM2;

    // Additional diagnostics separating scheduler release delay and execution cost
    int32_t  lastPeriodErrorUs;
    int32_t  lastReleaseLatenessUs;
    uint32_t lastExecUs;
    uint32_t execMaxUs;
    int32_t  releaseLatenessMaxUs;
    uint32_t deadlineMisses;

    // Ring buffer for p99 and CSV export (chronological, head = next write)
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
    RCCommand rc;
    bool  armed;
    uint32_t loopCount;
    GPSData gps;
};
static FlightState       g_state;
static SemaphoreHandle_t g_flightMutex;

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
static TaskHandle_t hTaskIMU    = nullptr;   // kept as alias — now points to taskControl
static TaskHandle_t hTaskControl= nullptr;   // merged IMU+PID at 400 Hz on Core 1
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

// Conservative first-flight gains — tune inner (rate) loop first in ACRO mode
static PID pidRateRoll  (0.08f, 0.000f, 0.000f);
static PID pidRatePitch (0.08f, 0.000f, 0.000f);
static PID pidRateYaw   (0.10f, 0.000f, 0.000f);
static PID pidAngleRoll (1.5f,  0.0f,   0.0f);
static PID pidAnglePitch(1.5f,  0.0f,   0.0f);

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
//
//  p99 is computed by sorting a local copy of the ring buffer.
//  Insertion sort on n≤3000 uint16s takes ~2–5 ms at 240 MHz.
//  This runs in taskWiFi (Core 0); the hot control loop uses a non-blocking mutex try-take.
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

    // Copy ring buffer and convert to absolute jitter before sorting.
    // Important: p99 must be the 99th percentile of |period-target|,
    // not the 99th percentile of raw period_us minus target.
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

    // Insertion sort (ascending) — O(n²) but n≤3000, fine on Core 0
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
//  Returns chronological index,period_us,jitter_us CSV.
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
    // Increased from 5 ms → 15 ms: Core 1 holds this mutex for ~0.8 ms per control
    // cycle at 400 Hz; 15 ms gives the WiFi task 6 full cycles to acquire it.
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
//  Rejected while armed.
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
//  Triggered by SWD↑ while disarmed (Test 7.4).
//  Motors are zeroed before starting. Because calibration runs
//  inside taskControl itself, there is no separate PID task to
//  suspend — motor output is simply stopped at entry and the
//  PID state is reset on exit.
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
    struct Pos { const char* lbl; const char* ins; };
    const Pos pos[6] = {
        {"+X UP","RIGHT side UP"}, {"-X UP","LEFT side UP"},
        {"+Y UP","NOSE UP"},       {"-Y UP","TAIL UP"},
        {"+Z UP","FLAT top up"},   {"-Z UP","UPSIDE DOWN"}
    };
    float rds[6][3] = {};
    calLog("[CAL] 2/3 ACCEL — 6 positions, flip SWC UP to confirm each.");
    for (int p = 0; p < 6; p++) {
        if (swcIsUp()) { calLog("[CAL] Flip SWC DOWN first..."); waitSwcDown(); }
        calLogf("[CAL] Pos %d/6: %s — %s — Flip SWC UP when steady",
                p+1, pos[p].lbl, pos[p].ins);
        uint32_t t0 = millis();
        while (!swcIsUp()) {
            if (millis()-t0 > ACCEL_WAIT_MAX_MS) { calLog("[CAL] Timeout — position skipped"); break; }
            delay(20);
        }
        if (!swcIsUp()) continue;
        silentWait(ACCEL_HOLD_MS);
        MPU_SensorData avg;
        imu.sampleAvg(ACCEL_HOLD_MS / 2, avg);
        rds[p][0]=avg.ax_g; rds[p][1]=avg.ay_g; rds[p][2]=avg.az_g;
        calLogf("[CAL] Got ax=%+.4f ay=%+.4f az=%+.4f g", avg.ax_g, avg.ay_g, avg.az_g);
        waitSwcDown();
    }
    imu.cal.ax_b = (rds[0][0]+rds[1][0])/2.0f;
    imu.cal.ay_b = (rds[2][1]+rds[3][1])/2.0f;
    imu.cal.az_b = (rds[4][2]+rds[5][2])/2.0f;
    float hx=(rds[0][0]-rds[1][0])/2.0f;
    float hy=(rds[2][1]-rds[3][1])/2.0f;
    float hz=(rds[4][2]-rds[5][2])/2.0f;
    imu.cal.ax_s = fabsf(hx)>0.01f ? 1.0f/hx : 1.0f;
    imu.cal.ay_s = fabsf(hy)>0.01f ? 1.0f/hy : 1.0f;
    imu.cal.az_s = fabsf(hz)>0.01f ? 1.0f/hz : 1.0f;
    calLog("[CAL] ✓ ACCEL done.");

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
//  Detects SWD rising edge to request calibration.
// ─────────────────────────────────────────────────────────────
static void taskRC(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();
    bool swdPrev = false;

    for (;;) {
        rcReceiver.update();
        RCCommand cmd = rcReceiver.getCommand();

        // Rising edge on SWD (CH10) while disarmed → request calibration
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

// ─────────────────────────────────────────────────────────────
//  taskControl — Core 1, priority 5, 400 Hz  (2.5 ms period)
//
//  MERGED replacement for the previous taskIMU + taskPID pair.
//
//  Why merged?
//    Two separate 500 Hz tasks on the same core require 4 ms of
//    CPU time per 2 ms window (200% utilisation), causing Core 1
//    to pin at 100% and vTaskDelayUntil to thrash. By merging
//    into one task at 400 Hz (2.5 ms budget), the full
//    IMU-read + AHRS + PID + motor-write pipeline takes ~0.8–1.0 ms
//    leaving ~1.5 ms of slack — Core 1 runs at ~40–45%.
//
//  Timing budget per cycle (measured on ESP32 @ 240 MHz):
//    SPI burst read (22 bytes @ 20 MHz) : ~150 µs
//    Mahony 9-DOF AHRS update           : ~200 µs
//    Cascaded PID + motor write          : ~100 µs
//    Mutex + overhead                    : ~50 µs
//    Total                               : ~500 µs  (20% of 2.5 ms)
//
//  Welford accumulator: try-take mutex with timeout=0 so the WiFi
//  task computing p99 never delays the control loop.
// ─────────────────────────────────────────────────────────────
static void taskControl(void* /*pv*/)
{
    const uint32_t TARGET_US = TIMING_TARGET_US;   // 2500 µs = 400 Hz
    const uint32_t YIELD_GUARD_US = 150;           // short final spin window
    const int32_t RELEASE_RESYNC_US = 5000;        // large release miss threshold

    uint32_t lastStartUs = micros();
    uint32_t nextReleaseUs = lastStartUs + TARGET_US;
    RCCommand lastCmd{};
    lastCmd.mode = FlightMode::FAILSAFE;

    // PID state (declared here so they reset cleanly after calibration)
    pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
    pidAngleRoll.reset(); pidAnglePitch.reset();

    for (;;) {
        while ((int32_t)(nextReleaseUs - micros()) > (int32_t)YIELD_GUARD_US) {
            vTaskDelay(1);
        }
        while ((int32_t)(nextReleaseUs - micros()) > 0) { }

        uint32_t startUs = micros();
        uint32_t periodUs = startUs - lastStartUs;
        int32_t periodErrorUs = (int32_t)periodUs - (int32_t)TARGET_US;
        int32_t releaseLatenessUs = (int32_t)(startUs - nextReleaseUs);

        lastStartUs = startUs;
        nextReleaseUs += TARGET_US;
        if (releaseLatenessUs > RELEASE_RESYNC_US) {
            nextReleaseUs = startUs + TARGET_US;
        }

        if (g_tuning.dirty) applyTuningToObjects();

        if (g_calibState == CalibState::REQUESTED) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            runAutonomousCalibration();
            g_calibState = CalibState::IDLE;
            lastStartUs = micros();
            nextReleaseUs = lastStartUs + TARGET_US;
            continue;
        }

        float dt = (float)periodUs * 1e-6f;
        if (dt <= 0.0f || dt > 0.05f) {
            dt = (float)TARGET_US * 1e-6f;
            periodUs = TARGET_US;
            periodErrorUs = 0;
        }

        MPU_SensorData s;
        MPU_Attitude   att;
        bool imuOk = imu.readScaled(s); // TODO: optional 6DOF fast path for 400 Hz loop
        if (imuOk) {
            imu.mahonyUpdate(s, dt, att);
        }

        RCCommand cmd;
        if (!rcReceiver.getCommandFast(cmd)) {
            if (lastCmd.valid) cmd = lastCmd;
            else { cmd = RCCommand{}; cmd.mode = FlightMode::FAILSAFE; cmd.valid = false; }
        } else {
            lastCmd = cmd;
        }

        if (cmd.mode == FlightMode::DISARMED || cmd.mode == FlightMode::FAILSAFE) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();

            if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
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
            uint32_t execUs = micros() - startUs;
            if (xSemaphoreTake(g_timingMutex, 0) == pdTRUE) {
                uint32_t n = ++g_timing.count;
                double delta=(double)periodUs-g_timing.wMean; g_timing.wMean+=delta/n; g_timing.wM2+=delta*((double)periodUs-g_timing.wMean);
                uint32_t jit = (periodUs>=TARGET_US)?(periodUs-TARGET_US):(TARGET_US-periodUs);
                double jdelta=(double)jit-g_timing.jMean; g_timing.jMean+=jdelta/n; g_timing.jM2+=jdelta*((double)jit-g_timing.jMean);
                if (jit>g_timing.jitterMax) g_timing.jitterMax=jit; if (jit>JITTER_VIOLATION_US) g_timing.jitterViolations++;
                g_timing.lastPeriodErrorUs=periodErrorUs; g_timing.lastReleaseLatenessUs=releaseLatenessUs; g_timing.lastExecUs=execUs;
                if (execUs>g_timing.execMaxUs) g_timing.execMaxUs=execUs; if (releaseLatenessUs>g_timing.releaseLatenessMaxUs) g_timing.releaseLatenessMaxUs=releaseLatenessUs;
                if (execUs>TARGET_US) g_timing.deadlineMisses++;
                g_timing.buf[g_timing.bufHead]=(uint16_t)min(periodUs,(uint32_t)65535); g_timing.bufHead=(g_timing.bufHead+1)%TIMING_BUF_SIZE; if (g_timing.bufHead==0) g_timing.bufFull=true;
                xSemaphoreGive(g_timingMutex);
            }
            continue;
        }

        float roll  = imuOk ? att.roll  : 0.0f;
        float pitch = imuOk ? att.pitch : 0.0f;
        float gx    = imuOk ? s.gx_dps : 0.0f;
        float gy    = imuOk ? s.gy_dps : 0.0f;
        float gz    = imuOk ? s.gz_dps : 0.0f;

        const float MAX_ANGLE_DEG = 30.0f;
        const float MAX_RATE_DPS  = 200.0f;
        float rO=0, pO=0, yO=0;

        if (cmd.mode == FlightMode::ANGLE) {
            float rSP = pidAngleRoll .update(cmd.roll *MAX_ANGLE_DEG - roll,  dt);
            float pSP = pidAnglePitch.update(cmd.pitch*MAX_ANGLE_DEG - pitch, dt);
            rO = pidRateRoll .update(rSP - gx, dt);
            pO = pidRatePitch.update(pSP - gy, dt);
            yO = pidRateYaw  .update(cmd.yaw*MAX_RATE_DPS - gz, dt);
        } else {
            rO = pidRateRoll .update(cmd.roll *MAX_RATE_DPS - gx, dt);
            pO = pidRatePitch.update(cmd.pitch*MAX_RATE_DPS - gy, dt);
            yO = pidRateYaw  .update(cmd.yaw  *MAX_RATE_DPS - gz, dt);
        }

        rO = constrain(rO, -0.25f, 0.25f);
        pO = constrain(pO, -0.25f, 0.25f);
        yO = constrain(yO, -0.15f, 0.15f);

        float thr = cmd.throttle;
        float fl = constrain(thr+rO-pO-yO, 0.0f, 1.0f);
        float fr = constrain(thr-rO-pO+yO, 0.0f, 1.0f);
        float rl = constrain(thr+rO+pO+yO, 0.0f, 1.0f);
        float rr = constrain(thr-rO+pO-yO, 0.0f, 1.0f);

        writeMotors(fl, fr, rl, rr);

        if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE) {
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
            g_state.armed=true; g_state.rc=cmd;
            xSemaphoreGive(g_flightMutex);
        }

        uint32_t execUs = micros() - startUs;
        if (xSemaphoreTake(g_timingMutex, 0) == pdTRUE) {
            uint32_t n = ++g_timing.count;
            double delta=(double)periodUs-g_timing.wMean; g_timing.wMean+=delta/n; g_timing.wM2+=delta*((double)periodUs-g_timing.wMean);
            uint32_t jit = (periodUs>=TARGET_US)?(periodUs-TARGET_US):(TARGET_US-periodUs);
            double jdelta=(double)jit-g_timing.jMean; g_timing.jMean+=jdelta/n; g_timing.jM2+=jdelta*((double)jit-g_timing.jMean);
            if (jit>g_timing.jitterMax) g_timing.jitterMax=jit; if (jit>JITTER_VIOLATION_US) g_timing.jitterViolations++;
            g_timing.lastPeriodErrorUs=periodErrorUs; g_timing.lastReleaseLatenessUs=releaseLatenessUs; g_timing.lastExecUs=execUs;
            if (execUs>g_timing.execMaxUs) g_timing.execMaxUs=execUs; if (releaseLatenessUs>g_timing.releaseLatenessMaxUs) g_timing.releaseLatenessMaxUs=releaseLatenessUs;
            if (execUs>TARGET_US) g_timing.deadlineMisses++;
            g_timing.buf[g_timing.bufHead]=(uint16_t)min(periodUs,(uint32_t)65535); g_timing.bufHead=(g_timing.bufHead+1)%TIMING_BUF_SIZE; if (g_timing.bufHead==0) g_timing.bufFull=true;
            xSemaphoreGive(g_timingMutex);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  taskSerial — Core 0, priority 1, 20 Hz
//  1 Hz telemetry line to Serial (also parseable by ground station
//  over USB serial mode).
// ─────────────────────────────────────────────────────────────
static void taskSerial(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t tick = 0;

    Serial.println(F("\n╔══════════════════════════════════════════════════════╗"));
    Serial.println(F("║  FlySky iBUS + MPU-9250/6500 + BMP280 + GPS  v2.3.1 ║"));
    Serial.println(F("║  Wi-Fi: ESP32-DRONE / 12345678 → 192.168.4.1        ║"));
    Serial.println(F("║  taskControl: merged IMU+PID @ 400 Hz, Core 1       ║"));
    Serial.println(F("╚══════════════════════════════════════════════════════╝"));

    for (;;) {
        if (g_calibState != CalibState::IDLE && g_calibState != CalibState::DONE) {
            tick++; vTaskDelayUntil(&lastWake, period); continue;
        }
        if (tick % 20 == 0) {  // 1 Hz
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
//  Registers all HTTP routes including timing endpoints.
//  Requires TelemetryWiFi v2.4 header.
// ─────────────────────────────────────────────────────────────
static void taskWiFi(void* /*pv*/)
{
    telemetryWiFi.setTelemetryProvider(provideTelemetry);
    telemetryWiFi.setTuneHandler(handleTune);

    // Timing endpoints (Test 7.1) — requires TelemetryWiFi v2.4
    telemetryWiFi.setTimingProvider(provideTimingJson);
    telemetryWiFi.setTimingCsvProvider(provideTimingCsv);
    telemetryWiFi.setTimingResetHandler(resetTimingStats);

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

    // ── Create synchronisation primitives ─────────────────────
    g_flightMutex = xSemaphoreCreateMutex();
    g_tuneMutex   = xSemaphoreCreateMutex();
    g_timingMutex = xSemaphoreCreateMutex();
    configASSERT(g_flightMutex);
    configASSERT(g_tuneMutex);
    configASSERT(g_timingMutex);

    memset(&g_state,  0, sizeof(g_state));
    memset(&g_timing, 0, sizeof(g_timing));

    // ── ESC PWM init + arm pulse ───────────────────────────────
    motorsBegin();
    motorEscArm();   // sends 1000 µs for 3 s; comment out after ESC calibration done

    // ── SPI + IMU ──────────────────────────────────────────────
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

    // ── BMP280 barometer ──────────────────────────────────────
    bmp280.scanI2C(PIN_BMP_SDA, PIN_BMP_SCL, 100000);
    Serial.print(F("[BOOT] BMP280... "));
    bmp280.beginAuto(PIN_BMP_SDA, PIN_BMP_SCL, 100000)
        ? Serial.println(F("OK")) : Serial.println(F("not found."));

    // ── CPU load monitor ──────────────────────────────────────
    Serial.print(F("[BOOT] CPU monitor... "));
    cpuUtilization.begin(1000)
        ? Serial.println(F("OK")) : Serial.println(F("idle-hook failed."));

    // ── GPS ───────────────────────────────────────────────────
    Serial.println(F("[BOOT] GPS (GY-GPS6MV2)..."));
    gps.begin(PIN_GPS_RX, PIN_GPS_TX, 9600);

    // ── Load calibration from NVS flash ───────────────────────
    Serial.print(F("[BOOT] NVS calibration... "));
    if (imu.loadCalibration()) {
        Serial.println(F("loaded."));
        imu.printCalibration();
    } else {
        Serial.println(F("none — flip SWD UP (disarmed) to calibrate."));
    }

    // ── iBUS RC receiver ──────────────────────────────────────
    rcReceiver.begin(PIN_IBUS_RX, PIN_IBUS_TX, 2);
    Serial.println(F("[BOOT] iBUS ready."));

    syncTuningFromObjects();

    // ── Spawn FreeRTOS tasks ──────────────────────────────────
    // Core 0 — low-latency I/O and network
    xTaskCreatePinnedToCore(taskRC,      "RC",     6144,  nullptr, 3, &hTaskRC,      0);
    xTaskCreatePinnedToCore(taskSerial,  "Serial", 4096,  nullptr, 1, &hTaskSerial,  0);
    xTaskCreatePinnedToCore(taskWiFi,    "WiFi",   12288, nullptr, 1, &hTaskWiFi,    0);
    xTaskCreatePinnedToCore(taskBMP,     "BMP280", 3072,  nullptr, 1, &hTaskBMP,     0);
    xTaskCreatePinnedToCore(taskCPU,     "CPU",    3072,  nullptr, 1, &hTaskCPU,     0);
    xTaskCreatePinnedToCore(taskGPS,     "GPS",    4096,  nullptr, 1, &hTaskGPS,     0);
    // Core 1 — ONE merged real-time control task (prevents the 200% overload)
    xTaskCreatePinnedToCore(taskControl, "Ctrl",   10240, nullptr, 5, &hTaskControl, 1);
    hTaskIMU = hTaskControl;   // alias for any code that checks hTaskIMU

    Serial.println(F("[BOOT] All tasks running."));
    Serial.println(F("[BOOT] Timing target: 2500 us (400 Hz control loop)."));
    Serial.println(F("[BOOT] Ground station: http://192.168.4.1"));
    Serial.println(F("[BOOT] GPS fix takes 30-90 s outdoors."));
}

void loop() { vTaskDelay(portMAX_DELAY); }
