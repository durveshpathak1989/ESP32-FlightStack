# LoggingMechanism integration plan for V5.0.0

Baseline source: `RC_FlightController/RC_FlightController.ino` on `master` v5.0.0.

This branch adds a reusable diagnostic logging library:

```text
RC_FlightController/src/Submodules/LoggingMechanism/
  LoggingMechanism.h
  LoggingMechanism.cpp
```

and a GCS page:

```text
GCS/DroneGCS_LoggingDiagnostics.html
```

## Why this exists

The current V5.0.0 log already captures many important fields: target angles/rates, EKF attitude, control attitude after zeroing, filtered/raw IMU, mag norm, EKF bias, rate errors, motor outputs, RPM estimates, notch settings, CPU, and timing. The important missing diagnostic layer is the **PID term breakdown** and **motor pre-clamp values**.

Those are needed to distinguish:

1. EKF/control-angle dynamic bias under throttle.
2. Angle-loop integrator buildup.
3. Rate-loop lag/damping.
4. Motor/thrust asymmetry or saturation.

## Integration steps

### 1. Include the new library in `RC_FlightController.ino`

Add near the other submodule includes:

```cpp
#include "src/Submodules/LoggingMechanism/LoggingMechanism.h"
```

### 2. Replace the local `FlightLogRow` ring with `LoggingMechanism`

Current V5.0.0 has an internal `FlightLogRow g_log[FLIGHT_LOG_SIZE]` ring and provider functions for `/flightlog/csv`. Replace that local ring with:

```cpp
static LoggingMechanism diagLog(600);   // 100 Hz x 6 s
```

In `setup()` after mutex initialization:

```cpp
diagLog.begin();
```

Then point the existing WiFi flight-log providers to wrappers:

```cpp
static uint16_t flightLogCount() { return diagLog.countAndFreeze(); }
static String flightLogHeader()  { return diagLog.csvHeader(); }
static String flightLogRowCsv(uint16_t i) { return diagLog.csvRow(i); }
static void resetFlightLog() { diagLog.reset(); }
```

The existing `TelemetryWiFi` class already has flight-log callbacks and streams `/flightlog/csv` chunked, so the transport mechanism does not need to change.

### 3. Modify `PID` to expose P/I/D terms

Current V5.0.0 PID only returns final output. Change the PID struct from:

```cpp
float update(float err,float dt){
    integral=constrain(integral+err*dt,-iLimit,iLimit);
    float d2=(err-prevError)/dt; prevError=err;
    return kp*err+ki*integral+kd*d2;
}
```

to:

```cpp
float lastP = 0.0f;
float lastI = 0.0f;
float lastD = 0.0f;
float lastOut = 0.0f;

float update(float err,float dt){
    integral=constrain(integral+err*dt,-iLimit,iLimit);
    float d2=(dt > 0.000001f) ? ((err-prevError)/dt) : 0.0f;
    prevError=err;
    lastP = kp * err;
    lastI = ki * integral;
    lastD = kd * d2;
    lastOut = lastP + lastI + lastD;
    return lastOut;
}

void reset(){
    integral=0;
    prevError=0;
    lastP=lastI=lastD=lastOut=0;
}
```

### 4. Capture pre-clamp motor values

Immediately after mixer calculation and before desaturation/clamp:

```cpp
float flPre = thr + rO - pO - yO;
float frPre = thr - rO - pO + yO;
float rlPre = thr + rO + pO + yO;
float rrPre = thr - rO + pO - yO;

float fl = flPre;
float fr = frPre;
float rl = rlPre;
float rr = rrPre;
```

Then perform the existing desaturation and clamp on `fl/fr/rl/rr`.

### 5. Push a `DiagnosticLogRow` at 100 Hz

Replace the old `FlightLogRow row` block with:

