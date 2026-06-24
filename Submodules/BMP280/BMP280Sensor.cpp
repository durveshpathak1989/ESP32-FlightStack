/**
 * ============================================================
 *  BMP280Sensor.cpp — BMP280 Barometer / Altitude Subsystem
 * ============================================================
 */

#include "BMP280Sensor.h"
#include "DebugConfig.h"

// BMP280 register map
#define BMP280_REG_ID          0xD0
#define BMP280_REG_RESET       0xE0
#define BMP280_REG_STATUS      0xF3
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_CONFIG      0xF5
#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_CALIB00     0x88
#define BMP280_CHIP_ID         0x58

BMP280Sensor bmp280(Wire);

BMP280Sensor::BMP280Sensor(TwoWire& wire)
    : _wire(wire), _addr(0x76), _begun(false), _seaLevelHpa(1013.25f),
      dig_T1(0), dig_T2(0), dig_T3(0), dig_P1(0), dig_P2(0), dig_P3(0),
      dig_P4(0), dig_P5(0), dig_P6(0), dig_P7(0), dig_P8(0), dig_P9(0),
      t_fine(0)
{
    _mutex = xSemaphoreCreateMutex();
}

bool BMP280Sensor::begin(uint8_t address, int sdaPin, int sclPin, uint32_t i2cHz)
{
    _addr = address;
    _wire.begin(sdaPin, sclPin, i2cHz);
    delay(25);

    uint8_t id = _read8(BMP280_REG_ID);
    if (id != BMP280_CHIP_ID) {
        DBG_PRINTF("[BMP280] WHO_AM_I=0x%02X expected 0x58 at address 0x%02X\n", id, _addr);
        return false;
    }

    if (!_readCalibration()) {
        DBG_PRINTLN(F("[BMP280] Failed to read compensation data"));
        return false;
    }

    // Standby 125 ms, IIR filter x4. Good balance for altitude display.
    _write8(BMP280_REG_CONFIG, 0b01001100);

    // Normal mode, temp oversampling x2, pressure oversampling x16.
    _write8(BMP280_REG_CTRL_MEAS, 0b01010111);

    _begun = true;
    DBG_PRINTF("[BMP280] OK at I2C address 0x%02X, SDA=GPIO%d, SCL=GPIO%d\n", _addr, sdaPin, sclPin);
    return true;
}


bool BMP280Sensor::beginAuto(int sdaPin, int sclPin, uint32_t i2cHz)
{
    _wire.begin(sdaPin, sclPin, i2cHz);
    delay(25);

    const uint8_t addresses[] = {0x76, 0x77};
    for (uint8_t i = 0; i < 2; i++) {
        _addr = addresses[i];
        uint8_t id = _read8(BMP280_REG_ID);
        DBG_PRINTF("[BMP280] Probe address 0x%02X: chip ID 0x%02X\n", _addr, id);
        if (id == BMP280_CHIP_ID) {
            return begin(_addr, sdaPin, sclPin, i2cHz);
        }
    }

    DBG_PRINTLN(F("[BMP280] Not found at 0x76 or 0x77."));
    DBG_PRINTLN(F("[BMP280] Check: VCC=3.3V, GND common, SDA=21, SCL=22, CSB=3.3V."));
    DBG_PRINTLN(F("[BMP280] If your board labels are SDI/SCK, use SDI->SDA and SCK->SCL."));
    return false;
}

void BMP280Sensor::scanI2C(int sdaPin, int sclPin, uint32_t i2cHz)
{
    _wire.begin(sdaPin, sclPin, i2cHz);
    delay(25);
    DBG_PRINTF("[I2C] Scanning SDA=GPIO%d SCL=GPIO%d at %lu Hz...\n", sdaPin, sclPin, (unsigned long)i2cHz);
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        _wire.beginTransmission(addr);
        uint8_t err = _wire.endTransmission();
        if (err == 0) {
            DBG_PRINTF("[I2C] Found device at 0x%02X\n", addr);
            found++;
        }
    }
    if (!found) DBG_PRINTLN(F("[I2C] No devices found. Wiring/power/pins are likely wrong."));
}

bool BMP280Sensor::isConnected()
{
    return _read8(BMP280_REG_ID) == BMP280_CHIP_ID;
}

