/**
 * ============================================================
 *  MPU9250.cpp  —  Implementation  (v3.1)
 *  Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)
 * ============================================================
 *
 *  v3.1 changes vs v3.0:
 *   • _hasMag flag set in begin() based on AK8963 WHO_AM_I.
 *     If the board has an MPU-6500 (no mag), or the internal
 *     I2C bus is broken, _hasMag = false and ALL AK8963 init
 *     steps are skipped — no SLV0 setup, no ASA read, nothing.
 *   • readRaw() skips the EXT_SENS_DATA burst when !_hasMag.
 *   • hasMag() public accessor added.
 *   • All other logic (AHRS, calibration, NVS) unchanged.
 * ============================================================
 */

#include "MPU9250.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
MPU9250::MPU9250(uint8_t csPin, SPIClass& spi)
    : _cs(csPin), _spi(spi)
{
    memset(&cal, 0, sizeof(cal));
    cal.ax_s = cal.ay_s = cal.az_s = 1.0f;
    cal.mx_s = cal.my_s = cal.mz_s = 1.0f;
    cal.mag_asa_x = cal.mag_asa_y = cal.mag_asa_z = 1.0f;
}

// ─────────────────────────────────────────────────────────────
//  SPI helpers
// ─────────────────────────────────────────────────────────────
void MPU9250::_writeReg(uint8_t reg, uint8_t val) {
    _spi.beginTransaction(SPISettings(MPU_SPI_SLOW, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    _spi.transfer(reg & 0x7F);
    _spi.transfer(val);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
}

uint8_t MPU9250::_readReg(uint8_t reg) {
    _spi.beginTransaction(SPISettings(MPU_SPI_FAST, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    _spi.transfer(reg | 0x80);
    uint8_t val = _spi.transfer(0x00);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
    return val;
}

void MPU9250::_burstRead(uint8_t reg, uint8_t* buf, uint8_t len) {
    _spi.beginTransaction(SPISettings(MPU_SPI_FAST, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    _spi.transfer(reg | 0x80);
    for (uint8_t i = 0; i < len; i++) buf[i] = _spi.transfer(0x00);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
}

// ─────────────────────────────────────────────────────────────
//  AK8963 helpers (only called when _hasMag == true)
// ─────────────────────────────────────────────────────────────
void MPU9250::_akWrite(uint8_t akReg, uint8_t val) {
    _writeReg(0x31, AK8963_ADDR);
    _writeReg(0x32, akReg);
    _writeReg(0x33, val);
    _writeReg(0x34, 0x80);
    uint32_t t = millis();
    while (!((_readReg(0x36) >> 6) & 0x01)) {
        if (millis() - t > 10) break;
        delayMicroseconds(100);
    }
    delay(1);
}

uint8_t MPU9250::_akReadByte(uint8_t akReg) {
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80);
    _writeReg(MPU_REG_I2C_SLV0_REG,  akReg);
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x81);
    delay(10);
    return _readReg(MPU_REG_EXT_SENS_DATA);
}

void MPU9250::_akBurstRead(uint8_t akReg, uint8_t* buf, uint8_t len) {
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80);
    _writeReg(MPU_REG_I2C_SLV0_REG,  akReg);
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x80 | (len & 0x0F));
    delay(10);
    _burstRead(MPU_REG_EXT_SENS_DATA, buf, len);
}

void MPU9250::_akSetupContinuous() {
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80);
    _writeReg(MPU_REG_I2C_SLV0_REG,  AK_REG_ST1);
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x88);   // EN=1, 8 bytes
}

