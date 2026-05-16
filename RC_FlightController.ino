/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v2.0                                   ║
 * ║  FlySky FS-iA6B iBUS  +  MPU-9250  +  HUZZAH32 Feather          ║
 * ║  Fully autonomous — no keyboard required                         ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                           ║
 * ║   FS-iA6B iBUS port → GPIO 16 (UART2 RX)                        ║
 * ║   FS-iA6B VCC       → 5V (BEC)   GND → GND                      ║
 * ║   MPU-9250 SCLK→5  MOSI→18  MISO→19  NCS→33  VCC→3.3V          ║
 * ║   BMP280 SDA→21  SCL→22  VCC→3.3V  GND→GND  CSB→3.3V        ║
 * ║   Motors: FL→25  FR→27  RL→14  RR→32 (DShot stub)               ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  RC SWITCH ASSIGNMENTS (FS-i6X Mode 2)                           ║
 * ║   CH1 Roll   CH2 Pitch   CH3 Throttle   CH4 Yaw                  ║
 * ║   CH5 SWA  → Arm/Disarm      (up >1700 = ARMED)                 ║
 * ║   CH5 VrA  → Spare / PID tune (knob)                            ║
 * ║   CH6 VrB  → Spare / PID tune (knob)                            ║
 * ║   CH7 SWA  → ARM / DISARM only                                  ║
 * ║   CH8 SWB  → ANGLE / ACRO mode                                  ║
 * ║   CH9 SWC  → Accel confirm during calibration                   ║
 * ║   CH10 SWD → CALIBRATION trigger only (flip UP while disarmed)  ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  AUTONOMOUS CALIBRATION SEQUENCE                                  ║
 * ║  Triggered by: VrA knob fully CW  OR  SWD flipped UP            ║
 * ║  Drone MUST be DISARMED (SWA down) before trigger is accepted    ║
 * ║                                                                   ║
 * ║  STAGE 1 — Gyro  (fully automatic, ~5 s)                        ║
 * ║    Place drone flat. Countdown printed to serial.                 ║
 * ║    No user action needed.                                         ║
 * ║                                                                   ║
 * ║  STAGE 2 — Accel  (6 orientations × 8 s each = ~50 s)           ║
 * ║    Serial tells you which face to point up.                       ║
 * ║    Flip SWC UP when the drone is steady in that position.        ║
 * ║    ESP32 samples for 3 s, then advances to the next position.    ║
 * ║    Flip SWC DOWN again before the next prompt.                   ║
 * ║                                                                   ║
 * ║  STAGE 3 — Mag  (30 s timed sweep)                               ║
 * ║    Serial tells you to rotate drone through all axes slowly.     ║
 * ║    Countdown printed every 5 s. No button presses needed.        ║
 * ║                                                                   ║
 * ║  Results auto-saved to NVS flash on completion.                  ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <SPI.h>
#include "MPU9250.h"
#include "FlySkyiBUS.h"
#include "TelemetryWiFi.h"
#include "BMP280Sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ─────────────────────────────────────────────────────────────
//  Pin assignments
// ─────────────────────────────────────────────────────────────
#define PIN_SPI_SCK         5
#define PIN_SPI_MISO        19
#define PIN_SPI_MOSI        18
#define PIN_MPU_CS          33
#define PIN_IBUS_RX         16
#define PIN_IBUS_TX         17
#define PIN_BMP_SDA         21
#define PIN_BMP_SCL         22

// ─────────────────────────────────────────────────────────────
//  RC thresholds
// ─────────────────────────────────────────────────────────────
#define SWC_THRESHOLD       1700    // µs — SWC up = "READY" confirm

// ─────────────────────────────────────────────────────────────
//  Calibration timing
// ─────────────────────────────────────────────────────────────
#define GYRO_SETTLE_MS      3000    // wait before gyro sampling
#define GYRO_SAMPLE_MS      5000    // duration of gyro averaging
#define ACCEL_HOLD_MS       3000    // samples taken per orientation
#define ACCEL_WAIT_MAX_MS   30000   // max wait for SWC flip per position
#define MAG_DURATION_MS     30000   // mag sweep window

