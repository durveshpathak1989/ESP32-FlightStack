/**
 * ============================================================
 *  MPU9250.cpp  —  Implementation  (v3.0)
 *  Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)
 * ============================================================
 *
 *  Key fix from previous version:
 *   • Mahony filter quaternion state (_q0-_q3) now lives in
 *     the class — not scattered across .ino globals — so it
 *     accumulates correctly and produces non-zero RPY.
 *   • Accel normalisation guard prevents NaN when sensor reads
 *     near-zero during reset.
 *   • Mag is only applied to AHRS when magOk == true AND
 *     mag norm is non-zero; otherwise filter falls back to
 *     6-DOF (no yaw drift from bad mag data).
 *   • dt clamped to [0.0001, 0.05] s to prevent filter blow-up
 *     on first call or after a delay.
 * ============================================================
 */

#include "MPU9250.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
MPU9250::MPU9250(uint8_t csPin, SPIClass& spi)
    : _cs(csPin), _spi(spi)
{
    // Initialise cal to safe defaults
    memset(&cal, 0, sizeof(cal));
    cal.ax_s = cal.ay_s = cal.az_s = 1.0f;
    cal.mx_s = cal.my_s = cal.mz_s = 1.0f;
    cal.mag_asa_x = cal.mag_asa_y = cal.mag_asa_z = 1.0f;
}

