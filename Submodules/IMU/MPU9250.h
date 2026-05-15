/**
 * ============================================================
 *  MPU9250.h  —  Driver + Calibration  (v3.0)
 *  Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)
 *  SPI (VSPI) — FreeRTOS compatible
 * ============================================================
 *
 *  Three-file project:
 *    MPU9250.h      ← this file  (declarations)
 *    MPU9250.cpp    ← implementations
 *    Drone_IMU_FreeRTOS.ino  ← FreeRTOS tasks + serial menu
 *
 *  The driver owns the SPI bus directly (no subclass override
 *  needed). Pass the CS pin to the constructor.
 *
 *  HUZZAH32 Feather wiring:
 *    MPU-9250  →  Feather
 *    VCC       →  3V   (3.3 V ONLY)
 *    GND       →  GND
 *    SCL/SCLK  →  SCK  (GPIO 5)
 *    SDA/MOSI  →  MO   (GPIO 18)
 *    AD0/MISO  →  MI   (GPIO 19)
 *    NCS/CS    →  33   (GPIO 33)
 *    INT       →  27   (GPIO 27)
 * ============================================================
 */

#pragma once
#ifndef MPU9250_H
#define MPU9250_H

#include <Arduino.h>
#include <SPI.h>
#include <math.h>
#include <Preferences.h>

// ─────────────────────────────────────────────────────────────
//  §1  Register Map — MPU-9250
// ─────────────────────────────────────────────────────────────
#define MPU_REG_SMPLRT_DIV      0x19
#define MPU_REG_CONFIG          0x1A
#define MPU_REG_GYRO_CONFIG     0x1B
#define MPU_REG_ACCEL_CONFIG    0x1C
#define MPU_REG_ACCEL_CONFIG2   0x1D
#define MPU_REG_INT_PIN_CFG     0x37
#define MPU_REG_INT_ENABLE      0x38
#define MPU_REG_INT_STATUS      0x3A
#define MPU_REG_ACCEL_XOUT_H    0x3B   // 14-byte burst start
#define MPU_REG_USER_CTRL       0x6A
#define MPU_REG_PWR_MGMT_1      0x6B
#define MPU_REG_PWR_MGMT_2      0x6C
#define MPU_REG_WHO_AM_I        0x75
#define MPU_WHO_AM_I_VAL        0x71   // Expected response

// ─────────────────────────────────────────────────────────────
//  §2  Register Map — AK8963 magnetometer (via I2C master)
// ─────────────────────────────────────────────────────────────
#define AK8963_ADDR             0x0C
#define MPU_REG_I2C_MST_CTRL    0x24
#define MPU_REG_I2C_SLV0_ADDR  0x25
#define MPU_REG_I2C_SLV0_REG   0x26
#define MPU_REG_I2C_SLV0_CTRL  0x27
#define MPU_REG_I2C_SLV1_ADDR  0x2A
#define MPU_REG_I2C_SLV1_REG   0x2B
#define MPU_REG_I2C_SLV1_CTRL  0x2C
// SLV4 — dedicated one-shot write slave (safer than SLV1 for AK writes)
#define MPU_REG_I2C_SLV4_ADDR  0x31
#define MPU_REG_I2C_SLV4_REG   0x32
#define MPU_REG_I2C_SLV4_DO    0x33
#define MPU_REG_I2C_SLV4_CTRL  0x34
#define MPU_REG_I2C_MST_STATUS 0x36
#define MPU_REG_EXT_SENS_DATA  0x49   // 8 mag bytes land here (ST1+6data+ST2)
#define AK_REG_WIA              0x00
#define AK_REG_ST1              0x02
#define AK_REG_HXL              0x03   // 6 data bytes
#define AK_REG_ST2              0x09
#define AK_REG_CNTL1            0x0A
#define AK_REG_CNTL2            0x0B
#define AK_REG_ASAX             0x10   // Factory sensitivity ROM
#define AK_WIA_VAL              0x48

// ─────────────────────────────────────────────────────────────
//  §3  Scale constants
// ─────────────────────────────────────────────────────────────
#define GYRO_SCALE_500DPS   (500.0f  / 32768.0f)   // °/s per LSB
#define ACCEL_SCALE_8G      (8.0f    / 32768.0f)   // g per LSB
#define MAG_SCALE_16BIT     (4912.0f / 32760.0f)   // µT per LSB
#define DEG2RAD             (3.14159265358979f / 180.0f)
#define RAD2DEG             (180.0f / 3.14159265358979f)

// ─────────────────────────────────────────────────────────────
//  §4  SPI speed constants
// ─────────────────────────────────────────────────────────────
#define MPU_SPI_SLOW    1000000UL   //  1 MHz — config writes
#define MPU_SPI_FAST   20000000UL   // 20 MHz — data reads

// ─────────────────────────────────────────────────────────────
//  §5  Data structures
// ─────────────────────────────────────────────────────────────

/** Raw 16-bit register values straight off the bus */
struct MPU_RawData {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t mx, my, mz;
    int16_t temp;
    bool    magOk;  // true if ST1 data-ready bit was set
};