void MPU9250::_readMagASA() {
    _akWrite(AK_REG_CNTL1, 0x00);
    delay(10);
    _akWrite(AK_REG_CNTL1, 0x0F);
    delay(10);
    uint8_t asa[3];
    _akBurstRead(AK_REG_ASAX, asa, 3);
    cal.mag_asa_x = ((float)asa[0] - 128.0f) / 256.0f + 1.0f;
    cal.mag_asa_y = ((float)asa[1] - 128.0f) / 256.0f + 1.0f;
    cal.mag_asa_z = ((float)asa[2] - 128.0f) / 256.0f + 1.0f;
    Serial.printf("[MPU9250] Mag ASA: X=%.4f Y=%.4f Z=%.4f\n",
                  cal.mag_asa_x, cal.mag_asa_y, cal.mag_asa_z);
    _akWrite(AK_REG_CNTL1, 0x00);
    delay(10);
}

// ─────────────────────────────────────────────────────────────
//  MPU-9250 core init (accel + gyro only)
// ─────────────────────────────────────────────────────────────
bool MPU9250::_initMPU() {
    _writeReg(MPU_REG_PWR_MGMT_1, 0x80);   // hard reset
    delay(100);
    _writeReg(MPU_REG_PWR_MGMT_1, 0x01);   // wake, auto clock
    delay(10);
    _writeReg(MPU_REG_PWR_MGMT_2, 0x00);   // all axes on
    delay(10);
    _writeReg(MPU_REG_CONFIG,          0x03);  // DLPF 41 Hz
    _writeReg(MPU_REG_SMPLRT_DIV,      0x00);  // 1 kHz
    _writeReg(MPU_REG_GYRO_CONFIG,     0x08);  // ±500 °/s
    _writeReg(MPU_REG_ACCEL_CONFIG,    0x10);  // ±8 g
    _writeReg(MPU_REG_ACCEL_CONFIG2,   0x03);  // accel DLPF 41 Hz
    _writeReg(MPU_REG_INT_PIN_CFG,     0x30);
    _writeReg(MPU_REG_INT_ENABLE,      0x01);
    return true;
}

// ─────────────────────────────────────────────────────────────
//  begin()
//
//  If AK8963 WHO_AM_I does not return 0x48, _hasMag is left
//  false and all magnetometer init is skipped. The sensor
//  works as a 6-DOF device (roll/pitch accurate, yaw drifts).
//
//  Common reasons AK8963 is absent:
//    • Board has MPU-6500 chip instead of MPU-9250 (very common
//      on HiLetgo and similar cheap breakouts — WHO_AM_I=0x70)
//    • Internal I2C bus damaged
//    • I2C master not enabled before first access
// ─────────────────────────────────────────────────────────────
bool MPU9250::begin() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    delay(10);

    // Verify MPU WHO_AM_I
    uint8_t who = _readReg(MPU_REG_WHO_AM_I);
    if (who == MPU6500_WHO_AM_I_VAL) {
        Serial.println(F("[MPU9250] Detected MPU-6500 (WHO_AM_I=0x70)."));
        Serial.println(F("[MPU9250] This chip has NO magnetometer."));
        Serial.println(F("[MPU9250] AHRS will run in 6-DOF mode (roll/pitch only)."));
    } else if (who != MPU_WHO_AM_I_VAL) {
        Serial.printf("[MPU9250] WHO_AM_I=0x%02X — expected 0x71. Check wiring!\n", who);
        return false;
    } else {
        Serial.printf("[MPU9250] WHO_AM_I=0x%02X OK\n", who);
    }

    // Init accel + gyro
    _initMPU();
    delay(10);

    // ── Try to detect AK8963 ─────────────────────────────────
    // Enable I2C master so we can talk to AK8963
    _writeReg(MPU_REG_USER_CTRL,    0x20);   // I2C_MST_EN=1
    delay(10);
    _writeReg(MPU_REG_I2C_MST_CTRL, 0x0D);  // 400 kHz
    delay(10);

    uint8_t akWho = _akReadByte(AK_REG_WIA);

    if (akWho == AK_WIA_VAL) {
        Serial.printf("[MPU9250] AK8963 WHO_AM_I=0x%02X OK — magnetometer active\n", akWho);
        _hasMag = true;

        _readMagASA();

        // Continuous mode 2, 16-bit output (100 Hz)
        _akWrite(AK_REG_CNTL1, 0x16);
        delay(10);

        // Configure SLV0 for automatic 8-byte burst each MPU sample
        _akSetupContinuous();
        delay(50);
    } else {
        Serial.printf("[MPU9250] AK8963 not found (WHO_AM_I=0x%02X). ", akWho);
        Serial.println(F("Running in 6-DOF mode."));
        _hasMag   = false;
        _magValid = false;

        // Disable I2C master — not needed without mag
        _writeReg(MPU_REG_USER_CTRL, 0x00);
    }

    return true;
}

