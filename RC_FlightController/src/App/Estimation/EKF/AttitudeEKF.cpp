/*
 * Name: AttitudeEKF.cpp
 * Use: 18-state inertial EKF implementation.
 * Version: 5.0.0-experimental
 * Created by: Durvesh Pathak dp676@cornell.edu
 */

#include "AttitudeEKF.h"
#include <math.h>
#include <string.h>

static constexpr float EKF_GRAVITY_MPS2 = 9.80665f;
static constexpr float EKF_MIN_VARIANCE = 1.0e-9f;
static constexpr float EKF_MAX_ABS_BIAS_RAD_S = 0.35f;
static constexpr float EKF_MAX_ABS_RATE_RAD_S = 35.0f;
static constexpr float EKF_TOF_MIN_DT_S = 0.015f;
static constexpr float EKF_TOF_MAX_DT_S = 0.250f;
static constexpr float EKF_TOF_MAX_ABS_VELOCITY_MPS = 5.0f;
static constexpr float EKF_TOF_MIN_TILT_SCALE = 0.30f;
static constexpr float EKF_TOF_VELOCITY_ALPHA = 0.35f;

AttitudeEKF::AttitudeEKF()
{
    _positionQ = 1.0e-6f;
    _velocityQ = 1.0e-5f;
    _worldAccelQ = 5.0e-3f;
    _angleQ = 8.0e-4f;
    _bodyRateQ = 2.0e-3f;
    _biasQ = 1.0e-6f;

    _gyroRateR = 2.5e-3f;
    _accelAngleR = 0.06f;
    _worldAccelR = 0.35f;
    _magYawR = 0.20f;
    _tofAltitudeR = 0.0025f;
    _tofVelocityR = 0.25f;

    _declinationRad = 0.0f;
    _magYawOffsetRad = 0.0f;
    _magYawSign = 1.0f;
    reset();
}

void AttitudeEKF::reset()
{
    memset(_x, 0, sizeof(_x));
    memset(_cov, 0, sizeof(_cov));

    _cov[PX][PX] = _cov[PY][PY] = _cov[PZ][PZ] = 10.0f;
    _cov[VX][VX] = _cov[VY][VY] = _cov[VZ][VZ] = 5.0f;
    _cov[AX][AX] = _cov[AY][AY] = _cov[AZ][AZ] = 2.0f;

    _cov[ROLL][ROLL] = 0.05f;
    _cov[PITCH][PITCH] = 0.05f;
    _cov[YAW][YAW] = 0.20f;

    _cov[WX][WX] = _cov[WY][WY] = _cov[WZ][WZ] = 0.20f;
    _cov[BGX][BGX] = _cov[BGY][BGY] = _cov[BGZ][BGZ] = 0.10f;

    _lastMagAccepted = false;
    _lastTofVelocityAccepted = false;
    _lastBmpAltM = 0.0f;
    _lastTofAltM = 0.0f;
    _lastBmpValid = false;
    _lastTofValid = false;
    _lastAltTsMs = 0;

    _tofHistoryValid = false;
    _previousTofVerticalM = 0.0f;
    _filteredTofVelocityMps = 0.0f;
    _previousTofTsMs = 0;
    _syncOutputs();
}

void AttitudeEKF::resetPositionVelocity()
{
    _x[PX] = _x[PY] = _x[PZ] = 0.0f;
    _x[VX] = _x[VY] = _x[VZ] = 0.0f;
    _x[AX] = _x[AY] = _x[AZ] = 0.0f;

    for (int i = PX; i <= AZ; ++i) {
        for (int j = 0; j < STATE_DIM; ++j) {
            _cov[i][j] = 0.0f;
            _cov[j][i] = 0.0f;
        }
    }
    _cov[PX][PX] = _cov[PY][PY] = _cov[PZ][PZ] = 10.0f;
    _cov[VX][VX] = _cov[VY][VY] = _cov[VZ][VZ] = 5.0f;
    _cov[AX][AX] = _cov[AY][AY] = _cov[AZ][AZ] = 2.0f;

    _tofHistoryValid = false;
    _previousTofVerticalM = 0.0f;
    _filteredTofVelocityMps = 0.0f;
    _previousTofTsMs = 0;
    _lastTofVelocityAccepted = false;
    _syncOutputs();
}

