#pragma once

#include <cstdint>

namespace flight {

struct FlightConstants {
    static constexpr float kPi = 3.14159265358979323846f;
    static constexpr float kDegToRad = kPi / 180.0f;
    static constexpr float kRadToDeg = 180.0f / kPi;

    static constexpr float kDefaultControlPeriodUs = 2500.0f;
    static constexpr float kDefaultControlPeriodS = 0.0025f;
    static constexpr float kDefaultMaxAngleDeg = 5.0f;
    static constexpr float kDefaultMaxRateDps = 200.0f;
    static constexpr float kDefaultYawMaxRateDps = 20.0f;
    static constexpr float kDefaultMotorIdle = 0.08f;
    static constexpr float kDefaultMotorMax = 1.0f;
    static constexpr float kDefaultThrottleCut = 0.03f;
    static constexpr float kDefaultIdleRampEnd = 0.15f;
};

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
    std::uint8_t version = 1;
};

// Versioned attitude contract used by ArchV5 services.
struct AttitudeState {
    float roll = 0.0f;       // rad, body roll angle, typically [-pi, pi].
    float pitch = 0.0f;      // rad, body pitch angle, typically [-pi/2, pi/2].
    float yaw = 0.0f;        // rad, heading angle, wrapped to [-pi, pi].
    float rollRate = 0.0f;   // rad/s, body-frame roll rate.
    float pitchRate = 0.0f;  // rad/s, body-frame pitch rate.
    float yawRate = 0.0f;    // rad/s, body-frame yaw rate.
    float accelX = 0.0f;     // m/s^2, body-frame acceleration.
    float accelY = 0.0f;     // m/s^2, body-frame acceleration.
    float accelZ = 0.0f;     // m/s^2, body-frame acceleration.
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct PilotCommand {
    float throttle = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    // Legacy fields kept for the existing portable ports.
    bool armed = false;
    bool angleMode = true;
    bool valid = false;

    // ArchV5 contract fields.
    bool armSwitch = false;
    std::uint8_t mode = 0;  // 0 angle, 1 acro, 2 alt-hold, 3 loiter, 4 auto, 5 land.
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
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
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct MotorCommand {
    float frontLeft = 0.0f;
    float frontRight = 0.0f;
    float rearLeft = 0.0f;
    float rearRight = 0.0f;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct BatteryState {
    enum Status : std::uint8_t {
        HEALTHY = 0,
        LOW_VOLTAGE = 1,
        CRITICAL = 2,
        DISCONNECTED = 3
    };

    float voltage = 0.0f;   // V, full pack voltage.
    float current = 0.0f;   // A, 0 when no current sensor is present.
    float capacity = 0.0f;  // mAh consumed or remaining per producer convention.
    std::uint8_t cellCount = 3;
    Status status = HEALTHY;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct GpsState {
    enum Fix : std::uint8_t {
        NO_FIX = 0,
        GPS_2D = 1,
        GPS_3D = 2,
        RTK_FIXED = 3
    };

    double latitude = 0.0;   // decimal degrees.
    double longitude = 0.0;  // decimal degrees.
    float altitude = 0.0f;   // m, WGS84/MSL as supplied by the GPS wrapper.
    float velNorth = 0.0f;   // m/s.
    float velEast = 0.0f;    // m/s.
    float velDown = 0.0f;    // m/s.
    std::uint8_t numSatellites = 0;
    float hdop = 99.9f;
    Fix fix = NO_FIX;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct ReceiverState {
    bool isConnected = false;
    std::uint64_t lastUpdateUs = 0;
    std::uint8_t version = 1;
};

struct FlightConfig {
    // Outer attitude loop gains. Values mirror RC_FlightController/src/Application/FlightConfig.h.
    float pidAttRoll_Kp = 1.50f;
    float pidAttRoll_Ki = 0.0f;
    float pidAttRoll_Kd = 0.0f;
    float pidAttPitch_Kp = 1.00f;
    float pidAttPitch_Ki = 0.0f;
    float pidAttPitch_Kd = 0.0f;
    float pidAttYaw_Kp = 1.00f;
    float pidAttYaw_Ki = 0.0f;
    float pidAttYaw_Kd = 0.0f;

    // Inner rate loop gains.
    float pidRateRoll_Kp = 0.001300f;
    float pidRateRoll_Ki = 0.000700f;
    float pidRateRoll_Kd = 0.000000010f;
    float pidRatePitch_Kp = 0.001300f;
    float pidRatePitch_Ki = 0.000700f;
    float pidRatePitch_Kd = 0.000000010f;
    float pidRateYaw_Kp = 0.008000f;
    float pidRateYaw_Ki = 0.001000f;
    float pidRateYaw_Kd = 0.0f;

    float pidRateRoll_Ff = 0.0f;
    float pidRatePitch_Ff = 0.0f;
    float pidRateYaw_Ff = 0.0f;
    float pidRateRoll_DLpfHz = 100.0f;
    float pidRatePitch_DLpfHz = 100.0f;
    float pidRateYaw_DLpfHz = 100.0f;
    float pidIntegralLimit = 50.0f;

    // Control limits and throttle shaping.
    float maxRoll = FlightConstants::kDefaultMaxAngleDeg * FlightConstants::kDegToRad;
    float maxPitch = FlightConstants::kDefaultMaxAngleDeg * FlightConstants::kDegToRad;
    float maxRateRoll = FlightConstants::kDefaultMaxRateDps * FlightConstants::kDegToRad;
    float maxRatePitch = FlightConstants::kDefaultMaxRateDps * FlightConstants::kDegToRad;
    float maxRateYaw = FlightConstants::kDefaultYawMaxRateDps * FlightConstants::kDegToRad;
    float maxAngleDeg = FlightConstants::kDefaultMaxAngleDeg;
    float maxRateDps = FlightConstants::kDefaultMaxRateDps;
    float maxPitchRateDps = FlightConstants::kDefaultMaxRateDps;
    float yawDeadband = 0.02f;
    float yawMaxRateDps = FlightConstants::kDefaultYawMaxRateDps;
    float rollOutputLimit = 0.120f;
    float pitchOutputLimit = 0.120f;
    float yawOutputLimit = 0.120f;
    float throttleExpo = 0.70f;
    float throttleUpRatePerSec = 0.50f;
    float throttleDownRatePerSec = 0.50f;
    float motorIdle = FlightConstants::kDefaultMotorIdle;
    float motorMax = FlightConstants::kDefaultMotorMax;
    float maxThrottle = FlightConstants::kDefaultMotorMax;
    float throttleCut = FlightConstants::kDefaultThrottleCut;
    float idleRampEnd = FlightConstants::kDefaultIdleRampEnd;

    // Basic failsafe thresholds.
    float batteryLowVoltage = 10.0f;
    float batteryOkVoltage = 11.0f;
    float batteryCriticalVoltage = 9.0f;
    std::uint32_t rcLossTimeoutMs = 2000;
    std::uint32_t gpsLossTimeoutMs = 5000;

    std::uint8_t version = 1;
};

}  // namespace flight

using AttitudeState = flight::AttitudeState;
using BatteryState = flight::BatteryState;
using FlightConfig = flight::FlightConfig;
using GpsState = flight::GpsState;
using MotorCommand = flight::MotorCommand;
using PilotCommand = flight::PilotCommand;
using ReceiverState = flight::ReceiverState;
