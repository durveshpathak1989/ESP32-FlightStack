/**
 * ============================================================
 *  MotorControl.cpp — ESC PWM Motor Control Implementation
 *  Adafruit HUZZAH32 / ESP32 · X-frame Quadcopter
 * ============================================================
 *
 *  ESC 3-pin wiring reminder:
 *    BLACK (pin 1) = GND   → PDB GND pad
 *    RED   (pin 2) = 5V    → PDB 5V BEC
 *    WHITE (pin 3) = SIG   → 33Ω series → HUZZAH32 GPIO
 *
 *  Uses ESP32 LEDC peripheral for hardware PWM.
 *  Resolution: 16-bit. Period: 20 ms (50 Hz).
 *  Duty calculation:
 *    counts = (pulse_µs / 20000) × 65535
 *    1000 µs → 3276 counts
 *    2000 µs → 6553 counts
 * ============================================================
 */

#include "MotorControl.h"

// ─────────────────────────────────────────────────────────────
//  Internal state
// ─────────────────────────────────────────────────────────────
static MotorState_t s_state = {0.0f, 0.0f, 0.0f, 0.0f, false};

// GPIO pin array (core 3.x: pin is the key for ledcAttach / ledcWrite)
static const uint8_t s_pins[4] = {MOTOR_PIN_FL, MOTOR_PIN_FR, MOTOR_PIN_RL, MOTOR_PIN_RR};

// ─────────────────────────────────────────────────────────────
//  Private helpers
// ─────────────────────────────────────────────────────────────

/** Convert microseconds to 16-bit LEDC duty count for 50 Hz */
static uint32_t _usToDuty(uint16_t us) {
    // period = 20000 µs, resolution = 65535 counts
    return (uint32_t)((uint32_t)us * 65535UL / 20000UL);
}

/** Write a specific pulse width in µs to one motor GPIO pin.
 *  Core 3.x: ledcWrite(pin, duty) — pin is the key, not channel number.
 */
static void _writeUs(uint8_t pin, uint16_t us) {
    ledcWrite(pin, _usToDuty(us));
}

/** Clamp float to [lo, hi] */
static float _clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/** Send the same µs to all four motors */
static void _allUs(uint16_t us) {
    for (int i = 0; i < 4; i++) _writeUs(s_pins[i], us);
}

/** Attach all four LEDC channels to their GPIO pins.
 *  Core 3.x API: ledcAttach(pin, freq, resolution) — channel auto-assigned.
 *  ledcWrite(pin, duty) uses pin as the key, not channel number.
 */
static void _attachChannels(void) {
    for (int i = 0; i < 4; i++) {
        // core >=3.0: ledcAttach(pin, freq, resolution_bits)
        ledcAttach(s_pins[i], MOTOR_PWM_FREQ_HZ, MOTOR_PWM_RESOLUTION);
    }
}

// ─────────────────────────────────────────────────────────────
//  Public API implementation
// ─────────────────────────────────────────────────────────────

void motorBegin(void) {
    Serial.println(F("[MOTOR] Initialising PWM channels..."));

    // Configure GPIO27 (MPU INT) as input — safety check so it is
    // never accidentally driven as an output
    pinMode(27, INPUT);

    _attachChannels();
    _allUs(MOTOR_US_MIN);      // safe starting position
    s_state = {0.0f, 0.0f, 0.0f, 0.0f, false};

    Serial.printf("[MOTOR] FL=GPIO%d  FR=GPIO%d  RL=GPIO%d  RR=GPIO%d\n",
                  MOTOR_PIN_FL, MOTOR_PIN_FR, MOTOR_PIN_RL, MOTOR_PIN_RR);
    Serial.printf("[MOTOR] PWM: %d Hz  16-bit  1000–2000 µs\n",
                  MOTOR_PWM_FREQ_HZ);
    Serial.println(F("[MOTOR] All motors at minimum (1000 µs). Ready."));
}

// ─────────────────────────────────────────────────────────────
bool motorArm(void) {
    Serial.println(F("[MOTOR] Arming sequence..."));
    _allUs(MOTOR_US_ARM);      // send sub-minimum to enter arm mode
    delay(500);
    _allUs(MOTOR_US_MIN);      // step up to minimum
    delay(2000);               // wait for ESC init beeps
    s_state.armed = true;
    Serial.println(F("[MOTOR] Armed. ESC ready."));
    return true;
}

// ─────────────────────────────────────────────────────────────
void motorDisarm(void) {
    _allUs(MOTOR_US_ARM);
    s_state.armed  = false;
    s_state.fl = s_state.fr = s_state.rl = s_state.rr = 0.0f;
    Serial.println(F("[MOTOR] Disarmed."));
}