void AttitudeEKF::setProcessNoise(float angleQ, float biasQ)
{
    _angleQ = fmaxf(angleQ, 0.0f);
    _biasQ = fmaxf(biasQ, 0.0f);
}

void AttitudeEKF::setAccelMeasurementNoise(float accelAngleR)
{
    _accelAngleR = fmaxf(accelAngleR, EKF_MIN_VARIANCE);
}

void AttitudeEKF::setMagMeasurementNoise(float magYawR)
{
    _magYawR = fmaxf(magYawR, EKF_MIN_VARIANCE);
}

void AttitudeEKF::setMagDeclinationDeg(float declinationDeg)
{
    _declinationRad = declinationDeg * AHRS_DEG_TO_RAD;
}

void AttitudeEKF::setMagYawOffsetDeg(float offsetDeg)
{
    _magYawOffsetRad = offsetDeg * AHRS_DEG_TO_RAD;
}

void AttitudeEKF::setMagYawSign(float sign)
{
    _magYawSign = (sign < 0.0f) ? -1.0f : 1.0f;
}

void AttitudeEKF::setKinematicProcessNoise(float positionQ, float velocityQ, float worldAccelQ)
{
    _positionQ = fmaxf(positionQ, 0.0f);
    _velocityQ = fmaxf(velocityQ, 0.0f);
    _worldAccelQ = fmaxf(worldAccelQ, 0.0f);
}

void AttitudeEKF::setRateProcessNoise(float bodyRateQ)
{
    _bodyRateQ = fmaxf(bodyRateQ, 0.0f);
}

void AttitudeEKF::setGyroMeasurementNoise(float gyroRateR)
{
    _gyroRateR = fmaxf(gyroRateR, EKF_MIN_VARIANCE);
}

void AttitudeEKF::setWorldAccelMeasurementNoise(float worldAccelR)
{
    _worldAccelR = fmaxf(worldAccelR, EKF_MIN_VARIANCE);
}

void AttitudeEKF::setTofMeasurementNoise(float altitudeR, float verticalVelocityR)
{
    _tofAltitudeR = fmaxf(altitudeR, EKF_MIN_VARIANCE);
    _tofVelocityR = fmaxf(verticalVelocityR, EKF_MIN_VARIANCE);
}

void AttitudeEKF::_predict(float dt)
{
    const float halfDt2 = 0.5f * dt * dt;
    for (int axis = 0; axis < 3; ++axis) {
        _x[PX + axis] += _x[VX + axis] * dt + _x[AX + axis] * halfDt2;
        _x[VX + axis] += _x[AX + axis] * dt;
        _x[ROLL + axis] += _x[WX + axis] * dt;
    }
    _x[YAW] = ahrsWrap180(_x[YAW] * AHRS_RAD_TO_DEG) * AHRS_DEG_TO_RAD;
}

void AttitudeEKF::_predictCovariance(float dt)
{
    float f[STATE_DIM][STATE_DIM];
    float fp[STATE_DIM][STATE_DIM];
    float next[STATE_DIM][STATE_DIM];
    memset(f, 0, sizeof(f));
    memset(fp, 0, sizeof(fp));
    memset(next, 0, sizeof(next));

    for (int i = 0; i < STATE_DIM; ++i) f[i][i] = 1.0f;

    const float halfDt2 = 0.5f * dt * dt;
    for (int axis = 0; axis < 3; ++axis) {
        f[PX + axis][VX + axis] = dt;
        f[PX + axis][AX + axis] = halfDt2;
        f[VX + axis][AX + axis] = dt;
        f[ROLL + axis][WX + axis] = dt;
    }

    for (int r = 0; r < STATE_DIM; ++r) {
        for (int k = 0; k < STATE_DIM; ++k) {
            const float frk = f[r][k];
            if (frk == 0.0f) continue;
            for (int c = 0; c < STATE_DIM; ++c) {
                fp[r][c] += frk * _cov[k][c];
            }
        }
    }

    for (int r = 0; r < STATE_DIM; ++r) {
        for (int c = 0; c < STATE_DIM; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < STATE_DIM; ++k) {
                const float fck = f[c][k];
                if (fck != 0.0f) sum += fp[r][k] * fck;
            }
            next[r][c] = sum;
        }
    }

    memcpy(_cov, next, sizeof(_cov));
    _addProcessNoise();
    _stabilizeCovariance();
}

