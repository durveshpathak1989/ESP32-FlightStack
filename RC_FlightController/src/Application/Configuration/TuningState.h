#pragma once

// Plain runtime configuration shared by controllers and estimators. This file
// is platform-independent: validation happens at input boundaries and storage
// is supplied by a platform adapter.
// Contract details: docs/SWC_INTERFACES.md
struct TuningState {
    float max_angle_deg;
    float max_rate_dps;
    float max_pitch_rate_dps;
    float roll_output_limit;
    float pitch_output_limit;
    float yaw_output_limit;
    float throttle_expo;
    float throttle_up_rate_per_sec;
    float throttle_down_rate_per_sec;
    float motor_idle;
    float motor_max;
    float throttle_cut;
    float idle_ramp_end;
    float pid_ilimit;
    float pid_roll_kp, pid_roll_ki, pid_roll_kd;
    float pid_pitch_kp, pid_pitch_ki, pid_pitch_kd;
    float pid_yaw_kp, pid_yaw_ki, pid_yaw_kd;
    float pid_roll_ff, pid_pitch_ff, pid_yaw_ff;
    float pid_roll_d_lpf_hz, pid_pitch_d_lpf_hz, pid_yaw_d_lpf_hz;
    float pid_angle_roll_kp, pid_angle_roll_ki, pid_angle_roll_kd;
    float pid_angle_pitch_kp, pid_angle_pitch_ki, pid_angle_pitch_kd;
    float pid_angle_yaw_kp;
    float yaw_deadband;
    float yaw_max_rate_dps;
    float mahony_kp, mahony_ki;
    float ahrs_filter_mode;
    float madgwick_beta;
    bool notch_enable;
    float notch_freq_hz;
    float notch_q;
    float ekf_angle_q;
    float ekf_bias_q;
    float ekf_accel_r;
    float ekf_mag_r;
    float ekf_mag_declination_deg;
    float ekf_mag_yaw_offset_deg;
    float ekf_mag_yaw_sign;
    volatile bool dirty;
};
