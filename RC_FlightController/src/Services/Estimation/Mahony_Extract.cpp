#include "Mahony_Extract.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kGravity = 9.80665f;

float invSqrt(float x)
{
    return x > 0.0f ? 1.0f / std::sqrt(x) : 0.0f;
}

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

float wrapPi(float rad)
{
    while (rad > flight::FlightConstants::kPi) rad -= 2.0f * flight::FlightConstants::kPi;
    while (rad <= -flight::FlightConstants::kPi) rad += 2.0f * flight::FlightConstants::kPi;
    return rad;
}

}  // namespace

void Mahony_Extract::setGains(float kp, float ki)
{
    _kp = kp;
    _ki = ki;
}

void Mahony_Extract::reset()
{
    _q0 = 1.0f;
    _q1 = 0.0f;
    _q2 = 0.0f;
    _q3 = 0.0f;
    _ix = 0.0f;
    _iy = 0.0f;
    _iz = 0.0f;
}

flight::AttitudeState Mahony_Extract::update(const SWC_EKFAHRSInput::RawImu& imu,
                                             std::uint64_t dtUs,
                                             std::uint64_t timestampUs)
{
    float dt = static_cast<float>(dtUs) * 0.000001f;
    if (!std::isfinite(dt) || dt <= 0.0f || dt > 0.05f) {
        dt = flight::FlightConstants::kDefaultControlPeriodS;
    }

    float gx = imu.gx;
    float gy = imu.gy;
    float gz = imu.gz;
    float ax = imu.ax / kGravity;
    float ay = imu.ay / kGravity;
    float az = imu.az / kGravity;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;
    const float a2 = ax * ax + ay * ay + az * az;
    if (a2 > 0.0001f) {
        const float invA = invSqrt(a2);
        ax *= invA;
        ay *= invA;
        az *= invA;

        const float vx = 2.0f * (_q1 * _q3 - _q0 * _q2);
        const float vy = 2.0f * (_q0 * _q1 + _q2 * _q3);
        const float vz = _q0 * _q0 - _q1 * _q1 - _q2 * _q2 + _q3 * _q3;

        ex += ay * vz - az * vy;
        ey += az * vx - ax * vz;
        ez += ax * vy - ay * vx;

        if (_ki > 0.0f) {
            _ix += _ki * ex * dt;
            _iy += _ki * ey * dt;
            _iz += _ki * ez * dt;
        } else {
            _ix = 0.0f;
            _iy = 0.0f;
            _iz = 0.0f;
        }

        gx += _kp * ex + _ix;
        gy += _kp * ey + _iy;
        gz += _kp * ez + _iz;
    }

    const float halfDt = 0.5f * dt;
    _q0 += (-_q1 * gx - _q2 * gy - _q3 * gz) * halfDt;
    _q1 += ( _q0 * gx + _q2 * gz - _q3 * gy) * halfDt;
    _q2 += ( _q0 * gy - _q1 * gz + _q3 * gx) * halfDt;
    _q3 += ( _q0 * gz + _q1 * gy - _q2 * gx) * halfDt;

    const float invQ = invSqrt(_q0 * _q0 + _q1 * _q1 + _q2 * _q2 + _q3 * _q3);
    if (invQ > 0.0f) {
        _q0 *= invQ;
        _q1 *= invQ;
        _q2 *= invQ;
        _q3 *= invQ;
    }

    flight::AttitudeState out;
    out.roll = std::atan2(2.0f * (_q0 * _q1 + _q2 * _q3),
                          1.0f - 2.0f * (_q1 * _q1 + _q2 * _q2));
    const float sinP = clampf(2.0f * (_q0 * _q2 - _q3 * _q1), -1.0f, 1.0f);
    out.pitch = std::asin(sinP);
    out.yaw = wrapPi(std::atan2(2.0f * (_q0 * _q3 + _q1 * _q2),
                                1.0f - 2.0f * (_q2 * _q2 + _q3 * _q3)));
    out.rollRate = gx;
    out.pitchRate = gy;
    out.yawRate = gz;
    out.accelX = imu.ax;
    out.accelY = imu.ay;
    out.accelZ = imu.az;
    out.timestampUs = timestampUs;
    return out;
}