void AttitudeEKF::_addProcessNoise()
{
    for (int axis = 0; axis < 3; ++axis) {
        _cov[PX + axis][PX + axis] += _positionQ;
        _cov[VX + axis][VX + axis] += _velocityQ;
        _cov[AX + axis][AX + axis] += _worldAccelQ;
        _cov[ROLL + axis][ROLL + axis] += _angleQ;
        _cov[WX + axis][WX + axis] += _bodyRateQ;
        _cov[BGX + axis][BGX + axis] += _biasQ;
    }
}

void AttitudeEKF::_updateScalar(const float h[STATE_DIM], float measurement,
                                float variance, bool wrapAngleInnovation)
{
    float ph[STATE_DIM];
    float innovation = measurement;
    float predicted = 0.0f;

    for (int i = 0; i < STATE_DIM; ++i) predicted += h[i] * _x[i];
    innovation -= predicted;
    if (wrapAngleInnovation) {
        innovation = ahrsWrap180(innovation * AHRS_RAD_TO_DEG) * AHRS_DEG_TO_RAD;
    }

    for (int r = 0; r < STATE_DIM; ++r) {
        float sum = 0.0f;
        for (int c = 0; c < STATE_DIM; ++c) sum += _cov[r][c] * h[c];
        ph[r] = sum;
    }

    float s = fmaxf(variance, EKF_MIN_VARIANCE);
    for (int i = 0; i < STATE_DIM; ++i) s += h[i] * ph[i];
    if (!isfinite(s) || s <= EKF_MIN_VARIANCE) return;

    float k[STATE_DIM];
    for (int i = 0; i < STATE_DIM; ++i) k[i] = ph[i] / s;
    for (int i = 0; i < STATE_DIM; ++i) _x[i] += k[i] * innovation;

    float oldP[STATE_DIM][STATE_DIM];
    memcpy(oldP, _cov, sizeof(oldP));
    for (int r = 0; r < STATE_DIM; ++r) {
        for (int c = 0; c < STATE_DIM; ++c) {
            float hp = 0.0f;
            for (int j = 0; j < STATE_DIM; ++j) hp += h[j] * oldP[j][c];
            _cov[r][c] = oldP[r][c] - k[r] * hp;
        }
    }

    _x[YAW] = ahrsWrap180(_x[YAW] * AHRS_RAD_TO_DEG) * AHRS_DEG_TO_RAD;
    for (int axis = 0; axis < 3; ++axis) {
        _x[WX + axis] = fmaxf(-EKF_MAX_ABS_RATE_RAD_S,
                             fminf(EKF_MAX_ABS_RATE_RAD_S, _x[WX + axis]));
        _x[BGX + axis] = fmaxf(-EKF_MAX_ABS_BIAS_RAD_S,
                              fminf(EKF_MAX_ABS_BIAS_RAD_S, _x[BGX + axis]));
    }
    _stabilizeCovariance();
}

void AttitudeEKF::_updateDirectState(int stateIndex, float measurement,
                                     float variance, bool wrapAngleInnovation)
{
    if (stateIndex < 0 || stateIndex >= STATE_DIM) return;
    float h[STATE_DIM] = {0.0f};
    h[stateIndex] = 1.0f;
    _updateScalar(h, measurement, variance, wrapAngleInnovation);
}

void AttitudeEKF::_updateGyro(const AHRSInput& in)
{
    const float gyro[3] = {
        in.gx_dps * AHRS_DEG_TO_RAD,
        in.gy_dps * AHRS_DEG_TO_RAD,
        in.gz_dps * AHRS_DEG_TO_RAD
    };

    for (int axis = 0; axis < 3; ++axis) {
        float h[STATE_DIM] = {0.0f};
        h[WX + axis] = 1.0f;
        h[BGX + axis] = 1.0f;
        _updateScalar(h, gyro[axis], _gyroRateR, false);
    }
}