void BMP280Sensor::setSeaLevelPressure(float hpa)
{
    if (hpa > 800.0f && hpa < 1200.0f) _seaLevelHpa = hpa;
}

float BMP280Sensor::seaLevelPressure() const { return _seaLevelHpa; }
uint8_t BMP280Sensor::address() const { return _addr; }

bool BMP280Sensor::read(BMP280Data& out)
{
    memset(&out, 0, sizeof(out));
    out.valid = false;

    if (!_begun) return false;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return false;

    uint8_t buf[6];
    bool ok = _readBytes(BMP280_REG_PRESS_MSB, buf, sizeof(buf));
    if (!ok) {
        xSemaphoreGive(_mutex);
        return false;
    }

    int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
    int32_t adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);

    float tempC = _compensateTemperature(adc_T);
    float pressurePa = _compensatePressure(adc_P);
    xSemaphoreGive(_mutex);

    if (pressurePa <= 0.0f || isnan(pressurePa)) return false;

    out.temperature_c = tempC;
    out.pressure_hpa = pressurePa / 100.0f;
    out.altitude_m = 44330.0f * (1.0f - powf(out.pressure_hpa / _seaLevelHpa, 0.19029495f));
    out.valid = true;
    out.ts_ms = millis();
    return true;
}

uint8_t BMP280Sensor::_read8(uint8_t reg)
{
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) return 0xFF;
    if (_wire.requestFrom((int)_addr, 1) != 1) return 0xFF;
    return _wire.read();
}

uint16_t BMP280Sensor::_read16LE(uint8_t reg)
{
    uint8_t b[2] = {0, 0};
    _readBytes(reg, b, 2);
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}

int16_t BMP280Sensor::_readS16LE(uint8_t reg)
{
    return (int16_t)_read16LE(reg);
}

void BMP280Sensor::_write8(uint8_t reg, uint8_t value)
{
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    _wire.write(value);
    _wire.endTransmission();
}

bool BMP280Sensor::_readBytes(uint8_t reg, uint8_t* buf, size_t len)
{
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) return false;

    size_t got = _wire.requestFrom((int)_addr, (int)len);
    if (got != len) return false;
    for (size_t i = 0; i < len; i++) buf[i] = _wire.read();
    return true;
}

bool BMP280Sensor::_readCalibration()
{
    uint8_t c[24];
    if (!_readBytes(BMP280_REG_CALIB00, c, sizeof(c))) return false;

    dig_T1 = (uint16_t)c[0]  | ((uint16_t)c[1]  << 8);
    dig_T2 = (int16_t)((uint16_t)c[2]  | ((uint16_t)c[3]  << 8));
    dig_T3 = (int16_t)((uint16_t)c[4]  | ((uint16_t)c[5]  << 8));
    dig_P1 = (uint16_t)c[6]  | ((uint16_t)c[7]  << 8);
    dig_P2 = (int16_t)((uint16_t)c[8]  | ((uint16_t)c[9]  << 8));
    dig_P3 = (int16_t)((uint16_t)c[10] | ((uint16_t)c[11] << 8));
    dig_P4 = (int16_t)((uint16_t)c[12] | ((uint16_t)c[13] << 8));
    dig_P5 = (int16_t)((uint16_t)c[14] | ((uint16_t)c[15] << 8));
    dig_P6 = (int16_t)((uint16_t)c[16] | ((uint16_t)c[17] << 8));
    dig_P7 = (int16_t)((uint16_t)c[18] | ((uint16_t)c[19] << 8));
    dig_P8 = (int16_t)((uint16_t)c[20] | ((uint16_t)c[21] << 8));
    dig_P9 = (int16_t)((uint16_t)c[22] | ((uint16_t)c[23] << 8));

    return dig_T1 != 0 && dig_P1 != 0;
}

float BMP280Sensor::_compensateTemperature(int32_t adc_T)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    float T = (float)((t_fine * 5 + 128) >> 8) / 100.0f;
    return T;
}

float BMP280Sensor::_compensatePressure(int32_t adc_P)
{
    // Bosch BMP280 datasheet integer compensation algorithm, returns Pa.
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0.0f;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / 256.0f;
}
