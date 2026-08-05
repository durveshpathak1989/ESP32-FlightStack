#pragma once

// Inputs, outputs, units, and failure behavior: docs/SWC_INTERFACES.md

#include "../../../Application/FlightConfig.h"

#if FLIGHT_IMU_BACKEND == FLIGHT_IMU_BACKEND_MPU9250

#include "../../../Submodules/IMU/MPU9250.h"

using ImuSensorData = MPU_SensorData;

class SelectedImu final : public MPU9250 {
public:
    explicit SelectedImu(uint8_t chipSelectPin) : MPU9250(chipSelectPin) {}
    static constexpr const char* backendName() { return "MPU9250/MPU6500"; }
    static constexpr bool supportsManualCalibration() { return true; }
};

#elif FLIGHT_IMU_BACKEND == FLIGHT_IMU_BACKEND_BNO085

#if !__has_include(<Adafruit_BNO08x.h>)
#error "BNO085 selected. Install the Adafruit BNO08x Arduino library."
#endif

#include <Adafruit_BNO08x.h>
#include <Preferences.h>
#include <Wire.h>

struct ImuSensorData {
    float ax_g = 0.0f, ay_g = 0.0f, az_g = 0.0f;
    float gx_dps = 0.0f, gy_dps = 0.0f, gz_dps = 0.0f;
    float mx_uT = 0.0f, my_uT = 0.0f, mz_uT = 0.0f;
    float temp_c = 0.0f;
    uint32_t ts_ms = 0;
    bool magOk = false;
};

struct BnoCalibrationCompatibility {
    float gx_b = 0.0f, gy_b = 0.0f, gz_b = 0.0f;
    float ax_b = 0.0f, ay_b = 0.0f, az_b = 0.0f;
    float ax_s = 1.0f, ay_s = 1.0f, az_s = 1.0f;
    float mx_b = 0.0f, my_b = 0.0f, mz_b = 0.0f;
    float mx_s = 1.0f, my_s = 1.0f, mz_s = 1.0f;
    float mag_asa_x = 1.0f, mag_asa_y = 1.0f, mag_asa_z = 1.0f;
    bool valid = true;
};

// BNO085 adapter. Raw calibrated reports feed the existing estimator, keeping
// controller behavior consistent across IMU choices. Onboard fused rotation
// vectors can be exposed later through the optional-attitude capability port.
class SelectedImu final {
public:
    explicit SelectedImu(uint8_t /*unusedChipSelect*/) : sensor_(-1) {}

    bool begin() {
        Wire.begin(PIN_BMP_SDA, PIN_BMP_SCL, BNO085_I2C_HZ);
        if (!sensor_.begin_I2C(BNO085_I2C_ADDRESS, &Wire)) return false;
        const bool accel = sensor_.enableReport(
            SH2_ACCELEROMETER, BNO085_REPORT_INTERVAL_US);
        const bool gyro = sensor_.enableReport(
            SH2_GYROSCOPE_CALIBRATED, BNO085_REPORT_INTERVAL_US);
        const bool mag = sensor_.enableReport(
            SH2_MAGNETIC_FIELD_CALIBRATED, 10000);
        return accel && gyro && mag;
    }

    bool readScaled(ImuSensorData& output) {
        bool changed = false;
        bool gyroUpdated = false;
        sh2_SensorValue_t event;
        for (uint8_t count = 0; count < 16 && sensor_.getSensorEvent(&event); ++count) {
            switch (event.sensorId) {
                case SH2_ACCELEROMETER:
                    latest_.ax_g = event.un.accelerometer.x / kGravity;
                    latest_.ay_g = event.un.accelerometer.y / kGravity;
                    latest_.az_g = event.un.accelerometer.z / kGravity;
                    accelValid_ = true;
                    changed = true;
                    break;
                case SH2_GYROSCOPE_CALIBRATED:
                    latest_.gx_dps = event.un.gyroscope.x * kRadiansToDegrees;
                    latest_.gy_dps = event.un.gyroscope.y * kRadiansToDegrees;
                    latest_.gz_dps = event.un.gyroscope.z * kRadiansToDegrees;
                    gyroValid_ = true;
                    gyroUpdated = true;
                    changed = true;
                    break;
                case SH2_MAGNETIC_FIELD_CALIBRATED:
                    latest_.mx_uT = event.un.magneticField.x;
                    latest_.my_uT = event.un.magneticField.y;
                    latest_.mz_uT = event.un.magneticField.z;
                    latest_.magOk = true;
                    magValid_ = true;
                    changed = true;
                    break;
                default:
                    break;
            }
        }
        if (!changed || !gyroUpdated || !accelValid_ || !gyroValid_) return false;
        latest_.ts_ms = millis();
        output = latest_;
        return true;
    }

    bool isConnected() { return accelValid_ || gyroValid_; }
    bool isMagConnected() { return magValid_; }
    bool hasMag() const { return true; }
    static constexpr const char* backendName() { return "BNO085"; }
    static constexpr bool supportsManualCalibration() { return false; }

    void sampleAvg(int sampleCount, ImuSensorData& output) {
        ImuSensorData sum{};
        int accepted = 0;
        while (accepted < sampleCount) {
            ImuSensorData sample;
            if (!readScaled(sample)) { delay(1); continue; }
            sum.ax_g += sample.ax_g; sum.ay_g += sample.ay_g; sum.az_g += sample.az_g;
            sum.gx_dps += sample.gx_dps; sum.gy_dps += sample.gy_dps; sum.gz_dps += sample.gz_dps;
            ++accepted;
        }
        const float divisor = static_cast<float>(accepted);
        output = sum;
        output.ax_g /= divisor; output.ay_g /= divisor; output.az_g /= divisor;
        output.gx_dps /= divisor; output.gy_dps /= divisor; output.gz_dps /= divisor;
    }

    void saveCalibration(const char* = "imu_cal") {}
    bool loadCalibration(const char* = "imu_cal") { return true; }
    void eraseCalibration(const char* = "imu_cal") {}
    void printCalibration() {}
    void calibrateGyro() {}
    void calibrateAccel() {}
    void calibrateMag(uint32_t = 20000) {}
    void diagMag() {}

    BnoCalibrationCompatibility cal;

private:
    static constexpr float kGravity = 9.80665f;
    static constexpr float kRadiansToDegrees = 57.29577951308232f;
    static constexpr uint32_t BNO085_I2C_HZ = 400000;
    Adafruit_BNO08x sensor_;
    ImuSensorData latest_{};
    bool accelValid_ = false;
    bool gyroValid_ = false;
    bool magValid_ = false;
};

#else
#error "Unsupported FLIGHT_IMU_BACKEND value"
#endif
