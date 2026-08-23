#pragma once
// Core layer: platform-independent IMU data contracts.
// Consumed by estimation algorithms (App) and sensor drivers
// (EcuAbstraction). No driver or Arduino headers may appear here.

#include <stdint.h>

#ifndef DEG2RAD
#define DEG2RAD             (3.14159265358979f / 180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG             (180.0f / 3.14159265358979f)
#endif

struct MPU_RawData {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t mx, my, mz;
    int16_t temp;
    bool    magOk;
};

struct MPU_SensorData {
    float ax_g,  ay_g,  az_g;
    float gx_dps, gy_dps, gz_dps;
    float mx_uT, my_uT, mz_uT;
    float temp_c;
    uint32_t ts_ms;
    bool magOk;
};

struct MPU_CalData {
    float gx_b, gy_b, gz_b;
    float ax_b, ay_b, az_b;
    float ax_s, ay_s, az_s;
    float mx_b, my_b, mz_b;
    float mx_s, my_s, mz_s;
    float mag_asa_x, mag_asa_y, mag_asa_z;
    bool  valid;
};