// ─────────────────────────────────────────────────────────────
//  Driver objects
// ─────────────────────────────────────────────────────────────
MPU9250 imu(PIN_MPU_CS);
// rcReceiver — extern singleton from FlySkyiBUS.cpp

// ─────────────────────────────────────────────────────────────
//  Shared flight state
// ─────────────────────────────────────────────────────────────
struct FlightState {
    float roll_deg, pitch_deg, yaw_deg;
    float gx_dps, gy_dps, gz_dps;
    float bmpTemp_c, bmpPressure_hpa, bmpAltitude_m;
    bool  bmpValid;
    float motorFL, motorFR, motorRL, motorRR;
    RCCommand rc;
    bool  armed;
    uint32_t loopCount;
};
static FlightState        g_state;
static SemaphoreHandle_t  g_flightMutex;

// ─────────────────────────────────────────────────────────────
//  Calibration state machine
// ─────────────────────────────────────────────────────────────
enum class CalibState : uint8_t {
    IDLE = 0,
    REQUESTED,
    RUNNING_GYRO,
    RUNNING_ACCEL,
    RUNNING_MAG,
    SAVING,
    DONE
};
static volatile CalibState g_calibState = CalibState::IDLE;

// ─────────────────────────────────────────────────────────────
//  Task handles
// ─────────────────────────────────────────────────────────────
static TaskHandle_t hTaskIMU    = nullptr;
static TaskHandle_t hTaskPID    = nullptr;
static TaskHandle_t hTaskRC     = nullptr;
static TaskHandle_t hTaskSerial = nullptr;
static TaskHandle_t hTaskWiFi   = nullptr;
static TaskHandle_t hTaskBMP    = nullptr;

// ─────────────────────────────────────────────────────────────
//  PID
// ─────────────────────────────────────────────────────────────
struct PID {
    float kp, ki, kd;
    float integral  = 0.0f;
    float prevError = 0.0f;
    float iLimit;
    PID(float p, float i, float d, float ilim = 50.0f)
        : kp(p), ki(i), kd(d), iLimit(ilim) {}
    float update(float error, float dt) {
        integral += error * dt;
        integral  = constrain(integral, -iLimit, iLimit);
        float deriv = (error - prevError) / dt;
        prevError   = error;
        return kp * error + ki * integral + kd * deriv;
    }
    void reset() { integral = 0.0f; prevError = 0.0f; }
};
static PID pidRateRoll (0.5f, 0.002f, 0.010f);
static PID pidRatePitch(0.5f, 0.002f, 0.010f);
static PID pidRateYaw  (1.0f, 0.005f, 0.000f);
static PID pidAngleRoll (4.0f, 0.0f, 0.0f);
static PID pidAnglePitch(4.0f, 0.0f, 0.0f);


// ─────────────────────────────────────────────────────────────
//  Helper: convert flight mode enum to ground-station string
// ─────────────────────────────────────────────────────────────
static const char* flightModeToString(FlightMode mode) {
    switch (mode) {
        case FlightMode::DISARMED: return "DISARMED";
        case FlightMode::ANGLE:    return "ANGLE";
        case FlightMode::ACRO:     return "ACRO";
        case FlightMode::FAILSAFE: return "FAILSAFE";
        default:                   return "UNKNOWN";
    }
}

// ─────────────────────────────────────────────────────────────
//  Wi-Fi telemetry provider
//  Called by TelemetryWiFi.cpp when the HTML requests /telemetry.
// ─────────────────────────────────────────────────────────────
static bool provideTelemetry(TelemetryPacket& out) {
    FlightState s;
    if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) != pdTRUE) {
        return false;
    }
    s = g_state;
    xSemaphoreGive(g_flightMutex);

    out.tick      = s.loopCount;
    out.mode      = flightModeToString(s.rc.mode);
    out.roll_deg  = s.roll_deg;
    out.pitch_deg = s.pitch_deg;
    out.yaw_deg   = s.yaw_deg;
    out.throttle  = s.rc.throttle;
    out.rc_roll   = s.rc.roll;
    out.rc_pitch  = s.rc.pitch;
    out.rc_yaw    = s.rc.yaw;
    out.motor_fl  = s.motorFL;
    out.motor_fr  = s.motorFR;
    out.motor_rl  = s.motorRL;
    out.motor_rr  = s.motorRR;
    out.rc_hz     = rcReceiver.getFrameRate();
    out.armed     = s.armed;
    out.rc_valid  = s.rc.valid;
    out.bmp_temp_c = s.bmpTemp_c;
    out.bmp_pressure_hpa = s.bmpPressure_hpa;
    out.bmp_altitude_m = s.bmpAltitude_m;
    out.bmp_valid = s.bmpValid;
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Motor stub
// ─────────────────────────────────────────────────────────────
static void writeMotors(float fl, float fr, float rl, float rr) {
    (void)fl; (void)fr; (void)rl; (void)rr;
    // Replace with DShotRMT calls when ESCs are wired
}
static void motorsOff() { writeMotors(0,0,0,0); }

