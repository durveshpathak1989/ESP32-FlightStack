#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_PIDControllerInput {
    flight::AttitudeState attitude;  // AHRS output, rad and rad/s, 400 Hz.
    flight::PilotCommand pilot;      // Receiver command, normalized sticks.
    flight::FlightConfig config;     // Current tuning and limits.
    bool isArmed = false;
    bool angleMode = true;
    bool imuValid = true;
    std::uint64_t dtUs = 2500;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_PIDControllerOutput {
    flight::MotorCommand motors;

    struct Debug {
        float targetRollDeg = 0.0f;
        float targetPitchDeg = 0.0f;
        float targetYawDeg = 0.0f;
        float targetRollRateDps = 0.0f;
        float targetPitchRateDps = 0.0f;
        float targetYawRateDps = 0.0f;

        float attErrRoll = 0.0f;
        float attErrPitch = 0.0f;
        float attErrYaw = 0.0f;
        float attPRoll = 0.0f;
        float attIRoll = 0.0f;
        float attDRoll = 0.0f;
        float attPPitch = 0.0f;
        float attIPitch = 0.0f;
        float attDPitch = 0.0f;
        float attPYaw = 0.0f;
        float attIYaw = 0.0f;
        float attDYaw = 0.0f;

        float rateErrRoll = 0.0f;
        float rateErrPitch = 0.0f;
        float rateErrYaw = 0.0f;
        float ratePRoll = 0.0f;
        float rateIRoll = 0.0f;
        float rateDRoll = 0.0f;
        float ratePPitch = 0.0f;
        float rateIPitch = 0.0f;
        float rateDPitch = 0.0f;
        float ratePYaw = 0.0f;
        float rateIYaw = 0.0f;
        float rateDYaw = 0.0f;

        float rollOutput = 0.0f;
        float pitchOutput = 0.0f;
        float yawOutput = 0.0f;
        float throttle = 0.0f;
        bool yawHoldActive = false;
        bool motorSaturated = false;
    } debug;

    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_PIDController {
public:
    void init(const SWC_PIDControllerInput& initial);
    SWC_PIDControllerOutput update(const SWC_PIDControllerInput& input);
    void reset();

private:
    struct AxisPid {
        float kp = 0.0f;
        float ki = 0.0f;
        float kd = 0.0f;
        float integral = 0.0f;
        float prevError = 0.0f;
        float iLimit = 50.0f;
        float prevMeasurement = 0.0f;
        float filteredMeasurementRate = 0.0f;
        bool hasPrevMeasurement = false;
        bool hasFilteredMeasurementRate = false;
        float lastP = 0.0f;
        float lastI = 0.0f;
        float lastD = 0.0f;
        float lastOut = 0.0f;

        float update(float error, float dt);
        float updateDOnMeasurement(float error, float measurement, float dt, float derivativeLpfHz);
        void reset();
    };

    struct LowPass {
        float y = 0.0f;
        bool initialized = false;

        float apply(float x, float dt, float cutoffHz);
        void reset();
    };

    AxisPid _rateRoll;
    AxisPid _ratePitch;
    AxisPid _rateYaw;
    AxisPid _angleRoll;
    AxisPid _anglePitch;
    AxisPid _angleYaw;
    LowPass _rollSetpoint;
    LowPass _pitchSetpoint;
    LowPass _yawSetpointFilter;
    float _yawSetpointDeg = 0.0f;
    float _smoothedThrottle = 0.0f;
    bool _yawHoldActive = false;

    void applyConfig(const flight::FlightConfig& config);
};
