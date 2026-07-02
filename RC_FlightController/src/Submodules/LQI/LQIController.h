/*
 * Name: LQIController.h
 * Use: Lightweight per-axis LQI attitude/rate controller for ESP32 flight loops.
 */

#pragma once
#ifndef LQI_CONTROLLER_H
#define LQI_CONTROLLER_H

#include <Arduino.h>

enum class LQIAxis : uint8_t {
    Roll = 0,
    Pitch = 1,
    Yaw = 2
};

struct LQIGains {
    float kAngle;
    float kRate;
    float kIntegral;
};

struct LQIContributions {
    float angle;
    float rate;
    float integral;
    float output;
    float integrator;
    bool outputLimited;
};

class LQIController {
public:
    LQIController();

    void begin();
    void reset();
    void resetAxis(LQIAxis axis);

    void setGains(LQIAxis axis, float kAngle, float kRate, float kIntegral);
    void setOutputLimits(float rollLimit, float pitchLimit, float yawLimit);
    void setOutputLimit(LQIAxis axis, float limit);
    void setIntegratorLimit(float limit);

    // Inputs are state errors: measured - target. Angle is radians, rate is rad/s.
    // Output is a normalized mixer correction matching the existing PID path.
    float updateAngleAxis(LQIAxis axis, float angleErrorRad, float rateErrorRadPerSec, float dtSec);

    // Rate-only path for ACRO. rateErrorRadPerSec is measured - target.
    float updateRateAxis(LQIAxis axis, float rateErrorRadPerSec, float dtSec);

    float lastAngleContribution(LQIAxis axis) const;
    float lastRateContribution(LQIAxis axis) const;
    float lastIntegralContribution(LQIAxis axis) const;
    float lastOutput(LQIAxis axis) const;
    float integrator(LQIAxis axis) const;
    bool outputLimited(LQIAxis axis) const;
    bool anyOutputLimited() const;

private:
    struct AxisState {
        LQIGains gains;
        float outputLimit;
        float integrator;
        LQIContributions last;
    };

    AxisState _axis[3];
    float _integratorLimit;

    static uint8_t _index(LQIAxis axis);
    float _update(LQIAxis axis, float angleErrorRad, float rateErrorRadPerSec, float dtSec, bool useAngle);
};

#endif // LQI_CONTROLLER_H
