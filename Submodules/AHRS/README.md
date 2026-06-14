# AHRS Estimator Libraries for Test_Quad

AHRS = **Attitude and Heading Reference System**.

These libraries are designed to let you A/B test estimators in your flight controller without rewriting the rest of the control loop.

## Files

| File | Purpose |
|---|---|
| `AHRSCommon.h` | Common input/output structs and math helpers |
| `MahonyAHRS.h/.cpp` | Standalone Mahony AHRS |
| `MadgwickAHRS.h/.cpp` | Standalone Madgwick 6-DOF AHRS |
| `RollPitchEKF.h/.cpp` | Small roll/pitch EKF-style estimator |

## Important honesty note

`RollPitchEKF` is **not** a full drone navigation EKF. It estimates:

```text
roll
pitch
gyro-x bias
gyro-y bias
```

It is useful for bench testing and Angle-mode comparison. A full EKF would estimate quaternion, gyro bias, accel bias, velocity, position, magnetometer states, etc.

## Units

All estimators expect:

```text
accel: g
gyro:  deg/s
mag:   uT, optional
dt:    seconds
output angles: degrees
```

## Example integration

Add includes:

```cpp
#include "AHRSCommon.h"
#include "MahonyAHRS.h"
#include "MadgwickAHRS.h"
#include "RollPitchEKF.h"
```

Create estimator objects:

```cpp
MahonyAHRS   estMahony;
MadgwickAHRS estMadgwick;
RollPitchEKF estEKF;
```

During setup:

```cpp
estMahony.setGains(1.0f, 0.005f);
estMadgwick.setBeta(0.08f);
estEKF.setProcessNoise(0.0005f, 0.00001f);
estEKF.setMeasurementNoise(0.08f);
```

In your control loop after `imu.readScaled(s)`:

```cpp
AHRSInput in;
in.ax_g = s.ax_g;
in.ay_g = s.ay_g;
in.az_g = s.az_g;
in.gx_dps = s.gx_dps;
in.gy_dps = s.gy_dps;
in.gz_dps = s.gz_dps;
in.mx_uT = s.mx_uT;
in.my_uT = s.my_uT;
in.mz_uT = s.mz_uT;
in.magValid = imu.hasMag();

AttitudeEstimate att;
estMadgwick.update(in, dt, att);

// Then use:
float roll  = att.roll_deg;
float pitch = att.pitch_deg;
float yaw   = att.yaw_deg;
```

## Recommended test order

1. Log current Mahony vs accel-only roll/pitch.
2. Test standalone Mahony with higher/lower Kp.
3. Test Madgwick with beta values:
   - `0.03`
   - `0.05`
   - `0.08`
   - `0.12`
4. Test `RollPitchEKF` only for roll/pitch Angle-mode comparison.

Do not fly immediately after switching estimators. First perform props-off tilt tests.