/** Calibrated, scaled sensor data */
struct MPU_SensorData {
    float ax_g,  ay_g,  az_g;      // Accelerometer  (g)
    float gx_dps, gy_dps, gz_dps;  // Gyroscope      (°/s)
    float mx_uT, my_uT, mz_uT;     // Magnetometer   (µT)
    float temp_c;                   // Die temperature (°C)
    uint32_t ts_ms;                 // millis() timestamp
};

/** Euler angles + quaternion from AHRS */
struct MPU_Attitude {
    float roll;          // degrees, +right wing down
    float pitch;         // degrees, +nose up
    float yaw;           // degrees, 0–360 from magnetic north
    float q0, q1, q2, q3;  // unit quaternion (w,x,y,z)
};

/** Calibration parameters stored in NVS */
struct MPU_CalData {
    // Gyro bias (°/s)
    float gx_b, gy_b, gz_b;
    // Accel bias (g) and per-axis scale
    float ax_b, ay_b, az_b;
    float ax_s, ay_s, az_s;
    // Magnetometer hard-iron bias (µT) and soft-iron scale
    float mx_b, my_b, mz_b;
    float mx_s, my_s, mz_s;
    // AK8963 factory sensitivity adjustment (read once at init)
    float mag_asa_x, mag_asa_y, mag_asa_z;
    bool  valid;
};

// ─────────────────────────────────────────────────────────────
//  §6  MPU9250 class
// ─────────────────────────────────────────────────────────────
class MPU9250 {
public:
    // ── Construction ────────────────────────────────────────
    /**
     * @param csPin   GPIO used as SPI chip-select
     * @param spi     SPI bus (default SPI = VSPI on ESP32)
     */
    explicit MPU9250(uint8_t csPin, SPIClass& spi = SPI);

    // ── Initialisation ──────────────────────────────────────
    /**
     * Initialise sensor, read AK8963 factory cal, apply config.
     * Call once in setup() AFTER SPI.begin().
     * @return true if WHO_AM_I matches
     */
    bool begin();

    // ── Diagnostics ─────────────────────────────────────────
    bool    isConnected();      ///< WHO_AM_I check
    bool    isMagConnected();   ///< AK8963 WHO_AM_I check
    uint8_t whoAmI();           ///< Raw WHO_AM_I byte
    float   readTemperature();  ///< Die temperature °C

    // ── Data reads ──────────────────────────────────────────
    /**
     * Read raw 16-bit values from sensor registers.
     * Fastest path — no float math.
     */
    bool readRaw(MPU_RawData& out);

    /**
     * Read + scale + apply calibration.
     * Call this once per loop iteration.
     */
    bool readScaled(MPU_SensorData& out);

    // ── AHRS (Mahony 9-DOF) ─────────────────────────────────
    /**
     * Feed calibrated data into Mahony AHRS.
     * Must be called AFTER readScaled().
     * @param s   output of readScaled()
     * @param dt  time since last call (seconds)
     * @param att output attitude struct
     */
    void mahonyUpdate(const MPU_SensorData& s, float dt, MPU_Attitude& att);

    /** Mahony gains (tuneable) */
    float mahonyKp = 2.0f;
    float mahonyKi = 0.005f;

    // ── Calibration ─────────────────────────────────────────
    MPU_CalData cal;   ///< Public — read/write freely

    /** Average N raw samples and fill gyro+accel fields of out. */
    void sampleAvg(int N, MPU_SensorData& out);

    /** 1000-sample gyro bias estimation (drone must be still). */
    void calibrateGyro();

    /** 6-position accel calibration. Prompts via Serial. */
    void calibrateAccel();

    /** Hard/soft-iron mag calibration. Rotates for durationMs ms. */
    void calibrateMag(uint32_t durationMs = 20000);

    /** Print raw EXT_SENS_DATA bytes + parsed mag — for debugging Mx/My/Mz=0 */
    void diagMag();

    /** Save cal to ESP32 NVS flash. */
    void saveCalibration(const char* ns = "imu_cal");

    /** Load cal from ESP32 NVS flash. Returns true if found. */
    bool loadCalibration(const char* ns = "imu_cal");

    /** Erase NVS calibration namespace. */
    void eraseCalibration(const char* ns = "imu_cal");

    /** Print calibration to Serial. */
    void printCalibration();

private:
    uint8_t   _cs;
    SPIClass& _spi;

    // Mahony AHRS state
    float _q0 = 1.0f, _q1 = 0.0f, _q2 = 0.0f, _q3 = 0.0f;
    float _ix = 0.0f, _iy = 0.0f, _iz = 0.0f;  // integral error

    // Last mag values (updated only when new mag data arrives)
    float _last_mx = 0.0f, _last_my = 0.0f, _last_mz = 0.0f;
    bool  _magValid = false;

    // SPI helpers
    void    _writeReg(uint8_t reg, uint8_t val);
    uint8_t _readReg (uint8_t reg);
    void    _burstRead(uint8_t reg, uint8_t* buf, uint8_t len);

    // AK8963 access via MPU I2C master
    void    _akWrite(uint8_t akReg, uint8_t val);
    uint8_t _akReadByte(uint8_t akReg);
    void    _akBurstRead(uint8_t akReg, uint8_t* buf, uint8_t len);
    void    _akSetupContinuous();

    // Initialisation helpers
    bool    _initMPU();
    void    _readMagASA();

    static float _invSqrt(float x);
};

#endif // MPU9250_H
