# Experimental 18-State EKF Integration

Flight-stack branch: `agent/18-state-ekf-integration`

EKF submodule branch: `agent/18-state-inertial-ekf`

Pinned EKF commit: `4e8bccfb7ea34a122b76bdede5c830f57d0b8585`

## State vector

```text
[px, py, pz,
 vx, vy, vz,
 ax_world, ay_world, az_world,
 roll, pitch, yaw,
 roll_rate, pitch_rate, yaw_rate,
 gyro_bias_x, gyro_bias_y, gyro_bias_z]
```

## Current control-loop behavior

The submodule remains API-compatible with the existing sketch. `attitudeEKF.update()` continues to provide roll, pitch, yaw, quaternion, and gyro-bias accessors.

Do not immediately replace the proven ACRO gyro feedback with the new estimated body-rate states. First validate estimated-rate latency, signs, covariance, and bias convergence from logs.

## ToF vertical aiding

The EKF now supports:

```cpp
attitudeEKF.updateTofMeasurement(tofDistanceM, tofSampleTimestampMs);
```

A fresh downward-facing ToF sample:

1. is projected to vertical height using EKF roll and pitch;
2. corrects the `PZ` state;
3. is differentiated against the previous fresh ToF sample;
4. corrects `VZ` when the derived velocity passes timing and outlier checks.

The timestamp must be the ToF sensor sample timestamp (`tofLastUpdate_ms`), not the current 400 Hz loop time. Duplicate timestamps are ignored so the same 40 Hz sample is not fused repeatedly.

## Recommended control-task wiring

Fuse ToF from `taskControl`, immediately after the EKF attitude update. Do not call the EKF from `taskToF`, because the EKF object is also mutated by the 400 Hz control task and is not internally locked.

Conceptual integration:

```cpp
static uint32_t lastFusedTofTimestampMs = 0;

float tofDistanceM = 0.0f;
uint32_t tofTimestampMs = 0;
bool tofValid = false;

if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE) {
    tofDistanceM = g_state.tofDistance_m;
    tofTimestampMs = g_state.tofLastUpdate_ms;
    tofValid = g_state.tofValid;
    xSemaphoreGive(g_flightMutex);
}

if (ahrsMode == 0 &&
    tofValid &&
    tofTimestampMs != 0 &&
    tofTimestampMs != lastFusedTofTimestampMs) {
    attitudeEKF.updateTofMeasurement(tofDistanceM, tofTimestampMs);
    lastFusedTofTimestampMs = tofTimestampMs;
}
```

This call should occur only when EKF mode is active. If the estimator is reset, calibration begins, or AHRS mode changes away from EKF, reset `lastFusedTofTimestampMs` and call `attitudeEKF.resetPositionVelocity()` as appropriate.

## Sign convention

World Z is positive upward. A larger ToF altitude therefore indicates ascent and produces positive `VZ`.

## Limitations and gating

ToF vertical velocity is relative to the surface below the vehicle. Reject or down-weight measurements over steps, furniture, vegetation, highly reflective surfaces, excessive tilt, invalid range status, stale samples, or operation outside the sensor range.

Horizontal position and velocity still require GPS or optical-flow aiding. IMU integration alone will drift.

## Validation order

1. Compile and inspect 400 Hz execution time and stack headroom.
2. Run a stationary test and verify rates, biases, and world acceleration converge without NaNs.
3. Raise and lower the disarmed vehicle by hand; verify ToF `PZ` and `VZ` signs.
4. Tilt the vehicle while holding constant vertical height; verify tilt compensation prevents a false altitude change.
5. Run props-off vibration testing.
6. Run restrained low-throttle testing.
7. Only then consider hover testing or enabling estimated-rate feedback.
