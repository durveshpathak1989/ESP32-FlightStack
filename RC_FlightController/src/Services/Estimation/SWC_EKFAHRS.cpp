#include "SWC_EKFAHRS.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kGravity = 9.80665f;
constexpr float kPi = flight::FlightConstants::kPi;

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

float wrapPi(float rad)
{
    while (rad > kPi) rad -= 2.0f * kPi;
    while (rad <= -kPi) rad += 2.0f * kPi;
    return rad;
}

float finiteOr(float value, float fallback)
{
    return std::isfinite(value) ? value : fallback;
}

}  // namespace

void SWC_EKFAHRS::init(const SWC_EKFAHRSInput& initial)
{
    reset();
    update(initial);
}

SWC_EKFAHRSOutput SWC_EKFAHRS::update(const SWC_EKFAHRSInput& input)
{
    SWC_EKFAHRSOutput out;
    out.timestampUs = input.timestampUs;
    out.attitude.timestampUs = input.timestampUs;

    float dt = static_cast<float>(input.dtUs) * 0.000001f;
    if (!std::isfinite(dt) || dt <= 0.0f || dt > 0.05f) {
        dt = flight::FlightConstants::kDefaultControlPeriodS;
    }

    const float ax = finiteOr(input.imu.ax, 0.0f) - _accelBiasX;
    const float ay = finiteOr(input.imu.ay, 0.0f) - _accelBiasY;
    const float az = finiteOr(input.imu.az, kGravity) - _accelBiasZ;
    const float gx = finiteOr(input.imu.gx, 0.0f) - _gyroBiasX;
    const float gy = finiteOr(input.imu.gy, 0.0f) - _gyroBiasY;
    const float gz = finiteOr(input.imu.gz, 0.0f) - _gyroBiasZ;

    _roll = wrapPi(_roll + gx * dt);
    _pitch = clampf(_pitch + gy * dt, -1.5f, 1.5f);
    _yaw = wrapPi(_yaw + gz * dt);

    const float accelNorm = std::sqrt(ax * ax + ay * ay + az * az);
    const float accelErrG = std::fabs((accelNorm / kGravity) - 1.0f);
    bool accelAccepted = false;
    if (accelNorm > 0.001f && accelErrG < 0.35f) {
        const float accelRoll = std::atan2(ay, az);
        const float accelPitch = std::atan2(-ax, std::sqrt(ay * ay + az * az));
        float alpha = 0.020f;
        if (accelErrG >= 0.20f) {
            alpha = 0.004f;
        } else if (accelErrG >= 0.10f) {
            alpha = 0.010f;
        }

        if (!_initialized) {
            _roll = accelRoll;
            _pitch = accelPitch;
            _initialized = true;
        } else {
            _roll = wrapPi(_roll + alpha * wrapPi(accelRoll - _roll));
            _pitch = clampf(_pitch + alpha * (accelPitch - _pitch), -1.5f, 1.5f);
        }
        accelAccepted = true;
    }

    bool magAccepted = false;
    const float magNorm = std::sqrt(input.imu.mx * input.imu.mx +
                                    input.imu.my * input.imu.my +
                                    input.imu.mz * input.imu.mz);
    if (input.imu.magValid && magNorm > 0.001f) {
        const float magYaw = std::atan2(-input.imu.my, input.imu.mx);
        _yaw = wrapPi(_yaw + 0.010f * wrapPi(magYaw - _yaw));
        magAccepted = true;
    }

    const bool nearlyStatic = accelAccepted &&
        std::fabs(gx) < 0.05f &&
        std::fabs(gy) < 0.05f &&
        std::fabs(gz) < 0.05f;
    if (nearlyStatic) {
        constexpr float kBiasAlpha = 0.0005f;
        _gyroBiasX += kBiasAlpha * (input.imu.gx - _gyroBiasX);
        _gyroBiasY += kBiasAlpha * (input.imu.gy - _gyroBiasY);
        _gyroBiasZ += kBiasAlpha * (input.imu.gz - _gyroBiasZ);
    }

    out.attitude.roll = _roll;
    out.attitude.pitch = _pitch;
    out.attitude.yaw = _yaw;
    out.attitude.rollRate = gx;
    out.attitude.pitchRate = gy;
    out.attitude.yawRate = gz;
    out.attitude.accelX = ax;
    out.attitude.accelY = ay;
    out.attitude.accelZ = az;
    out.debug.gyroBiasX = _gyroBiasX;
    out.debug.gyroBiasY = _gyroBiasY;
    out.debug.gyroBiasZ = _gyroBiasZ;
    out.debug.accelBiasX = _accelBiasX;
    out.debug.accelBiasY = _accelBiasY;
    out.debug.accelBiasZ = _accelBiasZ;
    out.debug.accelNormMps2 = accelNorm;
    out.debug.accelAccepted = accelAccepted;
    out.debug.magAccepted = magAccepted;
    out.debug.healthy = std::isfinite(_roll) && std::isfinite(_pitch) && std::isfinite(_yaw);
    return out;
}

void SWC_EKFAHRS::reset()
{
    _roll = 0.0f;
    _pitch = 0.0f;
    _yaw = 0.0f;
    _gyroBiasX = 0.0f;
    _gyroBiasY = 0.0f;
    _gyroBiasZ = 0.0f;
    _accelBiasX = 0.0f;
    _accelBiasY = 0.0f;
    _accelBiasZ = 0.0f;
    _initialized = false;
}