void AttitudeEKF::_updateAccelTilt(const AHRSInput& in)
{
    const float a2 = in.ax_g*in.ax_g + in.ay_g*in.ay_g + in.az_g*in.az_g;
    if (a2 <= 1.0e-6f) return;

    const float accelNorm = sqrtf(a2);
    const float accelErr = fabsf(accelNorm - 1.0f);
    float activeR = _accelAngleR;
    if (accelErr < 0.10f) {
        activeR = _accelAngleR;
    } else if (accelErr < 0.20f) {
        activeR = fmaxf(_accelAngleR, 0.15f);
    } else if (accelErr < 0.35f) {
        activeR = fmaxf(_accelAngleR, 0.50f);
    } else {
        activeR = 2.0f;
    }

    float rollDeg = 0.0f;
    float pitchDeg = 0.0f;
    ahrsAccelAnglesDeg(in.ax_g, in.ay_g, in.az_g, rollDeg, pitchDeg);
    _updateDirectState(ROLL, rollDeg * AHRS_DEG_TO_RAD, activeR, true);
    _updateDirectState(PITCH, pitchDeg * AHRS_DEG_TO_RAD, activeR, true);
}

void AttitudeEKF::_updateWorldAcceleration(const AHRSInput& in)
{
    const float a2 = in.ax_g*in.ax_g + in.ay_g*in.ay_g + in.az_g*in.az_g;
    if (a2 <= 1.0e-6f) return;

    const float cr = cosf(_x[ROLL]);
    const float sr = sinf(_x[ROLL]);
    const float cp = cosf(_x[PITCH]);
    const float sp = sinf(_x[PITCH]);
    const float cy = cosf(_x[YAW]);
    const float sy = sinf(_x[YAW]);

    const float axWorldG = (cy*cp) * in.ax_g
                         + (cy*sp*sr - sy*cr) * in.ay_g
                         + (cy*sp*cr + sy*sr) * in.az_g;
    const float ayWorldG = (sy*cp) * in.ax_g
                         + (sy*sp*sr + cy*cr) * in.ay_g
                         + (sy*sp*cr - cy*sr) * in.az_g;
    const float azWorldG = (-sp) * in.ax_g
                         + (cp*sr) * in.ay_g
                         + (cp*cr) * in.az_g;

    const float measured[3] = {
        axWorldG * EKF_GRAVITY_MPS2,
        ayWorldG * EKF_GRAVITY_MPS2,
        (azWorldG - 1.0f) * EKF_GRAVITY_MPS2
    };

    const float accelNorm = sqrtf(a2);
    const float normError = fabsf(accelNorm - 1.0f);
    const float adaptiveR = _worldAccelR * (1.0f + 8.0f * normError);
    for (int axis = 0; axis < 3; ++axis) {
        _updateDirectState(AX + axis, measured[axis], adaptiveR, false);
    }
}

bool AttitudeEKF::_magYawRad(const AHRSInput& in, float& yawRad) const
{
    const float m2 = in.mx_uT*in.mx_uT + in.my_uT*in.my_uT + in.mz_uT*in.mz_uT;
    if (!in.magValid || m2 < 1.0f) return false;

    const float cr = cosf(_x[ROLL]);
    const float sr = sinf(_x[ROLL]);
    const float cp = cosf(_x[PITCH]);
    const float sp = sinf(_x[PITCH]);

    const float xh = in.mx_uT * cp + in.mz_uT * sp;
    const float yh = in.mx_uT * sr * sp + in.my_uT * cr - in.mz_uT * sr * cp;

    yawRad = atan2f(-yh, xh);
    yawRad = _magYawSign * yawRad + _declinationRad + _magYawOffsetRad;
    yawRad = ahrsWrap180(yawRad * AHRS_RAD_TO_DEG) * AHRS_DEG_TO_RAD;
    return true;
}

