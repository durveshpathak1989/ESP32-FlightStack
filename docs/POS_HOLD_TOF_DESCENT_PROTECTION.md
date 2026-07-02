# POS_HOLD Shell and ToF Descent Protection

This branch adds two safety-oriented features:

- `FlightMode::POS_HOLD`, requested by SWC / CH9 while armed.
- ToF-based descent protection, active in ANGLE, ACRO, and POS_HOLD when the craft is armed and the ToF range is valid.
- ANGLE-mode vertical hold using a barometer + ToF height EKF and a conservative vertical PID throttle correction.

## POS_HOLD Status

Current status: `POS_HOLD_NO_XY_SENSOR`.

The mode is intentionally a shell, not true position hold. With only IMU plus downward ToF, the controller cannot hold X/Y position safely. True horizontal hold needs optical flow, GPS velocity, UWB, vision, or another horizontal position/velocity source.

Today POS_HOLD behaves like a gentle ANGLE mode:

- SWC high while armed enters `POSHOLD`.
- SWC high while disarmed does not enter POS_HOLD and remains available for calibration confirmation.
- Centered roll/pitch sticks command level attitude.
- Roll/pitch stick movement still gives pilot override.
- Telemetry reports `posHoldStatus=POS_HOLD_NO_XY_SENSOR` and `xyHoldAvailable=false`.

## Future Horizontal Hold Hooks

The firmware now has placeholders for future XY sensing:

- `HorizontalNavEstimate`
- `PositionHoldState`

Do not implement accelerometer-only XY integration as a real hold source. It drifts too quickly and is unsafe for position hold.

## ToF Descent Protection

The existing VL53L4CX driver remains the preferred ToF driver. `taskToF` runs on Core 0 at 40 Hz and publishes a filtered range snapshot for the 400 Hz control loop.

Sign convention:

- `tofVerticalVelocityMps > 0` means descending toward the ground.
- It is derived from filtered range: decreasing range produces positive vertical velocity.

Protection behavior:

- Disabled when disarmed or failsafe.
- Disabled when ToF is invalid, stale, out of range, or jump-rejected.
- Active only when range is greater than the tunable minimum active distance and below `DESCENT_PROTECT_START_M`.
- The default minimum active distance is `DESCENT_PROTECT_MIN_ACTIVE_M = 0.50 m`.
- Adds limited throttle boost only when descent rate exceeds `MAX_SAFE_DESCENT_MPS`.
- Enforces a small landing throttle floor near the ground.
- Caps all added authority with `DESCENT_ASSIST_MAX_BOOST` and motor/throttle limits.

Runtime tuning:

- Firmware key: `descent_protect_min_active_m`.
- GCS/telemetry key: `descentProtectMinActiveM`.
- GCS control: `DP Min m`.
- Accepted range is constrained between `TOF_MIN_VALID_M` and just below `DESCENT_PROTECT_START_M`.
- With the default `0.50 m`, descent protection is inactive at or below 0.50 m, so landing close to the ground is not artificially boosted by the guard.

Telemetry fields:

- `tofDistanceM`
- `tofVelocityMps`
- `tofValid`
- `tofStale`
- `tofJumpRejected`
- `rawThrottle`
- `protectedThrottle`
- `descentProtectActive`
- `descentThrottleBoost`
- `descentProtectMinActiveM`
- `posHoldRequested`
- `posHoldModeActive`
- `xyHoldAvailable`

## ANGLE Vertical Hold

ANGLE mode now has a vertical hold assist. ACRO remains manual and POS_HOLD remains the current no-XY-sensor shell.

Height estimator:

- The attitude EKF owns a separate 2-state vertical EKF: height and vertical velocity.
- BMP280 altitude is converted to a relative local height using a startup reference.
- VL53L4CX ToF range is tilt-compensated with roll/pitch before fusion.
- Each fresh BMP/ToF sensor sample corrects the EKF once; between samples the EKF predicts from vertical acceleration.

Controller behavior:

- Active only in ANGLE mode.
- Requires armed flight, valid EKF height, throttle above cut, height above `vertical_hold_min_active_m`, and tilt below `vertical_hold_max_tilt_deg`.
- Captures the current EKF height when the hold first becomes available.
- Uses throttle around `vertical_hold_center_throttle` as a climb/descent-rate command.
- Adds a capped PID throttle correction with `vertical_hold_output_limit`.
- Resets its integrator when disarmed, failsafe, calibration-blocked, invalid height, too low, or too tilted.

Runtime tuning keys:

- `vertical_hold_enable`
- `pid_vertical_kp`, `pid_vertical_ki`, `pid_vertical_kd`
- `vertical_hold_output_limit`
- `vertical_hold_center_throttle`
- `vertical_hold_deadband`
- `vertical_hold_max_climb_rate_mps`
- `vertical_hold_min_active_m`
- `vertical_hold_max_tilt_deg`
- `vertical_hold_d_lpf_hz`
- `ekf_alt_accel_q`, `ekf_baro_alt_r`, `ekf_tof_alt_r`

## Compile Checklist

1. Pull latest `master`.
2. Check out `feature/tof-descent-poshold`.
3. Update submodules.
4. Compile release with `VERBOSE_ON=0`.
5. Compile debug with `VERBOSE_ON=1`.
6. Confirm no ToF read occurs in the 400 Hz control loop.
7. Confirm `taskControl` still fits inside the 2500 us timing budget.

## Bench-Test Checklist

1. Disarmed:
   - SWC high must not enter POS_HOLD.
   - SWC must still confirm accel calibration steps.

2. Armed mode mapping:
   - SWC low + SWB low = ANGLE.
   - SWC low + SWB high = ACRO.
   - SWC high = POSHOLD.
   - POSHOLD self-levels like ANGLE and does not claim XY hold.

3. ToF disconnected:
   - Boot continues.
   - `tofValid=false`.
   - `descentProtectActive=false`.
   - Normal throttle behavior is unchanged.

4. ToF valid on bench:
   - Move the sensor toward the floor and confirm distance decreases smoothly.
   - Confirm `tofVelocityMps` is positive when moving toward the floor.
   - Confirm stale detection after blocking/unplugging the sensor.
   - Confirm jump rejection on unrealistic sudden range changes.

5. Descent protection:
   - Near ground with positive-down velocity above the limit, throttle boost increases but remains capped.
   - At or below the configured `DP Min m`, `descentProtectActive=false`.
   - Far above the start distance, boost stays zero.
   - Disarm/failsafe always shuts motors down regardless of ToF.

6. ANGLE vertical hold:
   - ACRO must not report `verticalHoldActive=true`.
   - ANGLE with invalid BMP/ToF height must report `verticalHoldHeightValid=false` and keep pilot throttle behavior.
   - ANGLE above `vertical_hold_min_active_m` and below tilt limit should capture the current height as target.
   - Moving throttle above/below center should slew the target height instead of jumping throttle.
   - Above `vertical_hold_max_tilt_deg`, `verticalHoldTiltLimited=true` and the vertical PID must reset.

7. Timing:
   - No heavy serial spam in the control path.
   - `/timing` remains stable with ToF connected.
