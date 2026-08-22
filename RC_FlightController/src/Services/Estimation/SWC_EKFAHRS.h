#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_EKFAHRSInput {
    struct RawImu {
        float ax = 0.0f;  // m/s^2, body X.
        float ay = 0.0f;  // m/s^2, body Y.
        float az = 9.80665f;  // m/s^2, body Z.
        float gx = 0.0f;  // rad/s.
        float gy = 0.0f;  // rad/s.
        float gz = 0.0f;  // rad/s.
        float mx = 0.0f;  // normalized or uT, optional.
        float my = 0.0f;
        float mz = 0.0f;
        bool magValid = false;
    } imu;

    float baroAltitude = 0.0f;
    struct GpsOptional {
        bool valid = false;
        double lat = 0.0;
        double lon = 0.0;
        float alt = 0.0f;
        float velNorth = 0.0f;
        float velEast = 0.0f;
        float velDown = 0.0f;
    } gps;

    std::uint64_t dtUs = 2500;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_EKFAHRSOutput {
    flight::AttitudeState attitude;
    struct Debug {
        float gyroBiasX = 0.0f;
        float gyroBiasY = 0.0f;
        float gyroBiasZ = 0.0f;
        float accelBiasX = 0.0f;
        float accelBiasY = 0.0f;
        float accelBiasZ = 0.0f;
        float accelNormMps2 = 0.0f;
        bool accelAccepted = false;
        bool magAccepted = false;
        bool healthy = true;
    } debug;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_EKFAHRS {
public:
    void init(const SWC_EKFAHRSInput& initial);
    SWC_EKFAHRSOutput update(const SWC_EKFAHRSInput& input);
    void reset();

private:
    float _roll = 0.0f;
    float _pitch = 0.0f;
    float _yaw = 0.0f;
    float _gyroBiasX = 0.0f;
    float _gyroBiasY = 0.0f;
    float _gyroBiasZ = 0.0f;
    float _accelBiasX = 0.0f;
    float _accelBiasY = 0.0f;
    float _accelBiasZ = 0.0f;
    bool _initialized = false;
};
