#pragma once

#include <Arduino.h>

struct ToFData {
    bool valid = false;
    float distance_m = 0.0f;
    float verticalVelocity_mps = 0.0f; // positive means descending toward the ground
    uint32_t lastUpdateMs = 0;
};

struct AltitudeSensorData {
    ToFData tof;
    bool bmpValid = false;
    float bmpAltitude_m = 0.0f;
    float bmpVerticalSpeed_mps = 0.0f;
    uint32_t bmpLastUpdateMs = 0;
};

struct VerticalHoldResult {
    float throttle = 0.0f;
    float targetHeight_m = 0.0f;
    float estimatedHeight_m = 0.0f;
    float estimatedVelocity_mps = 0.0f; // positive up
    float error_m = 0.0f;
    float throttleAdjust = 0.0f;
    float pilotRate_mps = 0.0f;
    bool available = false;
    bool active = false;
    bool heightValid = false;
    bool tiltLimited = false;
};

struct VerticalHoldConfig {
    bool enable = false;
    float throttleCut = 0.03f;
    float outputLimit = 0.0f;
    float centerThrottle = 0.50f;
    float deadband = 0.08f;
    float maxClimbRate_mps = 0.50f;
    float minActive_m = 0.50f;
    float maxTilt_deg = 25.0f;
    float dLpf_hz = 5.0f;
};

struct HorizontalNavEstimate {
    bool valid = false;
    float vx_mps = 0.0f;
    float vy_mps = 0.0f;
    float x_m = 0.0f;
    float y_m = 0.0f;
    uint32_t lastUpdateMs = 0;
};

struct PositionHoldState {
    bool requested = false;
    bool active = false;
    bool xySensorValid = false;
    float targetX_m = 0.0f;
    float targetY_m = 0.0f;
    float targetVx_mps = 0.0f;
    float targetVy_mps = 0.0f;
};

struct DescentProtectionResult {
    float throttle = 0.0f;
    float boost = 0.0f;
    bool active = false;
    bool tofStale = true;
};