// ─────────────────────────────────────────────────────────────
//  SPI helpers — private
// ─────────────────────────────────────────────────────────────
void MPU9250::_writeReg(uint8_t reg, uint8_t val) {
    _spi.beginTransaction(SPISettings(MPU_SPI_SLOW, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    _spi.transfer(reg & 0x7F);   // bit7=0 → write
    _spi.transfer(val);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
}

uint8_t MPU9250::_readReg(uint8_t reg) {
    _spi.beginTransaction(SPISettings(MPU_SPI_FAST, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    _spi.transfer(reg | 0x80);   // bit7=1 → read
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
//  AK8963 low-level helpers — ALL use SLV0 only
//
//  The MPU-9250 I2C master has two slave channels:
//    SLV0  read  channel  — triggered every sample, result → EXT_SENS_DATA
//    SLV4  write channel  — one-shot, has its own DO+CTRL pair at 0x33-0x35
//
//  Using SLV1 for writes is valid but SLV1_DO is at 0x64 ONLY after
//  the I2C master is enabled AND the SLV1 transfer completes.
//  To avoid all ordering hazards we use SLV4 for one-shot writes
//  (dedicated write slave, separate from the read pipeline).
// ─────────────────────────────────────────────────────────────

// One-shot write to AK8963 via SLV4 (dedicated write slave)
// SLV4 registers: ADDR=0x31, REG=0x32, DO=0x33, CTRL=0x34, DI=0x35
void MPU9250::_akWrite(uint8_t akReg, uint8_t val) {
    _writeReg(0x31, AK8963_ADDR);        // SLV4_ADDR: write mode (bit7=0)
    _writeReg(0x32, akReg);              // SLV4_REG
    _writeReg(0x33, val);               // SLV4_DO
    _writeReg(0x34, 0x80);              // SLV4_CTRL: enable one-shot (EN=1, INT=0)
    // Wait for SLV4 transaction to complete (I2C_MST_STATUS bit6 = I2C_SLV4_DONE)
    uint32_t t = millis();
    while (!((_readReg(0x36) >> 6) & 0x01)) {   // 0x36 = I2C_MST_STATUS
        if (millis() - t > 10) break;           // 10 ms timeout
        delayMicroseconds(100);
    }
    delay(1);
}

// One-shot read from AK8963 via SLV0 (temporarily, then restore)
uint8_t MPU9250::_akReadByte(uint8_t akReg) {
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80); // read mode
    _writeReg(MPU_REG_I2C_SLV0_REG,  akReg);
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x81);               // enable, 1 byte
    delay(10);                                              // one I2C transaction
    return _readReg(MPU_REG_EXT_SENS_DATA);
}

// Burst read N bytes from AK8963 via SLV0 (temporarily)
void MPU9250::_akBurstRead(uint8_t akReg, uint8_t* buf, uint8_t len) {
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80);
    _writeReg(MPU_REG_I2C_SLV0_REG,  akReg);
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x80 | (len & 0x0F));
    delay(10);
    _burstRead(MPU_REG_EXT_SENS_DATA, buf, len);
}

// Configure SLV0 for continuous 7-byte mag burst every sample:
//   reads ST1(1) + HXL..HZH(6) + ST2(1) = 8 bytes total
//   but we only need 7: ST1 + 6 data — ST2 is needed to unlatch AK8963
void MPU9250::_akSetupContinuous() {
    // Read ST1 through ST2 (8 bytes: ST1, HXL, HXH, HYL, HYH, HZL, HZH, ST2)
    _writeReg(MPU_REG_I2C_SLV0_ADDR, AK8963_ADDR | 0x80); // read
    _writeReg(MPU_REG_I2C_SLV0_REG,  AK_REG_ST1);         // start at ST1 (0x02)
    _writeReg(MPU_REG_I2C_SLV0_CTRL, 0x88);               // enable, 8 bytes
    // 0x88 = 1000_1000: EN=1, BYTE_SW=0, REG_DIS=0, GRP=0, LEN=8
}

// ─────────────────────────────────────────────────────────────
//  Read AK8963 factory sensitivity adjustment from Fuse ROM
// ─────────────────────────────────────────────────────────────
void MPU9250::_readMagASA() {
    // AK8963 must be in power-down before mode change
    _akWrite(AK_REG_CNTL1, 0x00);   // power down
    delay(10);
    _akWrite(AK_REG_CNTL1, 0x0F);   // Fuse ROM access mode
    delay(10);

    uint8_t asa[3];
    _akBurstRead(AK_REG_ASAX, asa, 3);

    // Sensitivity adjustment formula: Hadj = H * ((ASA-128)/256 + 1)
    cal.mag_asa_x = ((float)asa[0] - 128.0f) / 256.0f + 1.0f;
    cal.mag_asa_y = ((float)asa[1] - 128.0f) / 256.0f + 1.0f;
    cal.mag_asa_z = ((float)asa[2] - 128.0f) / 256.0f + 1.0f;

    Serial.printf("[MPU9250] Mag ASA: X=%.4f Y=%.4f Z=%.4f\n",
                  cal.mag_asa_x, cal.mag_asa_y, cal.mag_asa_z);

    _akWrite(AK_REG_CNTL1, 0x00);   // power down before mode change
    delay(10);
}

// ─────────────────────────────────────────────────────────────
//  MPU-9250 core initialisation
// ─────────────────────────────────────────────────────────────
bool MPU9250::_initMPU() {
    // Hard reset — clears all registers
    _writeReg(MPU_REG_PWR_MGMT_1, 0x80);
    delay(100);

    // Wake, auto-select best clock source
    _writeReg(MPU_REG_PWR_MGMT_1, 0x01);
    delay(10);

    // Enable accel + gyro (all axes)
    _writeReg(MPU_REG_PWR_MGMT_2, 0x00);
    delay(10);

    // DLPF 41 Hz (smooth enough for attitude, fast enough for 1 kHz)
    _writeReg(MPU_REG_CONFIG, 0x03);

    // Sample rate = gyro_rate / (1 + SMPLRT_DIV) = 1000 / 1 = 1 kHz
    _writeReg(MPU_REG_SMPLRT_DIV, 0x00);

    // Gyro ±500 °/s, Fchoice_b = 00 → use DLPF
    _writeReg(MPU_REG_GYRO_CONFIG, 0x08);

    // Accel ±8 g
    _writeReg(MPU_REG_ACCEL_CONFIG, 0x10);

    // Accel DLPF 41 Hz
    _writeReg(MPU_REG_ACCEL_CONFIG2, 0x03);

    // ── I2C master must be enabled BEFORE any AK8963 access ──
    // Disable I2C slave interface on MPU (we're using SPI)
    // and enable the internal I2C master for AK8963
    _writeReg(MPU_REG_USER_CTRL, 0x20);   // I2C_MST_EN=1, I2C_IF_DIS=0
    delay(10);

    // I2C master clock 400 kHz
    _writeReg(MPU_REG_I2C_MST_CTRL, 0x0D);
    delay(10);

    // INT pin: active-high, push-pull, 50 µs pulse, any-read clear
    _writeReg(MPU_REG_INT_PIN_CFG, 0x30);

    // Data-ready interrupt enable
    _writeReg(MPU_REG_INT_ENABLE, 0x01);

    return true;
}

// ─────────────────────────────────────────────────────────────
//  Public: begin()
//
//  Correct AK8963 init sequence (verified against datasheet):
//   1. Init MPU core (enables I2C master first)
//   2. Verify AK8963 WIA via SLV0 one-shot read
//   3. Read factory ASA from Fuse ROM (power-down → Fuse → power-down)
//   4. Put AK8963 in continuous mode 2, 16-bit
//   5. Configure SLV0 for automatic 8-byte burst each sample
//   NO diagnostic reads after step 5 — they would break SLV0 config
// ─────────────────────────────────────────────────────────────
bool MPU9250::begin() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    delay(10);

    // Step 1: verify MPU-9250
    uint8_t who = _readReg(MPU_REG_WHO_AM_I);
    if (who != MPU_WHO_AM_I_VAL) {
        Serial.printf("[MPU9250] WHO_AM_I=0x%02X expected 0x71 — check wiring!\n", who);
        return false;
    }
    Serial.printf("[MPU9250] WHO_AM_I=0x%02X OK\n", who);

    // Step 2: init MPU core (enables I2C master)
    _initMPU();
    delay(10);

    // Step 3: verify AK8963 (one-shot SLV0 read — OK before SLV0 burst is running)
    uint8_t akWho = _akReadByte(AK_REG_WIA);
    if (akWho == AK_WIA_VAL) {
        Serial.printf("[MPU9250] AK8963 WHO_AM_I=0x%02X OK\n", akWho);
    } else {
        Serial.printf("[MPU9250] AK8963 WHO_AM_I=0x%02X (expected 0x48) — mag disabled\n", akWho);
        // Don't return false — accel/gyro still work
    }

    // Step 4: read factory ASA (handles power-down and mode changes internally)
    _readMagASA();

    // Step 5: put AK8963 in continuous measurement mode 2, 16-bit output
    //   0x16 = 0001_0110: MODE=0110 (cont.2=100Hz), BIT=1 (16-bit)
    _akWrite(AK_REG_CNTL1, 0x16);
    delay(10);

    // Step 6: configure SLV0 to auto-read 8 bytes (ST1+6data+ST2) every sample
    // This is the LAST SLV0 write — nothing should change SLV0 after this
    _akSetupContinuous();
    delay(50);  // allow at least one AK8963 sample to arrive (100 Hz = 10 ms)

    _magValid = false;
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Public: diagnostics
// ─────────────────────────────────────────────────────────────
bool MPU9250::isConnected() {
    return _readReg(MPU_REG_WHO_AM_I) == MPU_WHO_AM_I_VAL;
}

bool MPU9250::isMagConnected() {
    // Do NOT call _akReadByte after begin() — it overwrites SLV0 config
    // and breaks the continuous mag burst. Report based on received data.
    return _magValid;
}

uint8_t MPU9250::whoAmI() {
    return _readReg(MPU_REG_WHO_AM_I);
}

float MPU9250::readTemperature() {
    uint8_t buf[2];
    _burstRead(0x41, buf, 2);   // TEMP_OUT_H / TEMP_OUT_L
    int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
    return (float)raw / 333.87f + 21.0f;
}

// ─────────────────────────────────────────────────────────────
//  Public: readRaw()
// ─────────────────────────────────────────────────────────────
bool MPU9250::readRaw(MPU_RawData& out) {
    // 14-byte burst: accel(6) + temp(2) + gyro(6)
    uint8_t buf[14];
    _burstRead(MPU_REG_ACCEL_XOUT_H, buf, 14);

    out.ax   = (int16_t)((buf[0]  << 8) | buf[1]);
    out.ay   = (int16_t)((buf[2]  << 8) | buf[3]);
    out.az   = (int16_t)((buf[4]  << 8) | buf[5]);
    out.temp = (int16_t)((buf[6]  << 8) | buf[7]);
    out.gx   = (int16_t)((buf[8]  << 8) | buf[9]);
    out.gy   = (int16_t)((buf[10] << 8) | buf[11]);
    out.gz   = (int16_t)((buf[12] << 8) | buf[13]);

    // 8-byte mag from EXT_SENS_DATA: ST1(0) HXL(1) HXH(2) HYL(3) HYH(4) HZL(5) HZH(6) ST2(7)
    // SLV0 was configured to read 8 bytes starting at AK_REG_ST1
    uint8_t mag[8];
    _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);

    // ST1 bit0 = DRDY (data ready), ST2 bit3 = HOFL (overflow)
    // ST2 MUST be read to unlatch the AK8963 data registers for the next sample
    out.magOk = (mag[0] & 0x01) && !(mag[7] & 0x08);
    if (out.magOk) {
        // AK8963 data is little-endian: low byte first
        out.mx = (int16_t)((mag[2] << 8) | mag[1]);   // HXH:HXL
        out.my = (int16_t)((mag[4] << 8) | mag[3]);   // HYH:HYL
        out.mz = (int16_t)((mag[6] << 8) | mag[5]);   // HZH:HZL
    } else {
        out.mx = out.my = out.mz = 0;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────
//  Public: readScaled()
// ─────────────────────────────────────────────────────────────
bool MPU9250::readScaled(MPU_SensorData& out) {
    MPU_RawData raw;
    if (!readRaw(raw)) return false;

    // Accel: subtract bias, apply scale factor, convert to g
    out.ax_g  = ((float)raw.ax * ACCEL_SCALE_8G - cal.ax_b) * cal.ax_s;
    out.ay_g  = ((float)raw.ay * ACCEL_SCALE_8G - cal.ay_b) * cal.ay_s;
    out.az_g  = ((float)raw.az * ACCEL_SCALE_8G - cal.az_b) * cal.az_s;

    // Gyro: subtract bias, convert to °/s
    out.gx_dps = (float)raw.gx * GYRO_SCALE_500DPS - cal.gx_b;
    out.gy_dps = (float)raw.gy * GYRO_SCALE_500DPS - cal.gy_b;
    out.gz_dps = (float)raw.gz * GYRO_SCALE_500DPS - cal.gz_b;

    // Mag: apply factory ASA, subtract hard-iron, apply soft-iron scale
    if (raw.magOk) {
        float mx_raw = (float)raw.mx * MAG_SCALE_16BIT * cal.mag_asa_x;
        float my_raw = (float)raw.my * MAG_SCALE_16BIT * cal.mag_asa_y;
        float mz_raw = (float)raw.mz * MAG_SCALE_16BIT * cal.mag_asa_z;
        out.mx_uT = (mx_raw - cal.mx_b) * cal.mx_s;
        out.my_uT = (my_raw - cal.my_b) * cal.my_s;
        out.mz_uT = (mz_raw - cal.mz_b) * cal.mz_s;
        // Cache for AHRS when new mag data isn't ready
        _last_mx = out.mx_uT;
        _last_my = out.my_uT;
        _last_mz = out.mz_uT;
        _magValid = true;
    } else {
        // Use cached values so AHRS isn't starved
        out.mx_uT = _last_mx;
        out.my_uT = _last_my;
        out.mz_uT = _last_mz;
    }

    out.temp_c = (float)raw.temp / 333.87f + 21.0f;
    out.ts_ms  = millis();

    return true;
}

// ─────────────────────────────────────────────────────────────
//  Fast inverse square root (Quake III style)
// ─────────────────────────────────────────────────────────────
float MPU9250::_invSqrt(float x) {
    union { float f; uint32_t i; } conv = { x };
    conv.i = 0x5F3759DFul - (conv.i >> 1);
    conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
    return conv.f;
}

// ─────────────────────────────────────────────────────────────
//  Public: mahonyUpdate()
//
//  This is the ONLY place that modifies _q0.._q3.
//  State accumulates across calls — that is what makes
//  roll/pitch/yaw non-zero after the first few iterations.
//
//  Input units:
//    s.gx/gy/gz_dps  — degrees per second (converted to rad/s here)
//    s.ax/ay/az_g    — g
//    s.mx/my/mz_uT   — µT (magnitude doesn't matter, only direction)
//    dt              — seconds since last call
// ─────────────────────────────────────────────────────────────
void MPU9250::mahonyUpdate(const MPU_SensorData& s, float dt, MPU_Attitude& att) {
    // Clamp dt to prevent filter blow-up
    if (dt <= 0.0f || dt > 0.05f) dt = 0.001f;

    // Convert gyro to rad/s
    float gx = s.gx_dps * DEG2RAD;
    float gy = s.gy_dps * DEG2RAD;
    float gz = s.gz_dps * DEG2RAD;

    float ax = s.ax_g;
    float ay = s.ay_g;
    float az = s.az_g;

    float mx = s.mx_uT;
    float my = s.my_uT;
    float mz = s.mz_uT;

    float ex = 0.0f, ey = 0.0f, ez = 0.0f;

    // ── Accelerometer correction ─────────────────────────────
    float aNorm = sqrtf(ax*ax + ay*ay + az*az);
    if (aNorm > 0.01f) {  // Guard against near-zero (free-fall / reset)
        float invA = _invSqrt(ax*ax + ay*ay + az*az);
        ax *= invA; ay *= invA; az *= invA;

        // Estimated gravity direction from current quaternion
        float vx = 2.0f * (_q1*_q3 - _q0*_q2);
        float vy = 2.0f * (_q0*_q1 + _q2*_q3);
        float vz = _q0*_q0 - _q1*_q1 - _q2*_q2 + _q3*_q3;

        // Cross product: measured × estimated
        ex += ay*vz - az*vy;
        ey += az*vx - ax*vz;
        ez += ax*vy - ay*vx;

        // ── Magnetometer correction (9-DOF) ─────────────────
        float mNorm = sqrtf(mx*mx + my*my + mz*mz);
        if (_magValid && mNorm > 0.1f) {
            float invM = _invSqrt(mx*mx + my*my + mz*mz);
            mx *= invM; my *= invM; mz *= invM;

            // Reference direction of Earth magnetic field
            // (project mag into horizontal plane using current quaternion)
            float hx = 2.0f*(mx*(0.5f - _q2*_q2 - _q3*_q3)
                           + my*(_q1*_q2 - _q0*_q3)
                           + mz*(_q1*_q3 + _q0*_q2));
            float hy = 2.0f*(mx*(_q1*_q2 + _q0*_q3)
                           + my*(0.5f - _q1*_q1 - _q3*_q3)
                           + mz*(_q2*_q3 - _q0*_q1));
            float bx = sqrtf(hx*hx + hy*hy);   // horizontal component
            float bz = 2.0f*(mx*(_q1*_q3 - _q0*_q2)
                           + my*(_q2*_q3 + _q0*_q1)
                           + mz*(0.5f - _q1*_q1 - _q2*_q2));

            // Estimated magnetic field direction from quaternion
            float wx = 2.0f*(bx*(0.5f - _q2*_q2 - _q3*_q3)
                           + bz*(_q1*_q3 - _q0*_q2));
            float wy = 2.0f*(bx*(_q1*_q2 - _q0*_q3)
                           + bz*(_q0*_q1 + _q2*_q3));
            float wz = 2.0f*(bx*(_q0*_q2 + _q1*_q3)
                           + bz*(0.5f - _q1*_q1 - _q2*_q2));

            ex += my*wz - mz*wy;
            ey += mz*wx - mx*wz;
            ez += mx*wy - my*wx;
        }

        // ── Integral feedback ────────────────────────────────
        _ix += mahonyKi * ex * dt;
        _iy += mahonyKi * ey * dt;
        _iz += mahonyKi * ez * dt;

        // ── Apply proportional + integral correction to gyro ─
        gx += mahonyKp * ex + _ix;
        gy += mahonyKp * ey + _iy;
        gz += mahonyKp * ez + _iz;
    }

    // ── Integrate quaternion rate ────────────────────────────
    float halfDt = 0.5f * dt;
    float dq0 = (-_q1*gx - _q2*gy - _q3*gz) * halfDt;
    float dq1 = ( _q0*gx + _q2*gz - _q3*gy) * halfDt;
    float dq2 = ( _q0*gy - _q1*gz + _q3*gx) * halfDt;
    float dq3 = ( _q0*gz + _q1*gy - _q2*gx) * halfDt;

    _q0 += dq0; _q1 += dq1; _q2 += dq2; _q3 += dq3;

    // ── Normalise quaternion ─────────────────────────────────
    float qNorm = _invSqrt(_q0*_q0 + _q1*_q1 + _q2*_q2 + _q3*_q3);
    _q0 *= qNorm; _q1 *= qNorm; _q2 *= qNorm; _q3 *= qNorm;

    // ── Quaternion → Euler angles ────────────────────────────
    att.q0 = _q0; att.q1 = _q1; att.q2 = _q2; att.q3 = _q3;

    att.roll  = RAD2DEG * atan2f(2.0f*(_q0*_q1 + _q2*_q3),
                                 1.0f - 2.0f*(_q1*_q1 + _q2*_q2));

    float sinP = 2.0f * (_q0*_q2 - _q3*_q1);
    sinP = constrain(sinP, -1.0f, 1.0f);   // clamp for asinf stability
    att.pitch = RAD2DEG * asinf(sinP);

    att.yaw   = RAD2DEG * atan2f(2.0f*(_q0*_q3 + _q1*_q2),
                                 1.0f - 2.0f*(_q2*_q2 + _q3*_q3));
    if (att.yaw < 0.0f) att.yaw += 360.0f;  // normalise to 0–360°
}

// ─────────────────────────────────────────────────────────────
//  Public: sampleAvg() — read N raw samples, return average
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
//  Calibration: Gyro — 1000-sample static bias
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateGyro() {
    Serial.println(F("\n[CAL] Gyro — place drone flat and still."));
    Serial.println(F("[CAL] Press ENTER to start..."));
    while (!Serial.available()) delay(10);
    while (Serial.available()) Serial.read();

    Serial.println(F("[CAL] Sampling 1000 readings (2 s)..."));
    MPU_SensorData avg;
    sampleAvg(1000, avg);

    cal.gx_b = avg.gx_dps;
    cal.gy_b = avg.gy_dps;
    cal.gz_b = avg.gz_dps;

    Serial.printf("[CAL] Gyro bias: X=%+.4f  Y=%+.4f  Z=%+.4f  °/s\n",
                  cal.gx_b, cal.gy_b, cal.gz_b);
}

// ─────────────────────────────────────────────────────────────
//  Calibration: Accel — 6-position bias + scale
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateAccel() {
    const char* labels[6] = {
        "+X axis UP  (right side up)",
        "-X axis UP  (left side up)",
        "+Y axis UP  (nose up)",
        "-Y axis UP  (nose down)",
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
        while (Serial.available()) Serial.read();

        MPU_SensorData avg;
        sampleAvg(500, avg);
        readings[pos][0] = avg.ax_g;
        readings[pos][1] = avg.ay_g;
        readings[pos][2] = avg.az_g;
        Serial.printf("[CAL]  Got: ax=%+.4f  ay=%+.4f  az=%+.4f\n",
                      avg.ax_g, avg.ay_g, avg.az_g);
    }

    // Bias = midpoint of opposite pair (removes offset)
    cal.ax_b = (readings[0][0] + readings[1][0]) / 2.0f;
    cal.ay_b = (readings[2][1] + readings[3][1]) / 2.0f;
    cal.az_b = (readings[4][2] + readings[5][2]) / 2.0f;

    // Scale = 1g / half-range (corrects gain)
    float hrX = (readings[0][0] - readings[1][0]) / 2.0f;
    float hrY = (readings[2][1] - readings[3][1]) / 2.0f;
    float hrZ = (readings[4][2] - readings[5][2]) / 2.0f;
    cal.ax_s = (fabsf(hrX) > 0.01f) ? 1.0f / hrX : 1.0f;
    cal.ay_s = (fabsf(hrY) > 0.01f) ? 1.0f / hrY : 1.0f;
    cal.az_s = (fabsf(hrZ) > 0.01f) ? 1.0f / hrZ : 1.0f;

    Serial.printf("[CAL] Accel bias: X=%+.4f  Y=%+.4f  Z=%+.4f  g\n",
                  cal.ax_b, cal.ay_b, cal.az_b);
    Serial.printf("[CAL] Accel scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  cal.ax_s, cal.ay_s, cal.az_s);
}

// ─────────────────────────────────────────────────────────────
//  Calibration: Mag — hard-iron + soft-iron (min/max sphere)
// ─────────────────────────────────────────────────────────────
void MPU9250::calibrateMag(uint32_t durationMs) {
    Serial.println(F("\n[CAL] Mag calibration — rotate slowly through ALL axes."));
    Serial.printf("[CAL] Duration: %lu s. Press ENTER to start...\n", durationMs/1000);
    while (!Serial.available()) delay(10);
    while (Serial.available()) Serial.read();

    float xmin= 1e9f, ymin= 1e9f, zmin= 1e9f;
    float xmax=-1e9f, ymax=-1e9f, zmax=-1e9f;
    uint32_t end_ms = millis() + durationMs;
    uint32_t lastPrint = 0;

    while (millis() < end_ms) {
        uint8_t mag[8];
        _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);
        bool ok = (mag[0] & 0x01) && !(mag[7] & 0x08);
        if (ok) {
            float mx = (int16_t)((mag[2]<<8)|mag[1]) * MAG_SCALE_16BIT * cal.mag_asa_x;
            float my = (int16_t)((mag[4]<<8)|mag[3]) * MAG_SCALE_16BIT * cal.mag_asa_y;
            float mz = (int16_t)((mag[6]<<8)|mag[5]) * MAG_SCALE_16BIT * cal.mag_asa_z;
            if (mx < xmin) xmin=mx; if (mx > xmax) xmax=mx;
            if (my < ymin) ymin=my; if (my > ymax) ymax=my;
            if (mz < zmin) zmin=mz; if (mz > zmax) zmax=mz;
        }
        if (millis() - lastPrint > 2000) {
            Serial.printf("[CAL] %lu s remaining...\n", (end_ms - millis())/1000);
            lastPrint = millis();
        }
        delay(10);
    }

    cal.mx_b = (xmax + xmin) / 2.0f;
    cal.my_b = (ymax + ymin) / 2.0f;
    cal.mz_b = (zmax + zmin) / 2.0f;
    float avg = ((xmax-xmin) + (ymax-ymin) + (zmax-zmin)) / 3.0f;
    float rx = xmax-xmin, ry = ymax-ymin, rz = zmax-zmin;
    cal.mx_s = (rx > 0.1f) ? avg / rx : 1.0f;
    cal.my_s = (ry > 0.1f) ? avg / ry : 1.0f;
    cal.mz_s = (rz > 0.1f) ? avg / rz : 1.0f;

    Serial.printf("[CAL] Mag bias:  X=%+.2f  Y=%+.2f  Z=%+.2f  µT\n",
                  cal.mx_b, cal.my_b, cal.mz_b);
    Serial.printf("[CAL] Mag scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",
                  cal.mx_s, cal.my_s, cal.mz_s);
}

// ─────────────────────────────────────────────────────────────
//  NVS: save / load / erase
// ─────────────────────────────────────────────────────────────
void MPU9250::saveCalibration(const char* ns) {
    Preferences p;
    p.begin(ns, false);
    p.putFloat("gx_b", cal.gx_b); p.putFloat("gy_b", cal.gy_b); p.putFloat("gz_b", cal.gz_b);
    p.putFloat("ax_b", cal.ax_b); p.putFloat("ay_b", cal.ay_b); p.putFloat("az_b", cal.az_b);
    p.putFloat("ax_s", cal.ax_s); p.putFloat("ay_s", cal.ay_s); p.putFloat("az_s", cal.az_s);
    p.putFloat("mx_b", cal.mx_b); p.putFloat("my_b", cal.my_b); p.putFloat("mz_b", cal.mz_b);
    p.putFloat("mx_s", cal.mx_s); p.putFloat("my_s", cal.my_s); p.putFloat("mz_s", cal.mz_s);
    p.putBool("valid", true);
    p.end();
    cal.valid = true;
    Serial.println(F("[NVS] Calibration saved."));
}

bool MPU9250::loadCalibration(const char* ns) {
    Preferences p;
    p.begin(ns, true);
    bool valid = p.getBool("valid", false);
    if (valid) {
        cal.gx_b = p.getFloat("gx_b", 0); cal.gy_b = p.getFloat("gy_b", 0); cal.gz_b = p.getFloat("gz_b", 0);
        cal.ax_b = p.getFloat("ax_b", 0); cal.ay_b = p.getFloat("ay_b", 0); cal.az_b = p.getFloat("az_b", 0);
        cal.ax_s = p.getFloat("ax_s", 1); cal.ay_s = p.getFloat("ay_s", 1); cal.az_s = p.getFloat("az_s", 1);
        cal.mx_b = p.getFloat("mx_b", 0); cal.my_b = p.getFloat("my_b", 0); cal.mz_b = p.getFloat("mz_b", 0);
        cal.mx_s = p.getFloat("mx_s", 1); cal.my_s = p.getFloat("my_s", 1); cal.mz_s = p.getFloat("mz_s", 1);
        cal.valid = true;
    }
    p.end();
    return valid;
}

void MPU9250::eraseCalibration(const char* ns) {
    Preferences p;
    p.begin(ns, false);
    p.clear();
    p.end();
    memset(&cal, 0, sizeof(cal));
    cal.ax_s = cal.ay_s = cal.az_s = 1.0f;
    cal.mx_s = cal.my_s = cal.mz_s = 1.0f;
    cal.mag_asa_x = cal.mag_asa_y = cal.mag_asa_z = 1.0f;
    Serial.println(F("[NVS] Calibration erased."));
}

void MPU9250::printCalibration() {
    Serial.println(F("\n--- MPU9250 Calibration ---"));
    Serial.printf("  Gyro bias:   X=%+.4f  Y=%+.4f  Z=%+.4f  °/s\n", cal.gx_b, cal.gy_b, cal.gz_b);
    Serial.printf("  Accel bias:  X=%+.4f  Y=%+.4f  Z=%+.4f  g\n",   cal.ax_b, cal.ay_b, cal.az_b);
    Serial.printf("  Accel scale: X=%+.4f  Y=%+.4f  Z=%+.4f\n",      cal.ax_s, cal.ay_s, cal.az_s);
    Serial.printf("  Mag bias:    X=%+.2f  Y=%+.2f  Z=%+.2f  µT\n",  cal.mx_b, cal.my_b, cal.mz_b);
    Serial.printf("  Mag scale:   X=%+.4f  Y=%+.4f  Z=%+.4f\n",      cal.mx_s, cal.my_s, cal.mz_s);
    Serial.printf("  Mag ASA:     X=%.4f  Y=%.4f  Z=%.4f\n",         cal.mag_asa_x, cal.mag_asa_y, cal.mag_asa_z);
    Serial.printf("  Valid: %s\n", cal.valid ? "YES" : "NO");
}
// ─────────────────────────────────────────────────────────────
//  diagMag() — print raw EXT_SENS_DATA bytes for debugging
//  Send 'D' from serial menu to call this.
//  What to look for:
//    mag[0] (ST1): should be 0x01 (DRDY set) on good reads
//    mag[7] (ST2): should be 0x00 (no overflow)
//    mag[1..6]: raw little-endian X/Y/Z data — nonzero if mag working
//  If ST1 is always 0x00: AK8963 not started or SLV0 config wrong
//  If ST2 has bit3 set: magnetic field too strong (unlikely indoors)
//  If all zeros: I2C master not enabled or SPI wiring issue
// ─────────────────────────────────────────────────────────────
void MPU9250::diagMag() {
    Serial.println(F("\n[DIAG] Raw EXT_SENS_DATA (10 samples):"));
    Serial.println(F("  [  ST1  ] [  HXL  HXH  HYL  HYH  HZL  HZH  ] [  ST2  ]"));

    for (int i = 0; i < 10; i++) {
        uint8_t mag[8];
        _burstRead(MPU_REG_EXT_SENS_DATA, mag, 8);

        bool drdy = (mag[0] & 0x01);
        bool hofl = (mag[7] & 0x08);

        int16_t rx = (int16_t)((mag[2]<<8)|mag[1]);
        int16_t ry = (int16_t)((mag[4]<<8)|mag[3]);
        int16_t rz = (int16_t)((mag[6]<<8)|mag[5]);

        float mx = rx * MAG_SCALE_16BIT * cal.mag_asa_x;
        float my = ry * MAG_SCALE_16BIT * cal.mag_asa_y;
        float mz = rz * MAG_SCALE_16BIT * cal.mag_asa_z;

        Serial.printf("  [0x%02X %s] [%6d %6d %6d]  ST2=0x%02X %s  → %.1f %.1f %.1f µT\n",
                      mag[0], drdy ? "DRDY" : "    ",
                      rx, ry, rz,
                      mag[7], hofl ? "HOFL" : "    ",
                      mx, my, mz);
        delay(15);
    }

    // Also read SLV0 config registers
    Serial.println(F("\n[DIAG] SLV0 config registers:"));
    Serial.printf("  SLV0_ADDR(0x25) = 0x%02X (should be 0x8C = AK8963|READ)\n",
                  _readReg(0x25));
    Serial.printf("  SLV0_REG (0x26) = 0x%02X (should be 0x02 = ST1)\n",
                  _readReg(0x26));
    Serial.printf("  SLV0_CTRL(0x27) = 0x%02X (should be 0x88 = EN|8bytes)\n",
                  _readReg(0x27));
    Serial.printf("  USER_CTRL(0x6A) = 0x%02X (bit5=I2C_MST_EN should be 1)\n",
                  _readReg(0x6A));
    Serial.printf("  I2C_MST_CTRL(0x24) = 0x%02X\n",
                  _readReg(0x24));
    Serial.printf("  I2C_MST_STATUS(0x36) = 0x%02X\n",
                  _readReg(0x36));
}