// ─────────────────────────────────────────────────────────────
//  Helper: is SWC currently UP?
//  Reads channel directly from the singleton — safe to call
//  from taskIMU because rcReceiver uses its own internal mutex.
// ─────────────────────────────────────────────────────────────
static bool swcIsUp() {
    return rcReceiver.getChannel(RC_CH_AUX3) >= SWC_THRESHOLD;
}

// ─────────────────────────────────────────────────────────────
//  Helper: wait for SWC to go DOWN (used to reset between steps)
// ─────────────────────────────────────────────────────────────
static void waitSwcDown() {
    while (swcIsUp()) delay(20);
}

// ─────────────────────────────────────────────────────────────
//  Helper: silent wait — just blocks for totalMs, no printing.
//  All user-facing messages are printed by the caller before/after.
// ─────────────────────────────────────────────────────────────
static void silentWait(uint32_t totalMs) {
    uint32_t start = millis();
    while (millis() - start < totalMs) {
        delay(50);
    }
}

// ═════════════════════════════════════════════════════════════
//  AUTONOMOUS CALIBRATION ROUTINE
//  Called from taskIMU when g_calibState == REQUESTED.
//  Runs entirely on Core 1. PID task is suspended for the
//  duration. No Serial input, no Enter key — RC switches only.
// ═════════════════════════════════════════════════════════════
static void runAutonomousCalibration()
{
    Serial.println(F("\n"));
    Serial.println(F("╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║         AUTONOMOUS CALIBRATION STARTED           ║"));
    Serial.println(F("║  SWA must stay DOWN (disarmed) throughout        ║"));
    Serial.println(F("║  SWC = confirm switch for accel orientations     ║"));
    Serial.println(F("╚══════════════════════════════════════════════════╝"));

    // ── STAGE 1: GYRO ────────────────────────────────────────
    g_calibState = CalibState::RUNNING_GYRO;

    Serial.println(F("\n[CAL] ══ STAGE 1/3: GYRO ══"));
    Serial.println(F("[CAL] ACTION: Place drone FLAT and COMPLETELY STILL."));
    Serial.printf( "[CAL] Waiting %d s then sampling %d s — do not touch...\n",
                   GYRO_SETTLE_MS / 1000, GYRO_SAMPLE_MS / 1000);
    silentWait(GYRO_SETTLE_MS);
    silentWait(GYRO_SAMPLE_MS);

    // Use sampleAvg directly — avoids the Serial.available() block
    // in calibrateGyro(). Sample N = GYRO_SAMPLE_MS / 2ms = 2500 reads.
    {
        const int N = GYRO_SAMPLE_MS / 2;
        MPU_SensorData avg;
        imu.sampleAvg(N, avg);
        imu.cal.gx_b = avg.gx_dps;
        imu.cal.gy_b = avg.gy_dps;
        imu.cal.gz_b = avg.gz_dps;
        Serial.printf("[CAL] Gyro bias: X=%+.4f  Y=%+.4f  Z=%+.4f  deg/s\n",
                      imu.cal.gx_b, imu.cal.gy_b, imu.cal.gz_b);
    }
    Serial.println(F("[CAL] ✓ GYRO complete.\n"));

    // ── STAGE 2: ACCEL ───────────────────────────────────────
    g_calibState = CalibState::RUNNING_ACCEL;

    // Labels and expected dominant axis for each of the 6 positions
    // dominant_axis: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
    struct AccelPos {
        const char* label;
        const char* instruction;
        int   axis;   // 0=ax 1=ay 2=az
        float sign;   // +1 or -1 expected reading
    };
    const AccelPos positions[6] = {
        { "+X UP  (right side up)",  "Rotate drone so RIGHT side faces UP",  0,  1.0f },
        { "-X UP  (left side up)",   "Rotate drone so LEFT side faces UP",   0, -1.0f },
        { "+Y UP  (nose up)",        "Rotate drone so NOSE faces UP",        1,  1.0f },
        { "-Y UP  (nose down)",      "Rotate drone so TAIL faces UP",        1, -1.0f },
        { "+Z UP  (top up, flat)",   "Place drone FLAT, TOP facing up",      2,  1.0f },
        { "-Z UP  (inverted)",       "Place drone UPSIDE DOWN",              2, -1.0f },
    };

    float readings[6][3] = {};

    Serial.println(F("[CAL] ══ STAGE 2/3: ACCEL ══"));
    Serial.println(F("[CAL] You will be given 6 orientations one at a time."));
    Serial.println(F("[CAL] For each one:"));
    Serial.println(F("[CAL]   1. Place drone in the described position"));
    Serial.println(F("[CAL]   2. Hold it steady"));
    Serial.println(F("[CAL]   3. Flip SWC UP to confirm — sampling begins"));
    Serial.println(F("[CAL]   4. Hold still for 3 s while ESP32 samples"));
    Serial.println(F("[CAL]   5. Flip SWC DOWN — next position will appear\n"));

    for (int pos = 0; pos < 6; pos++) {
        // Make sure SWC is down before showing the prompt
        if (swcIsUp()) {
            Serial.println(F("[CAL] Please flip SWC DOWN first..."));
            waitSwcDown();
        }

        Serial.printf("\n[CAL] Position %d/6: %s\n", pos + 1, positions[pos].label);
        Serial.printf("[CAL] >>> %s\n", positions[pos].instruction);
        Serial.println(F("[CAL] Flip SWC UP when drone is steady..."));

        // Wait for SWC rising edge — up to ACCEL_WAIT_MAX_MS
        uint32_t waitStart = millis();
        while (!swcIsUp()) {
            if (millis() - waitStart > ACCEL_WAIT_MAX_MS) {
                Serial.println(F("[CAL] Timeout — skipping this position!"));
                break;
            }
            delay(20);
        }

        if (!swcIsUp()) continue;   // timed out, use zero reading for this pos

        // SWC is UP — sample for ACCEL_HOLD_MS
        Serial.printf("[CAL] Sampling for %d s — HOLD STILL...\n",
                      ACCEL_HOLD_MS / 1000);
        silentWait(ACCEL_HOLD_MS);

        const int N = ACCEL_HOLD_MS / 2;
        MPU_SensorData avg;
        imu.sampleAvg(N, avg);
        readings[pos][0] = avg.ax_g;
        readings[pos][1] = avg.ay_g;
        readings[pos][2] = avg.az_g;

        Serial.printf("[CAL] Got: ax=%+.4f  ay=%+.4f  az=%+.4f  g\n",
                      avg.ax_g, avg.ay_g, avg.az_g);
        Serial.println(F("[CAL] ✓ Position captured. Flip SWC DOWN for next..."));

        // Wait for SWC to go DOWN before the loop advances
        waitSwcDown();
    }

    // Compute bias and scale from 6-point data
    imu.cal.ax_b = (readings[0][0] + readings[1][0]) / 2.0f;
    imu.cal.ay_b = (readings[2][1] + readings[3][1]) / 2.0f;
    imu.cal.az_b = (readings[4][2] + readings[5][2]) / 2.0f;

    float hrX = (readings[0][0] - readings[1][0]) / 2.0f;
    float hrY = (readings[2][1] - readings[3][1]) / 2.0f;
    float hrZ = (readings[4][2] - readings[5][2]) / 2.0f;
    imu.cal.ax_s = (fabsf(hrX) > 0.01f) ? 1.0f / hrX : 1.0f;
    imu.cal.ay_s = (fabsf(hrY) > 0.01f) ? 1.0f / hrY : 1.0f;
    imu.cal.az_s = (fabsf(hrZ) > 0.01f) ? 1.0f / hrZ : 1.0f;

    Serial.printf("\n[CAL] Accel bias:  X=%+.4f  Y=%+.4f  Z=%+.4f  g\n",
                  imu.cal.ax_b, imu.cal.ay_b, imu.cal.az_b);
    Serial.printf("[CAL] Accel scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  imu.cal.ax_s, imu.cal.ay_s, imu.cal.az_s);
    Serial.println(F("[CAL] ✓ ACCEL complete.\n"));

    // ── STAGE 3: MAG ─────────────────────────────────────────
    g_calibState = CalibState::RUNNING_MAG;

    Serial.println(F("[CAL] ══ STAGE 3/3: MAG ══"));
    Serial.println(F("[CAL] Rotate the drone slowly through ALL axes:"));
    Serial.println(F("[CAL]   — Roll left and right  (wing over wing)"));
    Serial.println(F("[CAL]   — Pitch nose up and down"));
    Serial.println(F("[CAL]   — Yaw slowly 360 degrees"));
    Serial.println(F("[CAL]   — Repeat a figure-8 motion"));
    Serial.printf( "[CAL] You have %d seconds. ROTATING NOW...\n\n",
                   MAG_DURATION_MS / 1000);

    float xmin =  1e9f, ymin =  1e9f, zmin =  1e9f;
    float xmax = -1e9f, ymax = -1e9f, zmax = -1e9f;

    uint32_t magEnd    = millis() + MAG_DURATION_MS;
    uint32_t lastPrint = 0;

    while (millis() < magEnd) {
        MPU_SensorData s;
        // Use readScaled so mag SLV0 pipeline is read correctly
        if (imu.readScaled(s)) {
            // Only update if mag data is fresh (non-zero)
            if (fabsf(s.mx_uT) > 0.1f || fabsf(s.my_uT) > 0.1f || fabsf(s.mz_uT) > 0.1f) {
                if (s.mx_uT < xmin) xmin = s.mx_uT;
                if (s.mx_uT > xmax) xmax = s.mx_uT;
                if (s.my_uT < ymin) ymin = s.my_uT;
                if (s.my_uT > ymax) ymax = s.my_uT;
                if (s.mz_uT < zmin) zmin = s.mz_uT;
                if (s.mz_uT > zmax) zmax = s.mz_uT;
            }
        }

        // Print every 10 s — just the time remaining, no data spam
        if (millis() - lastPrint >= 10000) {
            uint32_t rem = (millis() < magEnd) ? (magEnd - millis()) / 1000 : 0;
            Serial.printf("[CAL] Mag — %lu s remaining — keep rotating\n",
                          (unsigned long)rem);
            lastPrint = millis();
        }
        delay(10);
    }

    // Compute hard-iron bias and soft-iron scale
    imu.cal.mx_b = (xmax + xmin) / 2.0f;
    imu.cal.my_b = (ymax + ymin) / 2.0f;
    imu.cal.mz_b = (zmax + zmin) / 2.0f;

    float avgSpan = ((xmax - xmin) + (ymax - ymin) + (zmax - zmin)) / 3.0f;
    float rx = xmax - xmin, ry = ymax - ymin, rz = zmax - zmin;
    imu.cal.mx_s = (rx > 0.1f) ? avgSpan / rx : 1.0f;
    imu.cal.my_s = (ry > 0.1f) ? avgSpan / ry : 1.0f;
    imu.cal.mz_s = (rz > 0.1f) ? avgSpan / rz : 1.0f;

    Serial.printf("\n[CAL] Mag bias:  X=%+.2f  Y=%+.2f  Z=%+.2f  µT\n",
                  imu.cal.mx_b, imu.cal.my_b, imu.cal.mz_b);
    Serial.printf("[CAL] Mag scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  imu.cal.mx_s, imu.cal.my_s, imu.cal.mz_s);
    Serial.println(F("[CAL] ✓ MAG complete.\n"));

    // ── SAVE ─────────────────────────────────────────────────
    g_calibState = CalibState::SAVING;
    imu.cal.valid = true;
    imu.saveCalibration();

    Serial.println(F("╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║        CALIBRATION COMPLETE — SAVED TO FLASH    ║"));
    Serial.println(F("║  Arm the drone (SWA up) when ready to fly       ║"));
    Serial.println(F("╚══════════════════════════════════════════════════╝\n"));

    g_calibState = CalibState::DONE;
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskRC — Core 0, priority 3, 200 Hz
//
//  Arming  : SWA (CH7) only  — high = ARMED, low = DISARMED
//  Calib   : SWD (CH10) only — rising edge while DISARMED
//  No other switch or knob triggers any command
// ═════════════════════════════════════════════════════════════
static void taskRC(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();

    // Rising-edge detector for SWD
    bool swdPrev = false;

    for (;;) {
        rcReceiver.update();

        RCCommand cmd = rcReceiver.getCommand();

        // ── Calibration: SWD rising edge, DISARMED only ────────────
        // Fires ONCE on the LOW→HIGH transition of SWD.
        // Holding SWD high does NOT re-trigger.
        // SWA must be DOWN (DISARMED) — cannot start calib while armed.
        if (cmd.swdHigh && !swdPrev) {
            if (cmd.mode == FlightMode::DISARMED &&
               (g_calibState == CalibState::IDLE ||
                g_calibState == CalibState::DONE)) {
                g_calibState = CalibState::REQUESTED;
                Serial.println(F("[RC] Calibration started — SWD flipped UP."));
            } else if (cmd.mode != FlightMode::DISARMED) {
                Serial.println(F("[RC] Cannot calibrate while armed — disarm first (SWA down)."));
            }
        }

        swdPrev = cmd.swdHigh;
        vTaskDelayUntil(&lastWake, period);
    }
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskIMU — Core 1, priority 5, 500 Hz
//  Owns the SPI bus and the calibration state machine
// ═════════════════════════════════════════════════════════════
static void taskIMU(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(2);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastUs = micros();

    for (;;) {

        // ── Calibration requested ───────────────────────────
        if (g_calibState == CalibState::REQUESTED) {
            vTaskSuspend(hTaskPID);
            motorsOff();

            runAutonomousCalibration();   // blocks until all 3 stages done

            g_calibState = CalibState::IDLE;
            vTaskResume(hTaskPID);
            lastUs = micros();   // reset dt — calibration takes minutes
            lastWake = xTaskGetTickCount();
            continue;
        }

        // ── Normal IMU cycle ────────────────────────────────
        uint32_t nowUs = micros();
        float dt = (nowUs - lastUs) * 1e-6f;
        lastUs = nowUs;

        MPU_SensorData s;
        if (imu.readScaled(s)) {
            MPU_Attitude att;
            imu.mahonyUpdate(s, dt, att);

            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                g_state.roll_deg  = att.roll;
                g_state.pitch_deg = att.pitch;
                g_state.yaw_deg   = att.yaw;
                g_state.gx_dps    = s.gx_dps;
                g_state.gy_dps    = s.gy_dps;
                g_state.gz_dps    = s.gz_dps;
                g_state.loopCount++;
                xSemaphoreGive(g_flightMutex);
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskPID — Core 1, priority 4, 500 Hz
// ═════════════════════════════════════════════════════════════
static void taskPID(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(2);
    TickType_t lastWake = xTaskGetTickCount();
    const float dt = 0.002f;

    for (;;) {
        RCCommand cmd = rcReceiver.getCommand();

        if (cmd.mode == FlightMode::DISARMED ||
            cmd.mode == FlightMode::FAILSAFE) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = 0.0f;
                g_state.motorRL = g_state.motorRR = 0.0f;
                g_state.rc      = cmd;
                xSemaphoreGive(g_flightMutex);
            }
            vTaskDelayUntil(&lastWake, period);
            continue;
        }

        float roll_deg, pitch_deg, gx, gy, gz;
        if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
            roll_deg  = g_state.roll_deg;
            pitch_deg = g_state.pitch_deg;
            gx = g_state.gx_dps;
            gy = g_state.gy_dps;
            gz = g_state.gz_dps;
            xSemaphoreGive(g_flightMutex);
        } else { vTaskDelayUntil(&lastWake, period); continue; }

        const float MAX_ANGLE = 30.0f;
        const float MAX_RATE  = 200.0f;
        float rollOut = 0, pitchOut = 0, yawOut = 0;

        if (cmd.mode == FlightMode::ANGLE) {
            float rRSP = pidAngleRoll .update(cmd.roll  * MAX_ANGLE - roll_deg,  dt);
            float pRSP = pidAnglePitch.update(cmd.pitch * MAX_ANGLE - pitch_deg, dt);
            rollOut  = pidRateRoll .update(rRSP               - gx, dt);
            pitchOut = pidRatePitch.update(pRSP               - gy, dt);
            yawOut   = pidRateYaw  .update(cmd.yaw * MAX_RATE - gz, dt);
        } else {
            rollOut  = pidRateRoll .update(cmd.roll  * MAX_RATE - gx, dt);
            pitchOut = pidRatePitch.update(cmd.pitch * MAX_RATE - gy, dt);
            yawOut   = pidRateYaw  .update(cmd.yaw   * MAX_RATE - gz, dt);
        }

        float thr = cmd.throttle;
        float fl = constrain(thr + rollOut - pitchOut - yawOut, 0.0f, 1.0f);
        float fr = constrain(thr - rollOut - pitchOut + yawOut, 0.0f, 1.0f);
        float rl = constrain(thr + rollOut + pitchOut + yawOut, 0.0f, 1.0f);
        float rr = constrain(thr - rollOut + pitchOut - yawOut, 0.0f, 1.0f);
        writeMotors(fl, fr, rl, rr);

        if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
            g_state.motorFL = fl;  g_state.motorFR = fr;
            g_state.motorRL = rl;  g_state.motorRR = rr;
            g_state.armed   = true;
            g_state.rc      = cmd;
            xSemaphoreGive(g_flightMutex);
        }
        vTaskDelayUntil(&lastWake, period);
    }
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskSerial — Core 0, priority 1, 20 Hz
//  Status telemetry + calibration progress banner
//  NO commands that block on Serial.read()
// ═════════════════════════════════════════════════════════════
static void taskSerial(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t tick = 0;

    Serial.println(F("\n╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║  FlySky iBUS + MPU-9250 Standalone Controller    ║"));
    Serial.println(F("║  No keyboard needed — all commands via RC        ║"));
    Serial.println(F("╠══════════════════════════════════════════════════╣"));
    Serial.println(F("║  SWA (CH7) up = ARMED      down = DISARMED       ║"));
    Serial.println(F("║  SWB (CH8) down = ANGLE    up   = ACRO           ║"));
    Serial.println(F("║  SWD (CH10) flip UP while disarmed = CALIBRATE   ║"));
    Serial.println(F("║  SWC (CH9) = confirm switch during accel calib   ║"));
    Serial.println(F("╚══════════════════════════════════════════════════╝\n"));

    for (;;) {

        // ── Calibration in progress — taskSerial is silent.
        //    All messages are printed directly inside runAutonomousCalibration()
        //    at the exact moment the user needs to act. No heartbeat spam.
        if (g_calibState != CalibState::IDLE &&
            g_calibState != CalibState::DONE) {
            tick++;
            vTaskDelayUntil(&lastWake, period);
            continue;
        }

        // ── 1 Hz flight telemetry ──────────────────────────
        if (tick % 20 == 0) {
            FlightState s;
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                s = g_state;
                xSemaphoreGive(g_flightMutex);
            }

            const char* mStr = "???";
            switch (s.rc.mode) {
                case FlightMode::DISARMED: mStr = "DISARMD"; break;
                case FlightMode::ANGLE:    mStr = "ANGLE  "; break;
                case FlightMode::ACRO:     mStr = "ACRO   "; break;
                case FlightMode::FAILSAFE: mStr = "FAILSFE"; break;
            }
            Serial.printf("[%6lu] %s | R=%+6.1f P=%+6.1f Y=%6.1f | "
                          "T=%.2f R=%+.2f P=%+.2f Y=%+.2f | "
                          "MOT %.2f %.2f %.2f %.2f | RC@%.0fHz | "
                          "BMP T=%.1fC P=%.1fhPa ALT=%.1fm %s\n",
                          (unsigned long)tick, mStr,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
                          s.rc.throttle, s.rc.roll, s.rc.pitch, s.rc.yaw,
                          s.motorFL, s.motorFR, s.motorRL, s.motorRR,
                          rcReceiver.getFrameRate(),
                          s.bmpTemp_c, s.bmpPressure_hpa, s.bmpAltitude_m,
                          s.bmpValid ? "OK" : "NO_BMP");
        }

        tick++;
        vTaskDelayUntil(&lastWake, period);
    }
}


// ═════════════════════════════════════════════════════════════
//  TASK: taskBMP — Core 0, priority 1, 20 Hz
//  Reads BMP280 pressure, temperature, and calculated altitude.
// ═════════════════════════════════════════════════════════════
static void taskBMP(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        BMP280Data b;
        if (bmp280.read(b)) {
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpTemp_c = b.temperature_c;
                g_state.bmpPressure_hpa = b.pressure_hpa;
                g_state.bmpAltitude_m = b.altitude_m;
                g_state.bmpValid = b.valid;
                xSemaphoreGive(g_flightMutex);
            }
        } else {
            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.bmpValid = false;
                xSemaphoreGive(g_flightMutex);
            }
        }
        vTaskDelayUntil(&lastWake, period);
    }
}


