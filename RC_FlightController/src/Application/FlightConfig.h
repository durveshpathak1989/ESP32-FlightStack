#pragma once

// FlightConfig.h is the first file to open when adapting the aircraft.
// It contains hardware assignments, safe flight defaults, filter settings,
// and task timing. Runtime values changed by the GCS are stored separately in
// ESP32 NVS and override the TUNE_* defaults after boot.

#include <cstddef>
#include <cstdint>

// IMU backend selection. The default requires no new dependency. To use a
// BNO085, install Adafruit BNO08x and compile with
// -DFLIGHT_IMU_BACKEND=FLIGHT_IMU_BACKEND_BNO085.
#define FLIGHT_IMU_BACKEND_MPU9250 1
#define FLIGHT_IMU_BACKEND_BNO085 2
#ifndef FLIGHT_IMU_BACKEND
#define FLIGHT_IMU_BACKEND FLIGHT_IMU_BACKEND_MPU9250
#endif

// Hardware pins -------------------------------------------------------------
constexpr uint8_t PIN_SPI_SCK = 5;
constexpr uint8_t PIN_SPI_MISO = 19;
constexpr uint8_t PIN_SPI_MOSI = 18;
constexpr uint8_t PIN_MPU_CS = 33;
constexpr uint8_t PIN_MPU_INT = 27;
constexpr uint8_t PIN_MOTOR_FL = 25;
constexpr uint8_t PIN_MOTOR_FR = 4;
constexpr uint8_t PIN_MOTOR_RL = 26;
constexpr uint8_t PIN_MOTOR_RR = 32;
constexpr uint8_t PIN_IBUS_RX = 16;
constexpr uint8_t PIN_IBUS_TX = 2;
constexpr uint8_t PIN_BMP_SDA = 21;
constexpr uint8_t PIN_BMP_SCL = 22;
constexpr uint8_t PIN_GPS_RX = 23;
constexpr uint8_t PIN_GPS_TX = 17;
constexpr uint8_t PIN_BATTERY_ADC = 34;
constexpr uint8_t BNO085_I2C_ADDRESS = 0x4A;
constexpr uint32_t BNO085_REPORT_INTERVAL_US = 2500;

// Control-loop timing and filtering -----------------------------------------
constexpr size_t TIMING_BUF_SIZE = 800;
constexpr uint32_t TIMING_TARGET_US = 2500;
constexpr uint32_t JITTER_VIOLATION_US = 100;
constexpr float GYRO_LPF_HZ = 50.0f;
constexpr float RC_LPF_HZ = 70.0f;

// Time-of-flight sensor ------------------------------------------------------
constexpr uint8_t TOF_ST_ADDRESS = 0x12;
constexpr uint32_t TOF_I2C_HZ = 400000;
constexpr uint32_t TOF_TIMING_BUDGET_US = 20000;
constexpr uint32_t TOF_INTER_MEASUREMENT_MS = 25;
constexpr uint32_t TOF_TASK_PERIOD_MS = 25;
constexpr uint32_t TOF_STALE_MS = 200;

// Pilot limits and motor authority ------------------------------------------
constexpr float TUNE_MAX_ANGLE_DEG = 5.0f;
constexpr float TUNE_MAX_RATE_DPS = 200.0f;
constexpr float TUNE_ROLL_OUTPUT_LIMIT = 0.120f;
constexpr float TUNE_PITCH_OUTPUT_LIMIT = 0.120f;
constexpr float TUNE_YAW_OUTPUT_LIMIT = 0.120f;

// Throttle shaping ----------------------------------------------------------
constexpr float TUNE_THROTTLE_EXPO = 0.70f;
constexpr float TUNE_THROTTLE_UP_RATE_PER_SEC = 0.50f;
constexpr float TUNE_THROTTLE_DOWN_RATE_PER_SEC = 0.50f;
constexpr float TUNE_MOTOR_IDLE = 0.08f;
constexpr float TUNE_MOTOR_MAX = 1.00f;
constexpr float TUNE_THROTTLE_CUT = 0.03f;
constexpr float TUNE_IDLE_RAMP_END = 0.15f;

// Rate-loop PID defaults -----------------------------------------------------
constexpr float TUNE_RATE_ROLL_KP = 0.001300f;
constexpr float TUNE_RATE_ROLL_KI = 0.000700f;
constexpr float TUNE_RATE_ROLL_KD = 0.000000010f;
constexpr float TUNE_RATE_ROLL_FF = 0.000000f;
constexpr float TUNE_RATE_ROLL_D_LPF_HZ = 100.0f;
constexpr float TUNE_RATE_PITCH_KP = 0.001300f;
constexpr float TUNE_RATE_PITCH_KI = 0.000700f;
constexpr float TUNE_RATE_PITCH_KD = 0.000000010f;
constexpr float TUNE_RATE_PITCH_FF = 0.000000f;
constexpr float TUNE_RATE_PITCH_D_LPF_HZ = 100.0f;
constexpr float TUNE_RATE_YAW_KP = 0.008000f;
constexpr float TUNE_RATE_YAW_KI = 0.001000f;
constexpr float TUNE_RATE_YAW_KD = 0.0000000f;
constexpr float TUNE_RATE_YAW_FF = 0.000000f;
constexpr float TUNE_RATE_YAW_D_LPF_HZ = 100.0f;

