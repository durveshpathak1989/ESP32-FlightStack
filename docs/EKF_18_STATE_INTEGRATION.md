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

The estimated body-rate feedback path is wired but intentionally disabled by default. The established notch-filtered gyro path remains active until estimated-rate latency, signs, covariance, and timing are validated from bench logs.

## Gyro-rate comparison logging

The existing 100 Hz flight-log CSV now records four rate signals on roll, pitch, and yaw:

| Signal | CSV columns | Meaning |
| --- | --- | --- |
| Pre-filter gyro | `gxRaw`, `gyRaw`, `gzRaw` | Calibrated IMU gyro before the motor notch and 50 Hz software LPF |
| Active PID feedback | `feedbackRollRateDps`, `feedbackPitchRateDps`, `feedbackYawRateDps` | Reconstructed exactly from `targetRate - rateError`; with EKF-rate feedback disabled, this is the notch + LPF gyro signal |
| Bias-corrected feedback | `biasCorrectedRollRateDps`, `biasCorrectedPitchRateDps`, `biasCorrectedYawRateDps` | Active feedback minus the corresponding EKF gyro-bias estimate |
| EKF rate state | `ekfRollRateDps`, `ekfPitchRateDps`, `ekfYawRateDps` | The posterior 18-state EKF body-rate estimate captured after the same control-cycle EKF update |

`ekfRateValid` is `1` only when the selected AHRS mode is EKF and the EKF update produced a valid rate snapshot. `ahrsMode` remains in the row so post-processing can reject stale data from Mahony or Madgwick operation.

The flight log is currently sampled at 100 Hz even though control runs at 400 Hz. This is sufficient for initial shape, noise, bias, and gross-delay comparison. A dedicated short 400 Hz capture should be used before making a final phase-lag decision for the inner rate loop.

Recommended comparison sequence:

1. Props removed, armed at minimum throttle: leave the frame still for several seconds to compare zero-rate noise and bias.
2. Manually rotate one axis at a time with smooth positive and negative motion.
3. Run motors without props at several throttle levels to compare vibration rejection.
4. Plot all four signals against `t_us`; do not enable EKF rate feedback until signs, amplitude, and delay are verified on every axis.

The additional comparison fields add approximately 40 bytes to each logger row. With the current 120-row allocation, the extra buffer cost is approximately 4.8 KB.

## ToF vertical aiding

The EKF supports:

```cpp
attitudeEKF.updateTofMeasurement(tofDistanceM, tofSampleTimestampMs);
```

A fresh downward-facing ToF sample:

1. is projected to vertical height using EKF roll and pitch;
2. corrects the `PZ` state;
3. is differentiated against the previous fresh ToF sample;
4. corrects `VZ` when the derived velocity passes timing and outlier checks.

The timestamp must be the ToF sensor timestamp (`tofLastUpdate_ms`), not the current 400 Hz control-loop timestamp. Duplicate timestamps are ignored so the same 40 Hz sample is not fused repeatedly.

## Control-task wiring

ToF is fused from `taskControl`, immediately after the EKF attitude update. The EKF is not mutated from `taskToF`, because the EKF object is owned by the 400 Hz control task and is not internally locked.

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

If the estimator is reset, calibration begins, or AHRS mode changes away from EKF, the ToF fusion timestamp/history should also be reset.

## Sign convention

World Z is positive upward. A larger ToF altitude therefore indicates ascent and produces positive `VZ`.

## Limitations and gating

ToF vertical velocity is relative to the surface below the vehicle. Reject or down-weight measurements over steps, furniture, vegetation, highly reflective surfaces, excessive tilt, invalid range status, stale samples, or operation outside the sensor range.

Horizontal position and velocity still require GPS or optical-flow aiding. IMU integration alone will drift.

## Build output

The integration pull request runs the ESP32 release build with the project’s established 4 MB `min_spiffs` board configuration and `VERBOSE_ON=0`. The generated binaries, ELF/map files, build identity, and SHA-256 checksums are published under:

```text
firmware/agent-18-state-ekf-integration/
```

## Validation order

1. Compile and inspect 400 Hz execution time and stack headroom.
2. Run a stationary test and verify rates, biases, and world acceleration converge without NaNs.
3. Raise and lower the disarmed vehicle by hand; verify ToF `PZ` and `VZ` signs.
4. Tilt the vehicle while holding constant vertical height; verify tilt compensation prevents a false altitude change.
5. Run props-off vibration testing.
6. Run restrained low-throttle testing.
7. Only then consider hover testing or enabling estimated-rate feedback.
