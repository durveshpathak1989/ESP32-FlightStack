#pragma once

#include <algorithm>

#include "../../../Application/Configuration/TuningState.h"
#include "../../../Submodules/WiFiTelemetry/TelemetryWiFi.h"

// Boundary adapter from optional HTTP/configurator fields to one validated
// application tuning snapshot. It has no storage or controller side effects.
class TelemetryTuneAdapter {
public:
    static void apply(const TunePacket& in, TuningState& out,
                      float notchSampleHz) {
        if (in.has_max_angle_deg) out.max_angle_deg = clamp(in.max_angle_deg, 5.0f, 80.0f);
        if (in.has_max_rate_dps) out.max_rate_dps = clamp(in.max_rate_dps, 30.0f, 1200.0f);
        if (in.has_max_pitch_rate_dps) out.max_pitch_rate_dps = clamp(in.max_pitch_rate_dps, 30.0f, 1200.0f);
        if (in.has_roll_output_limit) out.roll_output_limit = clamp(in.roll_output_limit, 0.02f, 1.0f);
        if (in.has_pitch_output_limit) out.pitch_output_limit = clamp(in.pitch_output_limit, 0.02f, 1.0f);
        if (in.has_yaw_output_limit) out.yaw_output_limit = clamp(in.yaw_output_limit, 0.01f, 1.0f);
        if (in.has_throttle_expo) out.throttle_expo = clamp(in.throttle_expo, 0.0f, 0.95f);
        if (in.has_throttle_up_rate_per_sec) out.throttle_up_rate_per_sec = clamp(in.throttle_up_rate_per_sec, 0.05f, 10.0f);
        if (in.has_throttle_down_rate_per_sec) out.throttle_down_rate_per_sec = clamp(in.throttle_down_rate_per_sec, 0.05f, 10.0f);
        if (in.has_motor_idle) out.motor_idle = clamp(in.motor_idle, 0.0f, 0.40f);
        if (in.has_motor_max) out.motor_max = clamp(in.motor_max, 0.10f, 1.0f);
        if (in.has_throttle_cut) out.throttle_cut = clamp(in.throttle_cut, 0.0f, 0.30f);
        if (in.has_idle_ramp_end) out.idle_ramp_end = clamp(in.idle_ramp_end, 0.01f, 0.60f);
        if (in.has_pid_ilimit) out.pid_ilimit = clamp(in.pid_ilimit, 0.0f, 1000.0f);
        if (out.motor_idle > out.motor_max) out.motor_idle = out.motor_max;
        if (out.idle_ramp_end <= out.throttle_cut) out.idle_ramp_end = out.throttle_cut + 0.01f;

        if (in.has_pid_roll_kp) out.pid_roll_kp = in.pid_roll_kp;
        if (in.has_pid_roll_ki) out.pid_roll_ki = in.pid_roll_ki;
        if (in.has_pid_roll_kd) out.pid_roll_kd = in.pid_roll_kd;
        if (in.has_pid_pitch_kp) out.pid_pitch_kp = in.pid_pitch_kp;
        if (in.has_pid_pitch_ki) out.pid_pitch_ki = in.pid_pitch_ki;
        if (in.has_pid_pitch_kd) out.pid_pitch_kd = in.pid_pitch_kd;
        if (in.has_pid_yaw_kp) out.pid_yaw_kp = in.pid_yaw_kp;
        if (in.has_pid_yaw_ki) out.pid_yaw_ki = in.pid_yaw_ki;
        if (in.has_pid_yaw_kd) out.pid_yaw_kd = in.pid_yaw_kd;
        if (in.has_pid_roll_ff) out.pid_roll_ff = in.pid_roll_ff;
        if (in.has_pid_pitch_ff) out.pid_pitch_ff = in.pid_pitch_ff;
        if (in.has_pid_yaw_ff) out.pid_yaw_ff = in.pid_yaw_ff;
        if (in.has_pid_roll_d_lpf_hz) out.pid_roll_d_lpf_hz = clamp(in.pid_roll_d_lpf_hz, 0.0f, 200.0f);
        if (in.has_pid_pitch_d_lpf_hz) out.pid_pitch_d_lpf_hz = clamp(in.pid_pitch_d_lpf_hz, 0.0f, 200.0f);
        if (in.has_pid_yaw_d_lpf_hz) out.pid_yaw_d_lpf_hz = clamp(in.pid_yaw_d_lpf_hz, 0.0f, 200.0f);
        if (in.has_pid_angle_roll_kp) out.pid_angle_roll_kp = in.pid_angle_roll_kp;
        if (in.has_pid_angle_roll_ki) out.pid_angle_roll_ki = in.pid_angle_roll_ki;
        if (in.has_pid_angle_roll_kd) out.pid_angle_roll_kd = in.pid_angle_roll_kd;
        if (in.has_pid_angle_pitch_kp) out.pid_angle_pitch_kp = in.pid_angle_pitch_kp;
        if (in.has_pid_angle_pitch_ki) out.pid_angle_pitch_ki = in.pid_angle_pitch_ki;
        if (in.has_pid_angle_pitch_kd) out.pid_angle_pitch_kd = in.pid_angle_pitch_kd;
        if (in.has_pid_angle_yaw_kp) out.pid_angle_yaw_kp = in.pid_angle_yaw_kp;
        if (in.has_yaw_deadband) out.yaw_deadband = clamp(in.yaw_deadband, 0.0f, 0.50f);
        if (in.has_yaw_max_rate_dps) out.yaw_max_rate_dps = clamp(in.yaw_max_rate_dps, 10.0f, 500.0f);
        if (in.has_mahony_kp) out.mahony_kp = in.mahony_kp;
        if (in.has_mahony_ki) out.mahony_ki = in.mahony_ki;
        if (in.has_ahrs_filter_mode) out.ahrs_filter_mode = clamp(in.ahrs_filter_mode, 0.0f, 2.0f);
        if (in.has_madgwick_beta) out.madgwick_beta = clamp(in.madgwick_beta, 0.001f, 1.0f);
        if (in.has_notch_enable) out.notch_enable = in.notch_enable;
        if (in.has_notch_freq_hz) out.notch_freq_hz = clamp(in.notch_freq_hz, 1.0f, notchSampleHz * 0.45f);
        if (in.has_notch_q) out.notch_q = clamp(in.notch_q, 0.5f, 50.0f);
        if (in.has_ekf_angle_q) out.ekf_angle_q = clamp(in.ekf_angle_q, 0.000001f, 0.050000f);
        if (in.has_ekf_bias_q) out.ekf_bias_q = clamp(in.ekf_bias_q, 0.000000001f, 0.001000f);
        if (in.has_ekf_accel_r) out.ekf_accel_r = clamp(in.ekf_accel_r, 0.001f, 2.0f);
        if (in.has_ekf_mag_r) out.ekf_mag_r = clamp(in.ekf_mag_r, 0.001f, 5.0f);
        if (in.has_ekf_mag_declination_deg) out.ekf_mag_declination_deg = clamp(in.ekf_mag_declination_deg, -30.0f, 30.0f);
        if (in.has_ekf_mag_yaw_offset_deg) out.ekf_mag_yaw_offset_deg = clamp(in.ekf_mag_yaw_offset_deg, -180.0f, 180.0f);
        if (in.has_ekf_mag_yaw_sign) out.ekf_mag_yaw_sign = in.ekf_mag_yaw_sign < 0.0f ? -1.0f : 1.0f;
    }

private:
    static float clamp(float value, float minimum, float maximum) {
        return std::max(minimum, std::min(value, maximum));
    }
};
