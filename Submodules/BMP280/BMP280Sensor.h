/**
 * ============================================================
 *  BMP280Sensor.h — BMP280 Barometer / Altitude Subsystem
 *  ESP32 / FreeRTOS compatible, no external library required
 * ============================================================
 *
 *  Default wiring on ESP32-WROOM-32E / HUZZAH32-style boards:
 *    BMP280 VIN/VCC  -> 3.3 V
 *    BMP280 GND      -> GND
 *    BMP280 SDA/SDI  -> GPIO 21
 *    BMP280 SCL/SCK  -> GPIO 22
 *    BMP280 CSB      -> 3.3 V   (I2C mode)
 *    BMP280 SDO      -> GND for address 0x76, or 3.3 V for 0x77
 *
 *  Keep BMP280 on I2C. Your MPU-9250 is already using SPI, so this
 *  does not consume the MPU SPI pins.
 * ============================================================
 */

#pragma once
#ifndef BMP280_SENSOR_H
#define BMP280_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct BMP280Data {
    float temperature_c;   // °C
    float pressure_hpa;    // hPa / mbar
    float altitude_m;      // meters, relative to configured sea-level pressure
    bool  valid;
    uint32_t ts_ms;
};

class BMP280Sensor {
public:
    explicit BMP280Sensor(TwoWire& wire = Wire);

    bool begin(uint8_t address = 0x76,
               int sdaPin = 21,
               int sclPin = 22,
               uint32_t i2cHz = 400000);

    // Tries both common BMP280 I2C addresses: 0x76 and 0x77.
    bool beginAuto(int sdaPin = 21, int sclPin = 22, uint32_t i2cHz = 100000);

    // Debug helper: prints all detected I2C devices on the selected pins.
    void scanI2C(int sdaPin = 21, int sclPin = 22, uint32_t i2cHz = 100000);

    bool read(BMP280Data& out);
    bool isConnected();

    void setSeaLevelPressure(float hpa);
    float seaLevelPressure() const;
    uint8_t address() const;

private:
    TwoWire& _wire;
    uint8_t _addr;
    bool _begun;
    float _seaLevelHpa;
    SemaphoreHandle_t _mutex;

    // Factory compensation data from BMP280 NVM
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    int32_t  t_fine;

    uint8_t  _read8(uint8_t reg);
    uint16_t _read16LE(uint8_t reg);
    int16_t  _readS16LE(uint8_t reg);
    void     _write8(uint8_t reg, uint8_t value);
    bool     _readBytes(uint8_t reg, uint8_t* buf, size_t len);
    bool     _readCalibration();
    float    _compensateTemperature(int32_t adc_T);
    float    _compensatePressure(int32_t adc_P);
};

extern BMP280Sensor bmp280;

#endif // BMP280_SENSOR_H
