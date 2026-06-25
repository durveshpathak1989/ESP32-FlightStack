# Logger integration patch for `RC_FlightController.ino`

Branch: `feature/logger`  
Baseline: `master` firmware v5.0.0

The branch adds the renamed logger library:

```text
RC_FlightController/src/Submodules/Logger/Logger.h
RC_FlightController/src/Submodules/Logger/Logger.cpp
```

and GCS page:

```text
GCS/DroneGCS_LoggerDiagnostics.html
```

## Paper A logging objective

The logger CSV is intended to support Paper A tests for:

1. Real-time determinism: loop period, jitter, phase execution time, CPU load.
2. Sensor quality: accel/gyro/mag norms, filtered vs raw behavior, EKF gyro bias, mag acceptance.
3. EKF/target tracking: target angle, control angle after zeroing, raw EKF angle, angle error.
4. Rate-loop tracking: target rate, gyro rate, rate error.
5. PID explainability: P/I/D terms and final outputs for outer angle and inner rate loops.
6. Actuator authority: motor pre-clamp, post-clamp, saturation flag, diagonal motor/RPM imbalance.
7. Flight context: mode, armed state, throttle, notch filter state, BMP altitude/vertical speed, GPS validity, battery placeholder.

## Required `.ino` edits

### 1. Include Logger

Add near the other submodule includes:

```cpp
#include "src/Submodules/Logger/Logger.h"
```

### 2. Add Logger object

Replace or supersede the local `FlightLogRow`/`g_log[]` ring with:

```cpp
static Logger flightLogger(600);  // 100 Hz x 6 s
```

In `setup()` after mutex creation:

```cpp
flightLogger.begin();
```

### 3. Route existing `/flightlog/csv` providers to Logger

Keep the existing TelemetryWiFi flight-log endpoint. Replace provider wrappers with:

```cpp
static void resetFlightLog() {
    flightLogger.reset();
}

static uint16_t flightLogCount() {
    return flightLogger.countAndFreeze();
}

static String flightLogHeader() {
    return flightLogger.csvHeader();
}

static String flightLogRowCsv(uint16_t i) {
    return flightLogger.csvRow(i);
}
```

The current `TelemetryWiFi` already supports `setFlightLogCountProvider`, `setFlightLogHeaderProvider`, `setFlightLogRowProvider`, and `setFlightLogResetHandler`, so no HTTP transport rewrite is required.

### 4. Extend PID struct for P/I/D observability

Replace the current PID struct with this compatible version:

```cpp
struct PID {
    float kp, ki, kd, integral=0, prevError=0, iLimit=50.0f;
    float lastP=0.0f, lastI=0.0f, lastD=0.0f, lastOut=0.0f;

    PID(float p,float i,float d,float il=50.0f):kp(p),ki(i),kd(d),iLimit(il){}

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
        integral=0; prevError=0;
        lastP=lastI=lastD=lastOut=0.0f;
    }
};
```

### 5. Capture motor pre-clamp values

Replace the direct mixer assignment:

```cpp
float fl = thr + rO - pO - yO;
float fr = thr - rO - pO + yO;
float rl = thr + rO + pO + yO;
float rr = thr - rO + pO - yO;
```

with:

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

Then keep the existing desaturation and clamp logic on `fl/fr/rl/rr`.

### 6. Push LoggerRow at 100 Hz

Inside the existing `logDiv >= 4` block, replace `FlightLogRow row` with:

```cpp
LoggerRow row;
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
row.flags = ((cmd.mode != FlightMode::DISARMED) ? 0x0001 : 0)
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
row.ahrs_mode = g_ahrsFilterModeActive;
row.ekf_mag_used = attitudeEKF.lastMagAccepted() ? 1 : 0;

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

row.battery_v = BATTERY_VOLTAGE;   // replace later with ADC-measured battery voltage
row.cpu0_pct = g_state.cpuCore0_pct;
row.cpu1_pct = g_state.cpuCore1_pct;
row.notch_freq_hz = tune.notch_freq_hz;
row.notch_q = tune.notch_q;
row.notch_enable = tune.notch_enable ? 1 : 0;
row.bmp_alt_m = g_state.bmpAltitude_m;
row.bmp_vz_mps = g_state.bmpVerticalSpeed_mps;
row.gps_valid = g_state.gps.valid ? 1 : 0;
row.gps_sats = g_state.gps.satellites;
row.gps_hdop = g_state.gps.hdop;

flightLogger.push(row);
```

## Compile checklist

After applying the `.ino` changes:

```bash
git checkout feature/logger
arduino-cli compile --fqbn esp32:esp32:esp32 RC_FlightController
```

Then test endpoints while connected to ESP32 AP:

```text
GET  http://192.168.4.1/telemetry
POST http://192.168.4.1/flightlog/reset
GET  http://192.168.4.1/flightlog/csv
```