```cpp
DiagnosticLogRow row;
memset(&row, 0, sizeof(row));

row.t_us = nowUs;
row.loop_count = g_state.loopCount;
row.period_us = clampU16(periodUs);
row.jitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
row.control_exec_us = clampU16(controlDoneUs - execStartUs);
row.imu_read_us = clampU16(g_execTiming.lastImuUs);
row.rc_read_us = clampU16(g_execTiming.lastRcUs);
row.motor_exec_us = clampU16(g_execTiming.lastMotorUs);
row.mode = (uint8_t)cmd.mode;
row.flags = (cmd.mode != FlightMode::DISARMED ? 0x0001 : 0)
          | (imuOk ? 0x0002 : 0)
          | (cmd.valid ? 0x0004 : 0)
          | ((imuOk && imu.isMagConnected()) ? 0x0008 : 0)
          | (motorSaturated ? 0x0010 : 0)
          | (g_state.rpmActualValid ? 0x0020 : 0);

row.throttle = cmd.throttle;
row.rc_roll = rollCmd;
row.rc_pitch = pitchCmd;
row.rc_yaw = yawCmd;
row.target_roll_deg = targetRollDeg;
row.target_pitch_deg = targetPitchDeg;
row.target_yaw_deg = targetYawDeg;
row.target_roll_rate_dps = targetRollRateDps;
row.target_pitch_rate_dps = targetPitchRateDps;
row.target_yaw_rate_dps = targetYawRateDps;

row.ekf_roll_deg = imuOk ? att.roll_deg : 0.0f;
row.ekf_pitch_deg = imuOk ? att.pitch_deg : 0.0f;
row.ekf_yaw_deg = imuOk ? att.yaw_deg : 0.0f;
row.ctrl_roll_deg = roll;
row.ctrl_pitch_deg = pitch;
row.ctrl_yaw_deg = imuOk ? yawCtrlDeg : 0.0f;
row.zero_roll_deg = g_levelRollOffsetDeg;
row.zero_pitch_deg = g_levelPitchOffsetDeg;
row.zero_yaw_deg = g_levelYawOffsetDeg;

row.ax_g = imuOk ? s.ax_g : 0.0f;
row.ay_g = imuOk ? s.ay_g : 0.0f;
row.az_g = imuOk ? s.az_g : 0.0f;
row.gx_dps = imuOk ? s.gx_dps : 0.0f;
row.gy_dps = imuOk ? s.gy_dps : 0.0f;
row.gz_dps = imuOk ? s.gz_dps : 0.0f;
row.mx_uT = imuOk ? s.mx_uT : 0.0f;
row.my_uT = imuOk ? s.my_uT : 0.0f;
row.mz_uT = imuOk ? s.mz_uT : 0.0f;
row.accel_norm_g = sqrtf(row.ax_g*row.ax_g + row.ay_g*row.ay_g + row.az_g*row.az_g);
row.gyro_norm_dps = sqrtf(row.gx_dps*row.gx_dps + row.gy_dps*row.gy_dps + row.gz_dps*row.gz_dps);
row.mag_norm_uT = sqrtf(row.mx_uT*row.mx_uT + row.my_uT*row.my_uT + row.mz_uT*row.mz_uT);
row.ekf_bgx_dps = attitudeEKF.rollBiasDps();
row.ekf_bgy_dps = attitudeEKF.pitchBiasDps();
row.ekf_bgz_dps = attitudeEKF.yawBiasDps();

row.angle_roll_error_deg = angleErrRollDeg;
row.angle_pitch_error_deg = angleErrPitchDeg;
row.yaw_error_deg = yawErrDeg;
row.rate_roll_error_dps = rateErrRollDps;
row.rate_pitch_error_dps = rateErrPitchDps;
row.rate_yaw_error_dps = rateErrYawDps;

row.angle_roll_p = pidAngleRoll.lastP;
row.angle_roll_i = pidAngleRoll.lastI;
row.angle_roll_d = pidAngleRoll.lastD;
row.angle_roll_out = targetRollRateDps;
row.angle_pitch_p = pidAnglePitch.lastP;
row.angle_pitch_i = pidAnglePitch.lastI;
row.angle_pitch_d = pidAnglePitch.lastD;
row.angle_pitch_out = targetPitchRateDps;
row.angle_yaw_p = pidAngleYaw.lastP;
row.angle_yaw_i = pidAngleYaw.lastI;
row.angle_yaw_d = pidAngleYaw.lastD;
row.angle_yaw_out = targetYawRateDps;

row.rate_roll_p = pidRateRoll.lastP;
row.rate_roll_i = pidRateRoll.lastI;
row.rate_roll_d = pidRateRoll.lastD;
row.rate_roll_out = rO;
row.rate_pitch_p = pidRatePitch.lastP;
row.rate_pitch_i = pidRatePitch.lastI;
row.rate_pitch_d = pidRatePitch.lastD;
row.rate_pitch_out = pO;
row.rate_yaw_p = pidRateYaw.lastP;
row.rate_yaw_i = pidRateYaw.lastI;
row.rate_yaw_d = pidRateYaw.lastD;
row.rate_yaw_out = yO;

row.motor_fl_pre = flPre;
row.motor_fr_pre = frPre;
row.motor_rl_pre = rlPre;
row.motor_rr_pre = rrPre;
row.motor_fl = fl;
row.motor_fr = fr;
row.motor_rl = rl;
row.motor_rr = rr;
row.motor_diag_a = fl + rr;
row.motor_diag_b = fr + rl;
row.motor_diag_diff = row.motor_diag_a - row.motor_diag_b;

row.rpm_fl = cmdRpmFL;
row.rpm_fr = cmdRpmFR;
row.rpm_rl = cmdRpmRL;
row.rpm_rr = cmdRpmRR;
row.rpm_diag_a = cmdRpmFL + cmdRpmRR;
row.rpm_diag_b = cmdRpmFR + cmdRpmRL;
row.rpm_diag_diff = row.rpm_diag_a - row.rpm_diag_b;

row.cpu0_pct = g_state.cpuCore0_pct;
row.cpu1_pct = g_state.cpuCore1_pct;
row.notch_freq_hz = tune.notch_freq_hz;
row.notch_q = tune.notch_q;

diagLog.push(row);
```

### 6. GCS usage

Open this local file in a browser while connected to the ESP32 AP:

```text
GCS/DroneGCS_LoggingDiagnostics.html
```

Default ESP32 URL:

```text
http://192.168.4.1
```

The page can:

- poll `/telemetry`
- reset `/flightlog/reset`
- download `/flightlog/csv`
- plot roll/pitch target vs EKF/control angle
- plot accel/gyro norm
- plot motor diagonal command/RPM imbalance

## Notes

The page gracefully handles missing PID-term fields by displaying `--`. After the PID patch is applied, the CSV will contain the full P/I/D breakdown required for diagnosing angle-mode drift and lag.