// ─────────────────────────────────────────────────────────────
void motorOff(void) {
    _allUs(MOTOR_US_ARM);
    s_state.armed  = false;
    s_state.fl = s_state.fr = s_state.rl = s_state.rr = 0.0f;
}

// ─────────────────────────────────────────────────────────────
uint16_t motorUsFromThrottle(float t) {
    t = _clampf(t, 0.0f, 1.0f);
    return (uint16_t)(MOTOR_US_MIN + t * (float)(MOTOR_US_MAX - MOTOR_US_MIN));
}

// ─────────────────────────────────────────────────────────────
void motorSet(float fl, float fr, float rl, float rr) {
    fl = _clampf(fl, MOTOR_THROTTLE_MIN, MOTOR_THROTTLE_MAX);
    fr = _clampf(fr, MOTOR_THROTTLE_MIN, MOTOR_THROTTLE_MAX);
    rl = _clampf(rl, MOTOR_THROTTLE_MIN, MOTOR_THROTTLE_MAX);
    rr = _clampf(rr, MOTOR_THROTTLE_MIN, MOTOR_THROTTLE_MAX);

    _writeUs(MOTOR_PIN_FL, motorUsFromThrottle(fl));
    _writeUs(MOTOR_PIN_FR, motorUsFromThrottle(fr));
    _writeUs(MOTOR_PIN_RL, motorUsFromThrottle(rl));
    _writeUs(MOTOR_PIN_RR, motorUsFromThrottle(rr));

    s_state.fl = fl; s_state.fr = fr;
    s_state.rl = rl; s_state.rr = rr;
}

// ─────────────────────────────────────────────────────────────
void motorSetAll(float throttle) {
    motorSet(throttle, throttle, throttle, throttle);
}

// ─────────────────────────────────────────────────────────────
//  X-frame mixer
//
//  X configuration (top-down view, + = clockwise rotation):
//
//        FRONT
//    FL(CCW) ↺  ↻ FR(CW)
//         \    /
//          \  /
//     roll →→→← roll
//          /  \
//         /    \
//    RL(CW) ↻  ↺ RR(CCW)
//        REAR
//
//  Torque directions:
//    FL CCW: contributes +yaw (nose right)
//    FR CW:  contributes -yaw (nose left)
//    RL CW:  contributes +yaw
//    RR CCW: contributes -yaw
//
//  Mixer equations:
//    FL = thr + pitch + roll - yaw    (CCW)
//    FR = thr - pitch - roll - yaw    (CW)   ← negate pitch & roll
//    RL = thr - pitch + roll + yaw    (CW)
//    RR = thr + pitch - roll + yaw    (CCW)
//
//  Sign conventions (right-hand, NED-ish):
//    pitch > 0 → nose pitches DOWN (forward, increases FL/RR)
//    roll  > 0 → right side goes DOWN (right, increases FL/RL)
//    yaw   > 0 → nose rotates RIGHT  (CW from above, FL/RL higher)
// ─────────────────────────────────────────────────────────────
void motorMixerX(float throttle, float roll, float pitch, float yaw) {
    if (!s_state.armed) return;

    // Clamp inputs
    throttle = _clampf(throttle, 0.0f, MOTOR_THROTTLE_MAX);
    roll     = _clampf(roll,    -1.0f, 1.0f);
    pitch    = _clampf(pitch,   -1.0f, 1.0f);
    yaw      = _clampf(yaw,     -1.0f, 1.0f);

    // Scale authority: roll/pitch/yaw each get 25% of headroom
    // so at full throttle there is still room for corrections
    const float AUTH = 0.25f;
    float r = roll  * AUTH;
    float p = pitch * AUTH;
    float y = yaw   * AUTH;

    float fl = throttle + p + r - y;
    float fr = throttle - p - r - y;
    float rl = throttle - p + r + y;
    float rr = throttle + p - r + y;

    // If any motor would exceed max, scale all down proportionally
    float hi = fl;
    if (fr > hi) hi = fr;
    if (rl > hi) hi = rl;
    if (rr > hi) hi = rr;
    if (hi > MOTOR_THROTTLE_MAX) {
        float scale = MOTOR_THROTTLE_MAX / hi;
        fl *= scale; fr *= scale; rl *= scale; rr *= scale;
    }

    // Enforce minimum — if armed, never send below idle to prevent
    // motors stopping mid-flight on aggressive manoeuvres
    float idle = (throttle > MOTOR_IDLE_THROTTLE) ? MOTOR_IDLE_THROTTLE : 0.0f;
    fl = _clampf(fl, idle, MOTOR_THROTTLE_MAX);
    fr = _clampf(fr, idle, MOTOR_THROTTLE_MAX);
    rl = _clampf(rl, idle, MOTOR_THROTTLE_MAX);
    rr = _clampf(rr, idle, MOTOR_THROTTLE_MAX);

    motorSet(fl, fr, rl, rr);
}