// Angle-loop and yaw-hold PID defaults --------------------------------------
constexpr float TUNE_ANGLE_ROLL_KP = 1.50f;
constexpr float TUNE_ANGLE_ROLL_KI = 0.000f;
constexpr float TUNE_ANGLE_ROLL_KD = 0.000000f;
constexpr float TUNE_ANGLE_PITCH_KP = 1.00f;
constexpr float TUNE_ANGLE_PITCH_KI = 0.000f;
constexpr float TUNE_ANGLE_PITCH_KD = 0.000000f;
constexpr float TUNE_ANGLE_YAW_KP = 1.00f;
constexpr float TUNE_YAW_DEADBAND = 0.02f;
constexpr float TUNE_YAW_MAX_RATE_DPS = 20.0f;

// Vibration filtering -------------------------------------------------------
constexpr bool TUNE_NOTCH_ENABLE = true;
constexpr float TUNE_NOTCH_FREQ_HZ = 105.38f;
constexpr float TUNE_NOTCH_Q = 10.0f;
constexpr float NOTCH_SAMPLE_HZ = 400.0f;
constexpr bool TUNE_DYNAMIC_NOTCH_ENABLE = true;
constexpr float DYN_NOTCH_MIN_HZ = 45.0f;
constexpr float DYN_NOTCH_MAX_HZ = 170.0f;
constexpr uint32_t DYN_NOTCH_UPDATE_MS = 1000;
constexpr float DYN_NOTCH_ALPHA = 0.25f;
constexpr float DYN_NOTCH_MAX_STEP_HZ = 8.0f;
constexpr float DYN_NOTCH_MIN_THROTTLE = 0.15f;
constexpr float DYN_NOTCH_MIN_SCORE = 3.5f;

// Runtime diagnostics -------------------------------------------------------
constexpr bool LOG_CONTROL_TIMING = true;
constexpr bool LOG_MOTOR_SATURATION = false;
constexpr bool LOG_MOTOR_WRITE_RATE = false;
constexpr bool LOG_DYNAMIC_NOTCH_DEBUG = false;

// Attitude-estimator defaults -----------------------------------------------
constexpr float TUNE_EKF_ANGLE_Q = 0.0008f;
constexpr float TUNE_EKF_BIAS_Q = 0.000001f;
constexpr float TUNE_EKF_ACCEL_R = 0.400f;
constexpr float TUNE_EKF_MAG_R = 0.200f;
constexpr float TUNE_EKF_MAG_DECL_DEG = 0.0f;
constexpr float TUNE_EKF_MAG_YAW_OFF_DEG = 0.0f;
constexpr float TUNE_EKF_MAG_YAW_SIGN = 1.0f;
constexpr float TUNE_AHRS_FILTER_MODE = 0.0f;
constexpr float TUNE_MADGWICK_BETA = 0.080f;

// Calibration and level-zero capture ---------------------------------------
constexpr uint32_t LEVEL_ZERO_SAMPLE_MS = 1200;
constexpr uint16_t LEVEL_ZERO_SWB_THRESHOLD = 1700;
constexpr uint16_t SWC_THRESHOLD = 1700;
constexpr uint16_t ESC_CALIB_VRB_THRESHOLD = 1900;
constexpr uint32_t GYRO_SETTLE_MS = 3000;
constexpr uint32_t GYRO_SAMPLE_MS = 5000;
constexpr uint32_t ACCEL_HOLD_MS = 3000;
constexpr uint32_t ACCEL_WAIT_MAX_MS = 30000;
constexpr uint32_t MAG_DURATION_MS = 30000;

// Aircraft and battery ------------------------------------------------------
constexpr float MOTOR_KV = 920.0f;
constexpr float BATTERY_NOMINAL_V = 11.1f;
constexpr uint8_t BATTERY_CELL_COUNT = 3;
constexpr float BATTERY_RTOP_OHMS = 235000.0f;
constexpr float BATTERY_RBOTTOM_OHMS = 35000.0f;
constexpr float BATTERY_ADC_CAL_SCALE = 1.0f;
constexpr uint16_t FLIGHT_LOGGER_CAPACITY = 120;