bool AttitudeEKF::update(const AHRSInput& in, float dt, AttitudeEstimate& out)
{
    if (!isfinite(dt) || dt <= 0.0f || dt > 0.05f) dt = 0.0025f;

    _predict(dt);
    _predictCovariance(dt);
    _updateGyro(in);
    _updateAccelTilt(in);
    _updateWorldAcceleration(in);

    _lastMagAccepted = false;
    float magYaw = 0.0f;
    if (_magYawRad(in, magYaw)) {
        _updateDirectState(YAW, magYaw, _magYawR, true);
        _lastMagAccepted = true;
    }

    _syncOutputs();
    _quatFromEuler(_x[ROLL], _x[PITCH], _x[YAW], out);
    out.roll_deg = _x[ROLL] * AHRS_RAD_TO_DEG;
    out.pitch_deg = _x[PITCH] * AHRS_RAD_TO_DEG;
    out.yaw_deg = ahrsWrap360(_x[YAW] * AHRS_RAD_TO_DEG);
    return true;
}

void AttitudeEKF::updatePositionMeasurement(float x_m, float y_m, float z_m,
                                            float varianceX, float varianceY, float varianceZ)
{
    _updateDirectState(PX, x_m, varianceX);
    _updateDirectState(PY, y_m, varianceY);
    _updateDirectState(PZ, z_m, varianceZ);
    _syncOutputs();
}

void AttitudeEKF::updateVelocityMeasurement(float vx_mps, float vy_mps, float vz_mps,
                                            float varianceX, float varianceY, float varianceZ)
{
    _updateDirectState(VX, vx_mps, varianceX);
    _updateDirectState(VY, vy_mps, varianceY);
    _updateDirectState(VZ, vz_mps, varianceZ);
    _syncOutputs();
}

void AttitudeEKF::updateAltitudeMeasurement(float z_m, float variance)
{
    if (!isfinite(z_m)) return;
    _updateDirectState(PZ, z_m, variance);
    _syncOutputs();
}

void AttitudeEKF::updateTofMeasurement(float rangeM, uint32_t sampleTsMs)
{
    _lastTofVelocityAccepted = false;
    if (!isfinite(rangeM) || rangeM <= 0.0f || sampleTsMs == 0) return;

    // Do not repeatedly fuse the same physical ToF sample. The caller must pass
    // the sensor sample timestamp, not merely the current control-loop time.
    if (_tofHistoryValid && sampleTsMs == _previousTofTsMs) return;

    // A downward-facing range is slant distance when the aircraft is tilted.
    // Convert it to height above the local ground plane.
    const float tiltScale = cosf(_x[ROLL]) * cosf(_x[PITCH]);
    if (!isfinite(tiltScale) || tiltScale < EKF_TOF_MIN_TILT_SCALE) {
        _tofHistoryValid = false;
        return;
    }

    const float verticalM = rangeM * tiltScale;
    _updateDirectState(PZ, verticalM, _tofAltitudeR, false);

    if (_tofHistoryValid && sampleTsMs > _previousTofTsMs) {
        const float dt = (sampleTsMs - _previousTofTsMs) * 0.001f;
        if (dt >= EKF_TOF_MIN_DT_S && dt <= EKF_TOF_MAX_DT_S) {
            const float rawVelocityMps = (verticalM - _previousTofVerticalM) / dt;
            if (isfinite(rawVelocityMps) &&
                fabsf(rawVelocityMps) <= EKF_TOF_MAX_ABS_VELOCITY_MPS) {
                _filteredTofVelocityMps =
                    EKF_TOF_VELOCITY_ALPHA * rawVelocityMps +
                    (1.0f - EKF_TOF_VELOCITY_ALPHA) * _filteredTofVelocityMps;

                // ToF altitude increases while ascending, matching the EKF's
                // positive-up world-Z convention.
                _updateDirectState(VZ, _filteredTofVelocityMps, _tofVelocityR, false);
                _lastTofVelocityAccepted = true;
            }
        }
    } else {
        // Seed the derivative filter from the current predicted vertical speed.
        _filteredTofVelocityMps = _x[VZ];
    }

    _previousTofVerticalM = verticalM;
    _previousTofTsMs = sampleTsMs;
    _tofHistoryValid = true;
    _lastTofAltM = rangeM;
    _lastTofValid = true;
    _lastAltTsMs = sampleTsMs;
    _syncOutputs();
}

