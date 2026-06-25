/*
 * Name: LoggingMechanism.h
 * Use: High-speed diagnostic flight-log ring buffer for EKF, target tracking,
 *      PID terms, motor saturation, sensor quality, and timing telemetry.
 * Version: 1.0.0
 * Baseline: Test_Quad firmware v5.0.0
 */

#pragma once
#ifndef LOGGING_MECHANISM_H
#define LOGGING_MECHANISM_H

#include <Arduino.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#ifndef LOGGING_MECHANISM_DEFAULT_CAPACITY
#define LOGGING_MECHANISM_DEFAULT_CAPACITY 600U   // 100 Hz x 6 s, safe ESP32 default
#endif

// Compact but diagnosis-oriented row. Keep POD/simple types so it can be copied
// inside a short critical section without heap allocation.
struct DiagnosticLogRow {
    // Time / scheduler
    uint32_t t_us;
    uint32_t loop_count;
    uint16_t period_us;
    int16_t  jitter_us;
    uint16_t control_exec_us;
    uint16_t imu_read_us;
    uint16_t rc_read_us;
    uint16_t ahrs_exec_us;
    uint16_t pid_exec_us;
    uint16_t motor_exec_us;
    uint8_t  mode;        // 0=DISARMED, 1=ANGLE, 2=ACRO, 3=FAILSAFE
    uint16_t flags;       // bit0 armed, bit1 imuOk, bit2 rcValid, bit3 magValid, bit4 motorSat, bit5 rpmActualValid

    // RC and targets
    float throttle;
    float rc_roll;
    float rc_pitch;
    float rc_yaw;
    float target_roll_deg;
    float target_pitch_deg;
    float target_yaw_deg;
    float target_roll_rate_dps;
    float target_pitch_rate_dps;
    float target_yaw_rate_dps;

    // Estimator and zero-corrected controller attitude
    float ekf_roll_deg;
    float ekf_pitch_deg;
    float ekf_yaw_deg;
    float ctrl_roll_deg;
    float ctrl_pitch_deg;
    float ctrl_yaw_deg;
    float zero_roll_deg;
    float zero_pitch_deg;
    float zero_yaw_deg;

    // Sensor quality
    float ax_g;
    float ay_g;
    float az_g;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    float mx_uT;
    float my_uT;
    float mz_uT;
    float accel_norm_g;
    float gyro_norm_dps;
    float mag_norm_uT;
    float ekf_bgx_dps;
    float ekf_bgy_dps;
    float ekf_bgz_dps;

    // Tracking errors
    float angle_roll_error_deg;
    float angle_pitch_error_deg;
    float yaw_error_deg;
    float rate_roll_error_dps;
    float rate_pitch_error_dps;
    float rate_yaw_error_dps;

    // Outer angle-loop terms
    float angle_roll_p;
    float angle_roll_i;
    float angle_roll_d;
    float angle_roll_out;
    float angle_pitch_p;
    float angle_pitch_i;
    float angle_pitch_d;
    float angle_pitch_out;
    float angle_yaw_p;
    float angle_yaw_i;
    float angle_yaw_d;
    float angle_yaw_out;

    // Inner rate-loop terms
    float rate_roll_p;
    float rate_roll_i;
    float rate_roll_d;
    float rate_roll_out;
    float rate_pitch_p;
    float rate_pitch_i;
    float rate_pitch_d;
    float rate_pitch_out;
    float rate_yaw_p;
    float rate_yaw_i;
    float rate_yaw_d;
    float rate_yaw_out;

    // Motor mixer, before/after limiting
    float motor_fl_pre;
    float motor_fr_pre;
    float motor_rl_pre;
    float motor_rr_pre;
    float motor_fl;
    float motor_fr;
    float motor_rl;
    float motor_rr;
    float motor_diag_a;      // FL + RR
    float motor_diag_b;      // FR + RL
    float motor_diag_diff;   // A - B

    // RPM
    float rpm_fl;
    float rpm_fr;
    float rpm_rl;
    float rpm_rr;
    float rpm_diag_a;
    float rpm_diag_b;
    float rpm_diag_diff;

    // Power / CPU / filter
    float battery_v;
    float cpu0_pct;
    float cpu1_pct;
    float notch_freq_hz;
    float notch_q;
};

class LoggingMechanism {
public:
    explicit LoggingMechanism(uint16_t capacity = LOGGING_MECHANISM_DEFAULT_CAPACITY);
    ~LoggingMechanism();

    bool begin();
    void reset();
    void freeze();
    void resume();

    bool push(const DiagnosticLogRow& row);
    uint16_t countAndFreeze();
    uint16_t capacity() const { return _capacity; }
    bool isFrozen() const { return !_active; }

    String csvHeader() const;
    String csvRow(uint16_t chronologicalIndex) const;
    String statusJson() const;

private:
    DiagnosticLogRow* _rows;
    uint16_t _capacity;
    volatile uint16_t _head;
    volatile bool _full;
    volatile bool _active;
    volatile uint32_t _dropped;
    portMUX_TYPE _mux;

    uint16_t _availableUnsafe() const;
    uint16_t _mapChronologicalIndex(uint16_t chronologicalIndex) const;
    static void _appendFloat(String& s, float v, uint8_t dp);
};

#endif // LOGGING_MECHANISM_H