// ─────────────────────────────────────────────────────────────
//  Diagnostics
// ─────────────────────────────────────────────────────────────
bool    MPU9250::isConnected()   { return _readReg(MPU_REG_WHO_AM_I) == MPU_WHO_AM_I_VAL; }
bool    MPU9250::isMagConnected(){ return _magValid; }
bool    MPU9250::hasMag() const  { return _hasMag; }
uint8_t MPU9250::whoAmI()        { return _readReg(MPU_REG_WHO_AM_I); }

float MPU9250::readTemperature() {
    uint8_t buf[2];
    _burstRead(0x41, buf, 2);
    int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
    return (float)raw / 333.87f + 21.0f;
}

// ─────────────────────────────────────────────────────────────
//  readRaw()
// ─────────────────────────────────────────────────────────────
bool MPU9250::readRaw(MPU_RawData& out) {
    uint8_t buf[14];
    _burstRead(MPU_REG_ACCEL_XOUT_H, buf, 14);

    out.ax   = (int16_t)((buf[0]  << 8) | buf[1]);
    out.ay   = (int16_t)((buf[2]  << 8) | buf[3]);
    out.az   = (int16_t)((buf[4]  << 8) | buf[5]);
    out.temp = (int16_t)((buf[6]  << 8) | buf[7]);
    out.gx   = (int16_t)((buf[8]  << 8) | buf[9]);
    out.gy   = (int16_t)((buf[10] << 8) | buf[11]);
    out.gz   = (int16_t)((buf[12] << 8) | buf[13]);

    // Only attempt mag read if AK8963 was confirmed at boot
    if (_hasMag) {
        uint8_t mag[8];
        _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);
        out.magOk = (mag[0] & 0x01) && !(mag[7] & 0x08);
        if (out.magOk) {
            out.mx = (int16_t)((mag[2] << 8) | mag[1]);
            out.my = (int16_t)((mag[4] << 8) | mag[3]);
            out.mz = (int16_t)((mag[6] << 8) | mag[5]);
        } else {
            out.mx = out.my = out.mz = 0;
        }
    } else {
        // No mag hardware — fill zeros, AHRS will run 6-DOF
        out.magOk = false;
        out.mx = out.my = out.mz = 0;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────
//  readScaled()
// ─────────────────────────────────────────────────────────────
bool MPU9250::readScaled(MPU_SensorData& out) {
    MPU_RawData raw;
    if (!readRaw(raw)) return false;

    out.ax_g   = ((float)raw.ax * ACCEL_SCALE_8G - cal.ax_b) * cal.ax_s;
    out.ay_g   = ((float)raw.ay * ACCEL_SCALE_8G - cal.ay_b) * cal.ay_s;
    out.az_g   = ((float)raw.az * ACCEL_SCALE_8G - cal.az_b) * cal.az_s;
    out.gx_dps = (float)raw.gx * GYRO_SCALE_500DPS - cal.gx_b;
    out.gy_dps = (float)raw.gy * GYRO_SCALE_500DPS - cal.gy_b;
    out.gz_dps = (float)raw.gz * GYRO_SCALE_500DPS - cal.gz_b;

    if (raw.magOk) {
        float mx_raw = (float)raw.mx * MAG_SCALE_16BIT * cal.mag_asa_x;
        float my_raw = (float)raw.my * MAG_SCALE_16BIT * cal.mag_asa_y;
        float mz_raw = (float)raw.mz * MAG_SCALE_16BIT * cal.mag_asa_z;
        out.mx_uT = (mx_raw - cal.mx_b) * cal.mx_s;
        out.my_uT = (my_raw - cal.my_b) * cal.my_s;
        out.mz_uT = (mz_raw - cal.mz_b) * cal.mz_s;
        _last_mx = out.mx_uT;
        _last_my = out.my_uT;
        _last_mz = out.mz_uT;
        _magValid = true;
    } else {
        out.mx_uT = _last_mx;
        out.my_uT = _last_my;
        out.mz_uT = _last_mz;
    }

    out.temp_c = (float)raw.temp / 333.87f + 21.0f;
    out.ts_ms  = millis();
    return true;
}

// ─────────────────────────────────────────────────────────────
//  _invSqrt  (Quake III)
// ─────────────────────────────────────────────────────────────
float MPU9250::_invSqrt(float x) {
    union { float f; uint32_t i; } conv = { x };
    conv.i = 0x5F3759DFul - (conv.i >> 1);
    conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
    return conv.f;
}

// ─────────────────────────────────────────────────────────────
//  mahonyUpdate()
//  When _hasMag is false, _magValid is always false, so the
//  magnetometer correction block is skipped automatically and
//  the filter runs in 6-DOF mode.
// ─────────────────────────────────────────────────────────────
void MPU9250::mahonyUpdate(const MPU_SensorData& s, float dt, MPU_Attitude& att) {
    if (dt <= 0.0f || dt > 0.05f) dt = 0.001f;

    float gx = s.gx_dps * DEG2RAD;
    float gy = s.gy_dps * DEG2RAD;
    float gz = s.gz_dps * DEG2RAD;

    float ax = s.ax_g, ay = s.ay_g, az = s.az_g;
    float mx = s.mx_uT, my = s.my_uT, mz = s.mz_uT;
    float ex = 0.0f, ey = 0.0f, ez = 0.0f;

    float aNorm = sqrtf(ax*ax + ay*ay + az*az);
    if (aNorm > 0.01f) {
        float invA = _invSqrt(ax*ax + ay*ay + az*az);
        ax *= invA; ay *= invA; az *= invA;

        float vx = 2.0f * (_q1*_q3 - _q0*_q2);
        float vy = 2.0f * (_q0*_q1 + _q2*_q3);
        float vz = _q0*_q0 - _q1*_q1 - _q2*_q2 + _q3*_q3;

        ex += ay*vz - az*vy;
        ey += az*vx - ax*vz;
        ez += ax*vy - ay*vx;

        float mNorm = sqrtf(mx*mx + my*my + mz*mz);
        if (_magValid && mNorm > 0.1f) {
            float invM = _invSqrt(mx*mx + my*my + mz*mz);
            mx *= invM; my *= invM; mz *= invM;

            float hx = 2.0f*(mx*(0.5f-_q2*_q2-_q3*_q3)+my*(_q1*_q2-_q0*_q3)+mz*(_q1*_q3+_q0*_q2));
            float hy = 2.0f*(mx*(_q1*_q2+_q0*_q3)+my*(0.5f-_q1*_q1-_q3*_q3)+mz*(_q2*_q3-_q0*_q1));
            float bx = sqrtf(hx*hx + hy*hy);
            float bz = 2.0f*(mx*(_q1*_q3-_q0*_q2)+my*(_q2*_q3+_q0*_q1)+mz*(0.5f-_q1*_q1-_q2*_q2));

            float wx = 2.0f*(bx*(0.5f-_q2*_q2-_q3*_q3)+bz*(_q1*_q3-_q0*_q2));
            float wy = 2.0f*(bx*(_q1*_q2-_q0*_q3)+bz*(_q0*_q1+_q2*_q3));
            float wz = 2.0f*(bx*(_q0*_q2+_q1*_q3)+bz*(0.5f-_q1*_q1-_q2*_q2));

            ex += my*wz - mz*wy;
            ey += mz*wx - mx*wz;
            ez += mx*wy - my*wx;
        }

        _ix += mahonyKi * ex * dt;
        _iy += mahonyKi * ey * dt;
        _iz += mahonyKi * ez * dt;

        gx += mahonyKp * ex + _ix;
        gy += mahonyKp * ey + _iy;
        gz += mahonyKp * ez + _iz;
    }

    float halfDt = 0.5f * dt;
    float dq0 = (-_q1*gx - _q2*gy - _q3*gz) * halfDt;
    float dq1 = ( _q0*gx + _q2*gz - _q3*gy) * halfDt;
    float dq2 = ( _q0*gy - _q1*gz + _q3*gx) * halfDt;
    float dq3 = ( _q0*gz + _q1*gy - _q2*gx) * halfDt;

    _q0 += dq0; _q1 += dq1; _q2 += dq2; _q3 += dq3;

    float qNorm = _invSqrt(_q0*_q0 + _q1*_q1 + _q2*_q2 + _q3*_q3);
    _q0 *= qNorm; _q1 *= qNorm; _q2 *= qNorm; _q3 *= qNorm;

    att.q0 = _q0; att.q1 = _q1; att.q2 = _q2; att.q3 = _q3;

    att.roll  = RAD2DEG * atan2f(2.0f*(_q0*_q1+_q2*_q3), 1.0f-2.0f*(_q1*_q1+_q2*_q2));

    float sinP = constrain(2.0f*(_q0*_q2-_q3*_q1), -1.0f, 1.0f);
    att.pitch = RAD2DEG * asinf(sinP);

    att.yaw = RAD2DEG * atan2f(2.0f*(_q0*_q3+_q1*_q2), 1.0f-2.0f*(_q2*_q2+_q3*_q3));
    if (att.yaw < 0.0f) att.yaw += 360.0f;
}

// ─────────────────────────────────────────────────────────────
//  sampleAvg()
// ─────────────────────────────────────────────────────────────
void MPU9250::sampleAvg(int N, MPU_SensorData& out) {
    double ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
    uint8_t buf[14];
    for (int i = 0; i < N; i++) {
        _burstRead(MPU_REG_ACCEL_XOUT_H, buf, 14);
        ax += (int16_t)((buf[0] <<8)|buf[1]);
        ay += (int16_t)((buf[2] <<8)|buf[3]);
        az += (int16_t)((buf[4] <<8)|buf[5]);
        gx += (int16_t)((buf[8] <<8)|buf[9]);
        gy += (int16_t)((buf[10]<<8)|buf[11]);
        gz += (int16_t)((buf[12]<<8)|buf[13]);
        delay(2);
    }
    out.ax_g   = (float)(ax/N) * ACCEL_SCALE_8G;
    out.ay_g   = (float)(ay/N) * ACCEL_SCALE_8G;
    out.az_g   = (float)(az/N) * ACCEL_SCALE_8G;
    out.gx_dps = (float)(gx/N) * GYRO_SCALE_500DPS;
    out.gy_dps = (float)(gy/N) * GYRO_SCALE_500DPS;
    out.gz_dps = (float)(gz/N) * GYRO_SCALE_500DPS;
}

// ─────────────────────────────────────────────────────────────
//  Calibration: Gyro
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateGyro() {
    Serial.println(F("\n[CAL] Gyro — place drone flat and still."));
    Serial.println(F("[CAL] Press ENTER to start..."));
    while (!Serial.available()) delay(10);
    while (Serial.available())  Serial.read();

    Serial.println(F("[CAL] Sampling 1000 readings (~2 s)..."));
    MPU_SensorData avg;
    sampleAvg(1000, avg);
    cal.gx_b = avg.gx_dps;
    cal.gy_b = avg.gy_dps;
    cal.gz_b = avg.gz_dps;
    Serial.printf("[CAL] Gyro bias: X=%+.4f  Y=%+.4f  Z=%+.4f  deg/s\n",
                  cal.gx_b, cal.gy_b, cal.gz_b);
}

// ─────────────────────────────────────────────────────────────
//  Calibration: Accel
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateAccel() {
    // const char* labels[6] = {
    //     "+X axis UP  (right side up)",
    //     "-X axis UP  (left side up)",
    //     "+Y axis UP  (nose up)",
    //     "-Y axis UP  (nose down)",
    //     "+Z axis UP  (flat, top up)",
    //     "-Z axis UP  (inverted)"
    // };
    const char* labels[6] = {
    "+X axis UP  (nose up)",
    "-X axis UP  (nose down)",
    "+Y axis UP  (left side up)",
    "-Y axis UP  (right side up)",
    "+Z axis UP  (flat, top up)",
    "-Z axis UP  (inverted)"
    };
    float readings[6][3];
    Serial.println(F("\n[CAL] Accel 6-position calibration."));
    Serial.println(F("[CAL] For each orientation, place drone then press ENTER.\n"));

    for (int pos = 0; pos < 6; pos++) {
        Serial.printf("[CAL] Position %d/6: %s\n[CAL] Press ENTER when ready...\n",
                      pos+1, labels[pos]);
        while (!Serial.available()) delay(10);
        while (Serial.available())  Serial.read();
        MPU_SensorData avg;
        sampleAvg(500, avg);
        readings[pos][0] = avg.ax_g;
        readings[pos][1] = avg.ay_g;
        readings[pos][2] = avg.az_g;
        Serial.printf("[CAL]  Got: ax=%+.4f  ay=%+.4f  az=%+.4f\n",
                      avg.ax_g, avg.ay_g, avg.az_g);
    }
    cal.ax_b = (readings[0][0]+readings[1][0])/2.0f;
    cal.ay_b = (readings[2][1]+readings[3][1])/2.0f;
    cal.az_b = (readings[4][2]+readings[5][2])/2.0f;
    float hrX=(readings[0][0]-readings[1][0])/2.0f;
    float hrY=(readings[2][1]-readings[3][1])/2.0f;
    float hrZ=(readings[4][2]-readings[5][2])/2.0f;
    cal.ax_s = (fabsf(hrX)>0.01f) ? 1.0f/hrX : 1.0f;
    cal.ay_s = (fabsf(hrY)>0.01f) ? 1.0f/hrY : 1.0f;
    cal.az_s = (fabsf(hrZ)>0.01f) ? 1.0f/hrZ : 1.0f;
    Serial.printf("[CAL] Accel bias:  X=%+.4f  Y=%+.4f  Z=%+.4f  g\n",
                  cal.ax_b, cal.ay_b, cal.az_b);
    Serial.printf("[CAL] Accel scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  cal.ax_s, cal.ay_s, cal.az_s);
}

// ─────────────────────────────────────────────────────────────
//  Calibration: Mag
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateMag(uint32_t durationMs) {
    if (!_hasMag) {
        Serial.println(F("[CAL] Skipping mag calibration — no AK8963 detected."));
        return;
    }
    Serial.println(F("\n[CAL] Mag calibration — rotate slowly through ALL axes."));
    Serial.printf("[CAL] Duration: %lu s. Press ENTER to start...\n", durationMs/1000);
    while (!Serial.available()) delay(10);
    while (Serial.available())  Serial.read();

    float xmin= 1e9f, ymin= 1e9f, zmin= 1e9f;
    float xmax=-1e9f, ymax=-1e9f, zmax=-1e9f;
    uint32_t end_ms   = millis() + durationMs;
    uint32_t lastPrint = 0;

    while (millis() < end_ms) {
        uint8_t mag[8];
        _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);
        bool ok = (mag[0]&0x01) && !(mag[7]&0x08);
        if (ok) {
            float mx=(int16_t)((mag[2]<<8)|mag[1])*MAG_SCALE_16BIT*cal.mag_asa_x;
            float my=(int16_t)((mag[4]<<8)|mag[3])*MAG_SCALE_16BIT*cal.mag_asa_y;
            float mz=(int16_t)((mag[6]<<8)|mag[5])*MAG_SCALE_16BIT*cal.mag_asa_z;
            if(mx<xmin)xmin=mx; if(mx>xmax)xmax=mx;
            if(my<ymin)ymin=my; if(my>ymax)ymax=my;
            if(mz<zmin)zmin=mz; if(mz>zmax)zmax=mz;
        }
        if (millis()-lastPrint>2000) {
            Serial.printf("[CAL] %lu s remaining...\n",(end_ms-millis())/1000);
            lastPrint=millis();
        }
        delay(10);
    }
    cal.mx_b=(xmax+xmin)/2.0f;
    cal.my_b=(ymax+ymin)/2.0f;
    cal.mz_b=(zmax+zmin)/2.0f;
    float avg=((xmax-xmin)+(ymax-ymin)+(zmax-zmin))/3.0f;
    cal.mx_s=(xmax-xmin)>0.1f?avg/(xmax-xmin):1.0f;
    cal.my_s=(ymax-ymin)>0.1f?avg/(ymax-ymin):1.0f;
    cal.mz_s=(zmax-zmin)>0.1f?avg/(zmax-zmin):1.0f;
    Serial.printf("[CAL] Mag bias:  X=%+.2f  Y=%+.2f  Z=%+.2f  uT\n",
                  cal.mx_b, cal.my_b, cal.mz_b);
    Serial.printf("[CAL] Mag scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  cal.mx_s, cal.my_s, cal.mz_s);
}

// ─────────────────────────────────────────────────────────────
//  NVS
// ─────────────────────────────────────────────────────────────
void MPU9250::saveCalibration(const char* ns) {
    Preferences p; p.begin(ns, false);
    p.putFloat("gx_b",cal.gx_b); p.putFloat("gy_b",cal.gy_b); p.putFloat("gz_b",cal.gz_b);
    p.putFloat("ax_b",cal.ax_b); p.putFloat("ay_b",cal.ay_b); p.putFloat("az_b",cal.az_b);
    p.putFloat("ax_s",cal.ax_s); p.putFloat("ay_s",cal.ay_s); p.putFloat("az_s",cal.az_s);
    p.putFloat("mx_b",cal.mx_b); p.putFloat("my_b",cal.my_b); p.putFloat("mz_b",cal.mz_b);
    p.putFloat("mx_s",cal.mx_s); p.putFloat("my_s",cal.my_s); p.putFloat("mz_s",cal.mz_s);
    p.putBool("valid",true); p.end();
    cal.valid = true;
    Serial.println(F("[NVS] Calibration saved."));
}

bool MPU9250::loadCalibration(const char* ns) {
    Preferences p; p.begin(ns, true);
    bool valid = p.getBool("valid", false);
    if (valid) {
        cal.gx_b=p.getFloat("gx_b",0); cal.gy_b=p.getFloat("gy_b",0); cal.gz_b=p.getFloat("gz_b",0);
        cal.ax_b=p.getFloat("ax_b",0); cal.ay_b=p.getFloat("ay_b",0); cal.az_b=p.getFloat("az_b",0);
        cal.ax_s=p.getFloat("ax_s",1); cal.ay_s=p.getFloat("ay_s",1); cal.az_s=p.getFloat("az_s",1);
        cal.mx_b=p.getFloat("mx_b",0); cal.my_b=p.getFloat("my_b",0); cal.mz_b=p.getFloat("mz_b",0);
        cal.mx_s=p.getFloat("mx_s",1); cal.my_s=p.getFloat("my_s",1); cal.mz_s=p.getFloat("mz_s",1);
        cal.valid = true;
    }
    p.end();
    return valid;
}

void MPU9250::eraseCalibration(const char* ns) {
    Preferences p; p.begin(ns,false); p.clear(); p.end();
    memset(&cal, 0, sizeof(cal));
    cal.ax_s=cal.ay_s=cal.az_s=1.0f;
    cal.mx_s=cal.my_s=cal.mz_s=1.0f;
    cal.mag_asa_x=cal.mag_asa_y=cal.mag_asa_z=1.0f;
    Serial.println(F("[NVS] Calibration erased."));
}

void MPU9250::printCalibration() {
    Serial.println(F("\n--- MPU9250 Calibration ---"));
    Serial.printf("  Gyro bias:   X=%+.4f  Y=%+.4f  Z=%+.4f  deg/s\n", cal.gx_b,cal.gy_b,cal.gz_b);
    Serial.printf("  Accel bias:  X=%+.4f  Y=%+.4f  Z=%+.4f  g\n",     cal.ax_b,cal.ay_b,cal.az_b);
    Serial.printf("  Accel scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",        cal.ax_s,cal.ay_s,cal.az_s);
    if (_hasMag) {
        Serial.printf("  Mag bias:    X=%+.2f  Y=%+.2f  Z=%+.2f  uT\n",cal.mx_b,cal.my_b,cal.mz_b);
        Serial.printf("  Mag scale:   X=%+.4f  Y=%+.4f  Z=%+.4f\n",    cal.mx_s,cal.my_s,cal.mz_s);
        Serial.printf("  Mag ASA:     X=%.4f  Y=%.4f  Z=%.4f\n",       cal.mag_asa_x,cal.mag_asa_y,cal.mag_asa_z);
    } else {
        Serial.println(F("  Mag: NOT PRESENT (6-DOF mode)"));
    }
    Serial.printf("  Valid: %s\n", cal.valid?"YES":"NO");
}

// ─────────────────────────────────────────────────────────────
//  diagMag()
// ─────────────────────────────────────────────────────────────
void MPU9250::diagMag() {
    if (!_hasMag) {
        Serial.println(F("[DIAG] No AK8963 detected — skipping mag diagnostic."));
        Serial.println(F("[DIAG] Your board likely has an MPU-6500 (no magnetometer)."));
        Serial.println(F("[DIAG] WHO_AM_I=0x70 = MPU-6500,  0x71 = MPU-9250."));
        Serial.printf(  "[DIAG] Your chip reported WHO_AM_I=0x%02X\n", whoAmI());
        return;
    }
    Serial.println(F("\n[DIAG] Raw EXT_SENS_DATA (10 samples):"));
    Serial.println(F("  [  ST1  ] [  HXL  HXH  HYL  HYH  HZL  HZH  ] [  ST2  ]"));
    for (int i = 0; i < 10; i++) {
        uint8_t mag[8];
        _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);
        bool drdy=(mag[0]&0x01), hofl=(mag[7]&0x08);
        int16_t rx=(int16_t)((mag[2]<<8)|mag[1]);
        int16_t ry=(int16_t)((mag[4]<<8)|mag[3]);
        int16_t rz=(int16_t)((mag[6]<<8)|mag[5]);
        float mx=rx*MAG_SCALE_16BIT*cal.mag_asa_x;
        float my=ry*MAG_SCALE_16BIT*cal.mag_asa_y;
        float mz=rz*MAG_SCALE_16BIT*cal.mag_asa_z;
        Serial.printf("  [0x%02X %s] [%6d %6d %6d]  ST2=0x%02X %s  -> %.1f %.1f %.1f uT\n",
                      mag[0], drdy?"DRDY":"    ",
                      rx, ry, rz,
                      mag[7], hofl?"HOFL":"    ",
                      mx, my, mz);
        delay(15);
    }
    Serial.println(F("\n[DIAG] SLV0 config registers:"));
    Serial.printf("  SLV0_ADDR(0x25)=0x%02X (should be 0x8C)\n", _readReg(0x25));
    Serial.printf("  SLV0_REG (0x26)=0x%02X (should be 0x02)\n", _readReg(0x26));
    Serial.printf("  SLV0_CTRL(0x27)=0x%02X (should be 0x88)\n", _readReg(0x27));
    Serial.printf("  USER_CTRL(0x6A)=0x%02X (bit5 should be 1)\n",_readReg(0x6A));
}
