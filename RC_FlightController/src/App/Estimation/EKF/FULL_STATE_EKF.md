# Experimental 18-State Inertial EKF

Branch: `agent/18-state-inertial-ekf`

## State vector

The filter state is:

```text
x = [
  px, py, pz,
  vx, vy, vz,
  ax_world, ay_world, az_world,
  roll, pitch, yaw,
  roll_rate, pitch_rate, yaw_rate,
  gyro_bias_x, gyro_bias_y, gyro_bias_z
]
```

Units are metres, metres/second, metres/second squared, radians, radians/second, and radians/second respectively.

## Prediction model

- Position is propagated from velocity and world-frame acceleration.
- Velocity is propagated from world-frame acceleration.
- Attitude is propagated from estimated body rates.
- World acceleration, body rate, and gyro bias use random-walk process models.

The current experimental branch uses a small-angle Euler-rate approximation. Before aggressive-angle flight testing, replace it with the full body-rate-to-Euler-rate Jacobian or a quaternion error-state formulation.

## Measurement updates

- Gyroscope: `gyro_measured = body_rate + gyro_bias + noise`.
- Accelerometer tilt: gravity-derived roll and pitch are used with adaptive measurement noise.
- Accelerometer specific force: rotated into the world frame and used to correct world-acceleration states.
- Magnetometer: tilt-compensated yaw correction.
- Optional aiding hooks: position, velocity, and altitude measurements for GPS, optical flow, barometer, and ToF.

## ToF altitude and vertical velocity

A fresh downward-facing ToF range sample is tilt-compensated using the estimated roll and pitch:

```text
vertical_height = range * cos(roll) * cos(pitch)
```

The corrected height updates `pz`. Two consecutive fresh ToF samples provide a vertical-speed measurement:

```text
vz_tof = (height_k - height_k-1) / (time_k - time_k-1)
```

This measurement corrects `vz`. The implementation:

- ignores duplicate timestamps so one physical sample is not fused repeatedly;
- accepts sample intervals from 15 ms to 250 ms;
- rejects vertical-speed jumps above 5 m/s;
- low-pass filters the differentiated ToF velocity;
- uses positive-up sign convention, so increasing ToF altitude means positive `vz`;
- rejects ToF aiding at extreme tilt where the vertical projection is unreliable.

The caller must pass the ToF sensor sample timestamp, not the current 400 Hz control-loop timestamp.

## Important limitations

Position and horizontal velocity remain weakly observable with IMU-only operation and will drift. Reliable horizontal position hold still requires optical flow or GPS aiding.

ToF vertical velocity is relative to the local ground surface. It can become invalid over steps, furniture, vegetation, reflective surfaces, excessive tilt, or when the surface leaves the sensor range. Barometric and inertial consistency checks should eventually gate ToF innovations.

## Flight-test safety

This branch is experimental. Preserve the known ACRO/rate-loop gains, validate timing and covariance behaviour first, then perform static IMU, props-off, and restrained low-throttle tests before any hover. Do not substitute estimated body rates into the flight-control feedback loop until rate latency, bias convergence, and sign conventions are verified from logs.
