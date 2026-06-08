//**
 /* ============================================================
 *  MotorControl.h — ESC PWM Motor Control
 *  Adafruit HUZZAH32 / ESP32 · X-frame Quadcopter
 * ============================================================
 *
 *  Motor layout — X configuration (top-down view):
 *
 *        FRONT
 *    FL (CCW)  FR (CW)
 *        \    /
 *         \  /
 *          \/
 *          /\
 *         /  \
 *        /    \
 *    RL (CW)  RR (CCW)
 *        REAR
 *
 *  ESC wiring (3-pin connector per motor):
 *    BLACK  — GND  → PDB GND / ESC GND pad
 *    RED    — 5V   → PDB 5V BEC output
 *    WHITE  — SIG  → 33Ω → HUZZAH32 GPIO (signal)
 *
 *  Pin assignments (confirmed):
 *    FL (CCW) = GPIO 25   (A1,  left  rail)
 *    FR (CW)  = GPIO 15   (15/A8, right rail)
 *    RL (CW)  = GPIO 14   (14/A6, right rail)
 *    RR (CCW) = GPIO 32   (32/A7, right rail)  ggvc                                                                                                                                                                                                                                                                                                
 *
 *  ESC protocol: Standard PWM (1000–2000 µs @ 50 Hz)
 *    1000 µs = minimum throttle / armed
 *    2000 µs = full throttle
 *    <1000 µs sent during calibration sequence
 *
 *  ESC calibration:
 *    Call motorEscCalibrate() ONCE with props OFF and battery
 *    connected. The routine programs the ESC high/low endpoints.
 *    After calibration, call motorBegin() for normal operation.
 *
 *  Mixer (X-frame):
 *    FL = throttle + pitch + roll - yaw   (CCW)
 *    FR = throttle - pitch - roll - yaw   (CW)   ← note: FR inverted pitch/roll
 *    RL = throttle - pitch + roll + yaw   (CW)
 *    RR = throttle + pitch - roll + yaw   (CCW)
 *
 *  All throttle values are 0.0f (off) to 1.0f (full).
 * ============================================================
 */

#pragma once
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// ─────────────────────────────────────────────────────────────
//  Pin assignments — do NOT change unless rewiring the board
// ─────────────────────────────────────────────────────────────
#define MOTOR_PIN_FL   25    // Front-Left  CCW  A1 left rail
#define MOTOR_PIN_FR   15    // Front-Right CW   15/A8 right rail
#define MOTOR_PIN_RL   14    // Rear-Left   CW   14/A6 right rail
#define MOTOR_PIN_RR   32    // Rear-Right  CCW  32/A7 right rail

// ─────────────────────────────────────────────────────────────
//  ESC PWM parameters
// ─────────────────────────────────────────────────────────────
#define MOTOR_PWM_FREQ_HZ      50      // Standard ESC: 50 Hz (20 ms period)
#define MOTOR_PWM_RESOLUTION   16      // 16-bit: 0–65535 counts
#define MOTOR_US_MIN         1000      // µs → disarmed / minimum
#define MOTOR_US_MAX         2000      // µs → full throttle
#define MOTOR_US_ARM          900      // µs → below min, used to arm ESC
#define MOTOR_US_CAL_HIGH    2100      // µs → above max, used in calibration
#define MOTOR_US_IDLE        1050      // µs → just above min, motor barely spinning

// LEDC channels — NOT used in core 3.x (ledcAttach auto-assigns channels)
// Core 3.x uses pin number as the key for ledcWrite(), not channel index.
// Kept here for reference only.
// #define MOTOR_CH_FL   0
// #define MOTOR_CH_FR   1
// #define MOTOR_CH_RL   2
// #define MOTOR_CH_RR   3