void AttitudeEKF::updateAltitudeSensors(float bmpAltM, bool bmpValid,
                                        float tofAltM, bool tofValid, uint32_t tsMs)
{
    _lastBmpAltM = bmpAltM;
    _lastBmpValid = bmpValid;

    // Sequential Kalman updates are preferable to hard-coded pre-blending:
    // BMP provides a slow absolute-altitude correction, while ToF provides a
    // lower-altitude PZ correction and a differentiated VZ correction.
    if (bmpValid && isfinite(bmpAltM)) {
        updateAltitudeMeasurement(bmpAltM, 1.0f);
    }
    if (tofValid) {
        updateTofMeasurement(tofAltM, tsMs);
    } else {
        _lastTofValid = false;
        _lastTofVelocityAccepted = false;
    }
}

float AttitudeEKF::rollBiasDps() const { return _x[BGX] * AHRS_RAD_TO_DEG; }
float AttitudeEKF::pitchBiasDps() const { return _x[BGY] * AHRS_RAD_TO_DEG; }
float AttitudeEKF::yawBiasDps() const { return _x[BGZ] * AHRS_RAD_TO_DEG; }
float AttitudeEKF::rollRateDps() const { return _x[WX] * AHRS_RAD_TO_DEG; }
float AttitudeEKF::pitchRateDps() const { return _x[WY] * AHRS_RAD_TO_DEG; }
float AttitudeEKF::yawRateDps() const { return _x[WZ] * AHRS_RAD_TO_DEG; }

float AttitudeEKF::state(int index) const
{
    return (index >= 0 && index < STATE_DIM) ? _x[index] : 0.0f;
}

float AttitudeEKF::covariance(int row, int col) const
{
    if (row < 0 || row >= STATE_DIM || col < 0 || col >= STATE_DIM) return 0.0f;
    return _cov[row][col];
}

void AttitudeEKF::_syncOutputs()
{
    _posVel.posX_m = _x[PX];
    _posVel.posY_m = _x[PY];
    _posVel.posZ_m = _x[PZ];
    _posVel.velX_mps = _x[VX];
    _posVel.velY_mps = _x[VY];
    _posVel.velZ_mps = _x[VZ];
    _posVel.accelX_mps2 = _x[AX];
    _posVel.accelY_mps2 = _x[AY];
    _posVel.accelZ_mps2 = _x[AZ];
    _posVel.valid = true;

    _bodyRates.rollRate_dps = rollRateDps();
    _bodyRates.pitchRate_dps = pitchRateDps();
    _bodyRates.yawRate_dps = yawRateDps();
    _bodyRates.valid = true;
}

void AttitudeEKF::_stabilizeCovariance()
{
    for (int r = 0; r < STATE_DIM; ++r) {
        for (int c = r + 1; c < STATE_DIM; ++c) {
            const float avg = 0.5f * (_cov[r][c] + _cov[c][r]);
            _cov[r][c] = avg;
            _cov[c][r] = avg;
        }
        if (!isfinite(_cov[r][r]) || _cov[r][r] < EKF_MIN_VARIANCE) {
            _cov[r][r] = EKF_MIN_VARIANCE;
        }
    }
}

void AttitudeEKF::_quatFromEuler(float roll, float pitch, float yaw, AttitudeEstimate& out)
{
    const float cr = cosf(roll * 0.5f);
    const float sr = sinf(roll * 0.5f);
    const float cp = cosf(pitch * 0.5f);
    const float sp = sinf(pitch * 0.5f);
    const float cy = cosf(yaw * 0.5f);
    const float sy = sinf(yaw * 0.5f);

    out.q0 = cr*cp*cy + sr*sp*sy;
    out.q1 = sr*cp*cy - cr*sp*sy;
    out.q2 = cr*sp*cy + sr*cp*sy;
    out.q3 = cr*cp*sy - sr*sp*cy;
}
