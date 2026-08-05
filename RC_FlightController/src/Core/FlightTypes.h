#pragma once

#include <cstdint>

namespace flight {

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ImuSample {
    Vec3f accelerationG;
    Vec3f angularRateDps;
    Vec3f magneticFieldUt;
    float temperatureC = 0.0f;
    std::uint64_t timestampUs = 0;
    bool accelerationValid = false;
    bool angularRateValid = false;
    bool magneticFieldValid = false;
};

struct PilotCommand {
    float throttle = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    bool armed = false;
    bool angleMode = true;
    bool valid = false;
};

struct Attitude {
    float rollDeg = 0.0f;
    float pitchDeg = 0.0f;
    float yawDeg = 0.0f;
    float q0 = 1.0f;
    float q1 = 0.0f;
    float q2 = 0.0f;
    float q3 = 0.0f;
    bool valid = false;
};

struct MotorCommand {
    float frontLeft = 0.0f;
    float frontRight = 0.0f;
    float rearLeft = 0.0f;
    float rearRight = 0.0f;
};

}  // namespace flight
