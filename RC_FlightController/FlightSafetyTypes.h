#pragma once

#include <Arduino.h>

struct ToFData {
    bool valid = false;
    float distance_m = 0.0f;
    float verticalVelocity_mps = 0.0f; // positive means descending toward the ground
    uint32_t lastUpdateMs = 0;
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