// ─────────────────────────────────────────────────────────────
//  Safety limits
// ─────────────────────────────────────────────────────────────
#define MOTOR_THROTTLE_MAX     0.95f   // never command 100% (leaves headroom)
#define MOTOR_THROTTLE_MIN     0.0f
#define MOTOR_IDLE_THROTTLE    0.05f   // ~1050 µs, props spinning slowly

// ─────────────────────────────────────────────────────────────
//  Motor state
// ─────────────────────────────────────────────────────────────
typedef struct {
    float fl;   // 0.0 – 1.0
    float fr;
    float rl;
    float rr;
    bool  armed;
} MotorState_t;

// ─────────────────────────────────────────────────────────────
//  Public API
// ─────────────────────────────────────────────────────────────

/**
 * motorBegin()
 * Call once in setup() AFTER motorEscCalibrate() has been done.
 * Configures LEDC channels, sets all motors to minimum (1000 µs).
 */
void motorBegin(void);

/**
 * motorArm()
 * Sends arm pulse (MOTOR_US_ARM) then ramps to MOTOR_US_MIN.
 * Wait at least 3 seconds after battery connect before calling.
 * Returns true when ESC beep sequence is expected to complete.
 */
bool motorArm(void);

/**
 * motorDisarm()
 * Sets all outputs to MOTOR_US_ARM (below minimum) — ESC idles.
 */
void motorDisarm(void);

/**
 * motorSet(fl, fr, rl, rr)
 * Set individual motor throttles 0.0–1.0 directly.
 * Clamps to [MOTOR_THROTTLE_MIN, MOTOR_THROTTLE_MAX].
 */
void motorSet(float fl, float fr, float rl, float rr);

/**
 * motorSetAll(throttle)
 * Set all four motors to the same throttle.
 */
void motorSetAll(float throttle);

/**
 * motorOff()
 * Immediately stop all motors (sends < 1000 µs).
 * Call this on any safety fault or disarm.
 */
void motorOff(void);

/**
 * motorMixerX(throttle, roll, pitch, yaw)
 * X-frame mixer. Inputs are all –1.0 to +1.0 (except throttle 0–1).
 *   roll  : +right wing down  (right = +)
 *   pitch : +nose down        (forward = +)
 *   yaw   : +nose right       (clockwise = +)
 * Internally clamps and applies to FL/FR/RL/RR via the X mixer.
 * Does nothing if disarmed.
 */
void motorMixerX(float throttle, float roll, float pitch, float yaw);

/**
 * motorGetState()
 * Returns current motor state (last commanded values).
 */
MotorState_t motorGetState(void);

/**
 * motorEscCalibrate()
 * ──────────────────────────────────────────────────────────────
 * ESC ENDPOINT CALIBRATION ROUTINE
 *
 * IMPORTANT — READ BEFORE CALLING:
 *   1. REMOVE ALL PROPELLERS before starting.
 *   2. Disconnect battery.
 *   3. Call this function — it will block and print instructions
 *      over Serial at 115200 baud.
 *   4. Follow the on-screen prompts.
 *
 * Most ESCs use this sequence:
 *   a) Power off ESC
 *   b) Send HIGH signal (2000+ µs)
 *   c) Power on ESC — ESC beeps to acknowledge high point
 *   d) Send LOW signal (1000 µs)
 *   e) ESC beeps to acknowledge low point — calibration saved
 *
 * After this routine completes, power-cycle and call motorBegin().
 * You only need to do this ONCE per ESC (values stored in ESC EEPROM).
 * ──────────────────────────────────────────────────────────────
 */
void motorEscCalibrate(void);

/**
 * motorEscArm()
 * Fast arm sequence for normal startup after calibration is done.
 * Sends 1000 µs for 3 s (ESC init tone), then holds ready.
 */
void motorEscArm(void);

/**
 * motorUsFromThrottle(t)
 * Helper: convert throttle 0.0–1.0 to microseconds 1000–2000.
 */
uint16_t motorUsFromThrottle(float t);

#endif // MOTOR_CONTROL_H