// ═════════════════════════════════════════════════════════════
//  TASK: taskWiFi — Core 0, priority 1
//  Handles browser HTTP requests for /telemetry.
// ═════════════════════════════════════════════════════════════
static void taskWiFi(void* /*pv*/)
{
    telemetryWiFi.setTelemetryProvider(provideTelemetry);
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

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_MPU_CS);
    delay(10);

    g_flightMutex = xSemaphoreCreateMutex();
    configASSERT(g_flightMutex);
    memset(&g_state, 0, sizeof(g_state));

    Serial.print(F("[BOOT] MPU-9250... "));
    if (!imu.begin()) {
        Serial.println(F("FAILED — check wiring. Halting."));
        while (true) delay(1000);
    }
    Serial.println(F("OK"));

    Serial.println(F("[BOOT] BMP280 scan..."));
    bmp280.scanI2C(PIN_BMP_SDA, PIN_BMP_SCL, 100000);

    Serial.print(F("[BOOT] BMP280... "));
    if (bmp280.beginAuto(PIN_BMP_SDA, PIN_BMP_SCL, 100000)) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("not found — altitude telemetry disabled."));
    }

    Serial.print(F("[BOOT] NVS calibration... "));
    if (imu.loadCalibration()) {
        Serial.println(F("loaded."));
        imu.printCalibration();
    } else {
        Serial.println(F("not found."));
        Serial.println(F("[BOOT] >>> Trigger calibration: flip SWD up or turn VrA CW (SWA must be DOWN)"));
    }

    rcReceiver.begin(PIN_IBUS_RX, PIN_IBUS_TX, 2);
    Serial.println(F("[BOOT] iBUS receiver ready."));

    xTaskCreatePinnedToCore(taskRC,     "RC",     3072, nullptr, 3, &hTaskRC,     0);
    xTaskCreatePinnedToCore(taskSerial, "Serial", 4096, nullptr, 1, &hTaskSerial, 0);
    xTaskCreatePinnedToCore(taskWiFi,   "WiFi",   4096, nullptr, 1, &hTaskWiFi,   0);
    xTaskCreatePinnedToCore(taskBMP,    "BMP280", 3072, nullptr, 1, &hTaskBMP,    0);
    xTaskCreatePinnedToCore(taskIMU,    "IMU",    8192, nullptr, 5, &hTaskIMU,    1);
    xTaskCreatePinnedToCore(taskPID,    "PID",    4096, nullptr, 4, &hTaskPID,    1);

    Serial.println(F("[BOOT] All tasks running."));
}

void loop() { vTaskDelay(portMAX_DELAY); }
