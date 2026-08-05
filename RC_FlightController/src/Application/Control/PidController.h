#pragma once

#include <algorithm>

namespace pid_detail {
constexpr float kPi = 3.14159265358979323846f;

inline float clamp(float value, float low, float high) {
    return std::max(low, std::min(value, high));
}
}  // namespace pid_detail

// Small, stateful PID controller used by both angle and rate loops.
// Public diagnostic terms make telemetry explicit without duplicating math.
class PidController {
public:
    float kp;
    float ki;
    float kd;
    float integral = 0.0f;
    float prevError = 0.0f;
    float iLimit;
    float prevMeasurement = 0.0f;
    float filteredMeasurementRate = 0.0f;
    bool hasPrevMeasurement = false;
    bool hasFilteredMeasurementRate = false;
    float lastP = 0.0f;
    float lastI = 0.0f;
    float lastD = 0.0f;
    float lastOut = 0.0f;

    PidController(float proportional, float integralGain, float derivative,
                  float integralLimit = 50.0f)
        : kp(proportional), ki(integralGain), kd(derivative),
          iLimit(integralLimit) {}

    float update(float error, float dt) {
        integral = pid_detail::clamp(integral + error * dt, -iLimit, iLimit);
        const float errorRate = dt > 0.000001f ? (error - prevError) / dt : 0.0f;
        prevError = error;
        lastP = kp * error;
        lastI = ki * integral;
        lastD = kd * errorRate;
        lastOut = lastP + lastI + lastD;
        return lastOut;
    }

    float updateDOnMeasurement(float error, float measurement, float dt,
                               float derivativeLpfHz) {
        integral = pid_detail::clamp(integral + error * dt, -iLimit, iLimit);
        float measurementRate = 0.0f;
        if (hasPrevMeasurement && dt > 0.000001f)
            measurementRate = (measurement - prevMeasurement) / dt;

        if (derivativeLpfHz > 0.0f && dt > 0.000001f) {
            const float tau = 1.0f / (2.0f * pid_detail::kPi * derivativeLpfHz);
            const float alpha = pid_detail::clamp(dt / (tau + dt), 0.0f, 1.0f);
            if (!hasFilteredMeasurementRate) {
                filteredMeasurementRate = measurementRate;
                hasFilteredMeasurementRate = true;
            } else {
                filteredMeasurementRate +=
                    alpha * (measurementRate - filteredMeasurementRate);
            }
        } else {
            filteredMeasurementRate = measurementRate;
            hasFilteredMeasurementRate = true;
        }

        prevError = error;
        prevMeasurement = measurement;
        hasPrevMeasurement = true;
        lastP = kp * error;
        lastI = ki * integral;
        lastD = -kd * filteredMeasurementRate;
        lastOut = lastP + lastI + lastD;
        return lastOut;
    }

    void reset() {
        integral = 0.0f;
        prevError = 0.0f;
        prevMeasurement = 0.0f;
        filteredMeasurementRate = 0.0f;
        hasPrevMeasurement = false;
        hasFilteredMeasurementRate = false;
        lastP = lastI = lastD = lastOut = 0.0f;
    }
};