// ─────────────────────────────────────────────────────────────
MotorState_t motorGetState(void) {
    return s_state;
}

// ─────────────────────────────────────────────────────────────
void motorEscArm(void) {
    Serial.println(F("[ESC] Sending arm pulse (1000 µs × 3 s)..."));
    _allUs(MOTOR_US_MIN);
    delay(3000);
    Serial.println(F("[ESC] ESC armed and ready."));
    s_state.armed = true;
}

// ─────────────────────────────────────────────────────────────
//  ESC CALIBRATION ROUTINE
//  Blocks until complete. Follow Serial prompts at 115200 baud.
//  PROPS MUST BE OFF — NEVER calibrate with propellers attached.
// ─────────────────────────────────────────────────────────────
void motorEscCalibrate(void) {
    // Ensure channels are set up
    _attachChannels();

    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════════════════════╗"));
    Serial.println(F("║          ESC ENDPOINT CALIBRATION ROUTINE           ║"));
    Serial.println(F("╠══════════════════════════════════════════════════════╣"));
    Serial.println(F("║  ⚠  REMOVE ALL PROPELLERS BEFORE CONTINUING  ⚠      ║"));
    Serial.println(F("║                                                      ║"));
    Serial.println(F("║  Motor wiring reminder:                              ║"));
    Serial.println(F("║    BLACK = GND  → PDB GND pad                        ║"));
    Serial.println(F("║    RED   = 5V   → PDB 5V BEC                         ║"));
    Serial.println(F("║    WHITE = SIG  → 33Ω → HUZZAH32 GPIO                ║"));
    Serial.println(F("║                                                      ║"));
    Serial.println(F("║  X-frame layout:                                     ║"));
    Serial.println(F("║    FL (GPIO25) CCW │ FR (GPIO15) CW                  ║"));
    Serial.println(F("║    RL (GPIO14) CW  │ RR (GPIO32) CCW                 ║"));
    Serial.println(F("╚══════════════════════════════════════════════════════╝"));
    Serial.println(F(""));
    Serial.println(F("[CAL] Step 1/5 — Disconnect battery now."));
    Serial.println(F("[CAL] Press any key + Enter when battery is disconnected."));

    // Wait for user
    while (!Serial.available()) delay(100);
    while (Serial.available()) Serial.read();

    // ── Step 2: Send high throttle BEFORE connecting battery ──
    Serial.println(F("[CAL] Step 2/5 — Sending HIGH signal (2000 µs) to all ESCs."));
    _allUs(MOTOR_US_CAL_HIGH);
    delay(200);
    Serial.println(F("[CAL] HIGH signal active."));
    Serial.println(F(""));
    Serial.println(F("[CAL] Step 3/5 — Connect battery NOW."));
    Serial.println(F("[CAL] Wait for ESC startup beeps, then press any key + Enter."));
    Serial.println(F("[CAL] (Most ESCs play a short melody then a double-beep)"));

    while (!Serial.available()) delay(100);
    while (Serial.available()) Serial.read();

    // ── Step 3: Drop to low throttle ──
    Serial.println(F("[CAL] Step 4/5 — Sending LOW signal (1000 µs) to all ESCs."));
    _allUs(MOTOR_US_MIN);
    delay(200);
    Serial.println(F("[CAL] LOW signal active."));
    Serial.println(F("[CAL] Wait for confirmation beeps from ESC (usually 2–3 beeps)."));
    Serial.println(F("[CAL] Then press any key + Enter."));

    while (!Serial.available()) delay(100);
    while (Serial.available()) Serial.read();

    // ── Step 4: Short arm ──
    Serial.println(F("[CAL] Step 5/5 — Sending arm pulse (900 µs)."));
    _allUs(MOTOR_US_ARM);
    delay(500);
    _allUs(MOTOR_US_MIN);
    delay(1500);

    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════════════════════════╗"));
    Serial.println(F("║  ESC CALIBRATION COMPLETE                            ║"));
    Serial.println(F("║  Endpoints saved in ESC EEPROM — not needed again.   ║"));
    Serial.println(F("║                                                      ║"));
    Serial.println(F("║  Next steps:                                         ║"));
    Serial.println(F("║  1. Power-cycle the drone (disconnect battery)        ║"));
    Serial.println(F("║  2. Remove calibration call from code                 ║"));
    Serial.println(F("║  3. Call motorBegin() + motorEscArm() for normal use  ║"));
    Serial.println(F("╚══════════════════════════════════════════════════════╝"));
    Serial.println(F(""));

    s_state = {0.0f, 0.0f, 0.0f, 0.0f, false};
}
