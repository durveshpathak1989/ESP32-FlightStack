/*
 * Name: LQIController.cpp
 * Use: Lightweight per-axis LQI attitude/rate controller for ESP32 flight loops.
 */

#include "LQIController.h"

LQIController::LQIController()
    : _integratorLimit(0.20f)
{
    begin();
}

void LQIController::begin()
{
    for (uint8_t i = 0; i < 3; ++i) {
        _axis[i].gains = {0.0f, 0.0f, 0.0f};
        _axis[i].outputLimit = 0.10f;
        _axis[i].integrator = 0.0f;
        _axis[i].last = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false};
    }
}

void LQIController::reset()
{
    resetAxis(LQIAxis::Roll);
    resetAxis(LQIAxis::Pitch);
    resetAxis(LQIAxis::Yaw);
}

void LQIController::resetAxis(LQIAxis axis)
{
    AxisState& a = _axis[_index(axis)];
    a.integrator = 0.0f;
    a.last = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false};
}

void LQIController::setGains(LQIAxis axis, float kAngle, float kRate, float kIntegral)
{
    AxisState& a = _axis[_index(axis)];
    a.gains.kAngle = kAngle;
    a.gains.kRate = kRate;
    a.gains.kIntegral = kIntegral;
}

void LQIController::setOutputLimits(float rollLimit, float pitchLimit, float yawLimit)
{
    setOutputLimit(LQIAxis::Roll, rollLimit);
    setOutputLimit(LQIAxis::Pitch, pitchLimit);
    setOutputLimit(LQIAxis::Yaw, yawLimit);
}

void LQIController::setOutputLimit(LQIAxis axis, float limit)
{
    _axis[_index(axis)].outputLimit = constrain(fabsf(limit), 0.0f, 1.0f);
}

void LQIController::setIntegratorLimit(float limit)
{
    _integratorLimit = constrain(fabsf(limit), 0.0f, 100.0f);
    for (uint8_t i = 0; i < 3; ++i) {
        _axis[i].integrator = constrain(_axis[i].integrator, -_integratorLimit, _integratorLimit);
    }
}

float LQIController::updateAngleAxis(LQIAxis axis, float angleErrorRad, float rateErrorRadPerSec, float dtSec)
{
    return _update(axis, angleErrorRad, rateErrorRadPerSec, dtSec, true);
}

float LQIController::updateRateAxis(LQIAxis axis, float rateErrorRadPerSec, float dtSec)
{
    return _update(axis, 0.0f, rateErrorRadPerSec, dtSec, false);
}

float LQIController::lastAngleContribution(LQIAxis axis) const
{
    return _axis[_index(axis)].last.angle;
}

float LQIController::lastRateContribution(LQIAxis axis) const
{
    return _axis[_index(axis)].last.rate;
}

float LQIController::lastIntegralContribution(LQIAxis axis) const
{
    return _axis[_index(axis)].last.integral;
}

float LQIController::lastOutput(LQIAxis axis) const
{
    return _axis[_index(axis)].last.output;
}

float LQIController::integrator(LQIAxis axis) const
{
    return _axis[_index(axis)].integrator;
}

bool LQIController::outputLimited(LQIAxis axis) const
{
    return _axis[_index(axis)].last.outputLimited;
}

bool LQIController::anyOutputLimited() const
{
    return outputLimited(LQIAxis::Roll) ||
           outputLimited(LQIAxis::Pitch) ||
           outputLimited(LQIAxis::Yaw);
}

uint8_t LQIController::_index(LQIAxis axis)
{
    const uint8_t i = static_cast<uint8_t>(axis);
    return (i < 3) ? i : 0;
}

float LQIController::_update(LQIAxis axis, float angleErrorRad, float rateErrorRadPerSec, float dtSec, bool useAngle)
{
    AxisState& a = _axis[_index(axis)];
    const float dt = (dtSec > 0.0f && dtSec < 0.10f) ? dtSec : 0.0f;
    const float iInput = useAngle ? angleErrorRad : rateErrorRadPerSec;
    a.integrator = constrain(a.integrator + iInput * dt, -_integratorLimit, _integratorLimit);

    const float angleTerm = useAngle ? (-a.gains.kAngle * angleErrorRad) : 0.0f;
    const float rateTerm = -a.gains.kRate * rateErrorRadPerSec;
    const float integralTerm = -a.gains.kIntegral * a.integrator;
    const float raw = angleTerm + rateTerm + integralTerm;
    const float limited = constrain(raw, -a.outputLimit, a.outputLimit);

    a.last.angle = angleTerm;
    a.last.rate = rateTerm;
    a.last.integral = integralTerm;
    a.last.output = limited;
    a.last.integrator = a.integrator;
    a.last.outputLimited = (limited != raw);
    return limited;
}
