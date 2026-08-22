#include "SWC_PIDController.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = flight::FlightConstants::kPi;
constexpr float kRadToDeg = flight::FlightConstants::kRadToDeg;
constexpr float kDegToRad = flight::FlightConstants::kDegToRad;

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

float wrapDeg180(float deg)
{
    while (deg > 180.0f) deg -= 360.0f;
    while (deg <= -180.0f) deg += 360.0f;
    return deg;
}

float smoothStep01(float x)
{
    x = clampf(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

float throttleExpo(float x, float expo)
{
    x = clampf(x, 0.0f, 1.0f);
    expo = clampf(expo, 0.0f, 0.95f);
    return (1.0f - expo) * x + expo * x * x * x;
}

flight::MotorCommand makeMotorCommand(float fl, float fr, float rl, float rr, std::uint64_t timestampUs)
{
    flight::MotorCommand command;
    command.frontLeft = fl;
    command.frontRight = fr;
    command.rearLeft = rl;
    command.rearRight = rr;
    command.timestampUs = timestampUs;
    return command;
}

}  // namespace

float SWC_PIDController::AxisPid::update(float error, float dt)
{
    integral = clampf(integral + error * dt, -iLimit, iLimit);
    const float errorRate = dt > 0.000001f ? (error - prevError) / dt : 0.0f;
    prevError = error;
    lastP = kp * error;
    lastI = ki * integral;
    lastD = kd * errorRate;
    lastOut = lastP + lastI + lastD;
    return lastOut;
}

float SWC_PIDController::AxisPid::updateDOnMeasurement(float error,
                                                       float measurement,
                                                       float dt,
                                                       float derivativeLpfHz)
{
    integral = clampf(integral + error * dt, -iLimit, iLimit);
    float measurementRate = 0.0f;
    if (hasPrevMeasurement && dt > 0.000001f) {
        measurementRate = (measurement - prevMeasurement) / dt;
    }

    if (derivativeLpfHz > 0.0f && dt > 0.000001f) {
        const float tau = 1.0f / (2.0f * kPi * derivativeLpfHz);
        const float alpha = clampf(dt / (tau + dt), 0.0f, 1.0f);
        if (!hasFilteredMeasurementRate) {
            filteredMeasurementRate = measurementRate;
            hasFilteredMeasurementRate = true;
        } else {
            filteredMeasurementRate += alpha * (measurementRate - filteredMeasurementRate);
        }
    } else {
        filteredMeasurementRate = measurementRate;
        hasFilteredMeasurementRate = true;
    }

    prevError = error;
    prevMeasurement = measurement;
    hasPrevMeasurement = true;
    lastP = kp * error;
    lastI = ki * integral;
    lastD = -kd * filteredMeasurementRate;
    lastOut = lastP + lastI + lastD;
    return lastOut;
}

void SWC_PIDController::AxisPid::reset()
{
    integral = 0.0f;
    prevError = 0.0f;
    prevMeasurement = 0.0f;
    filteredMeasurementRate = 0.0f;
    hasPrevMeasurement = false;
    hasFilteredMeasurementRate = false;
    lastP = 0.0f;
    lastI = 0.0f;
    lastD = 0.0f;
    lastOut = 0.0f;
}

float SWC_PIDController::LowPass::apply(float x, float dt, float cutoffHz)
{
    if (dt <= 0.0f || cutoffHz <= 0.0f) return x;
    if (!initialized) {
        y = x;
        initialized = true;
        return y;
    }
    const float rc = 1.0f / (2.0f * kPi * cutoffHz);
    const float alpha = dt / (dt + rc);
    y += alpha * (x - y);
    return y;
}

void SWC_PIDController::LowPass::reset()
{
    y = 0.0f;
    initialized = false;
}

void SWC_PIDController::init(const SWC_PIDControllerInput& initial)
{
    applyConfig(initial.config);
    reset();
}

void SWC_PIDController::applyConfig(const flight::FlightConfig& config)
{
    _rateRoll.kp = config.pidRateRoll_Kp;
    _rateRoll.ki = config.pidRateRoll_Ki;
    _rateRoll.kd = config.pidRateRoll_Kd;
    _ratePitch.kp = config.pidRatePitch_Kp;
    _ratePitch.ki = config.pidRatePitch_Ki;
    _ratePitch.kd = config.pidRatePitch_Kd;
    _rateYaw.kp = config.pidRateYaw_Kp;
    _rateYaw.ki = config.pidRateYaw_Ki;
    _rateYaw.kd = config.pidRateYaw_Kd;

    _angleRoll.kp = config.pidAttRoll_Kp;
    _angleRoll.ki = config.pidAttRoll_Ki;
    _angleRoll.kd = config.pidAttRoll_Kd;
    _anglePitch.kp = config.pidAttPitch_Kp;
    _anglePitch.ki = config.pidAttPitch_Ki;
    _anglePitch.kd = config.pidAttPitch_Kd;
    _angleYaw.kp = config.pidAttYaw_Kp;
    _angleYaw.ki = config.pidAttYaw_Ki;
    _angleYaw.kd = config.pidAttYaw_Kd;

    _rateRoll.iLimit = config.pidIntegralLimit;
    _ratePitch.iLimit = config.pidIntegralLimit;
    _rateYaw.iLimit = config.pidIntegralLimit;
    _angleRoll.iLimit = config.pidIntegralLimit;
    _anglePitch.iLimit = config.pidIntegralLimit;
    _angleYaw.iLimit = config.pidIntegralLimit;
}

SWC_PIDControllerOutput SWC_PIDController::update(const SWC_PIDControllerInput& input)
{
    applyConfig(input.config);

    SWC_PIDControllerOutput out;
    out.timestampUs = input.timestampUs;
    out.motors = makeMotorCommand(0.0f, 0.0f, 0.0f, 0.0f, input.timestampUs);

    if (!input.isArmed) {
        reset();
        return out;
    }

    float dt = static_cast<float>(input.dtUs) * 0.000001f;
    if (!std::isfinite(dt) || dt <= 0.0f || dt > 0.05f) {
        dt = flight::FlightConstants::kDefaultControlPeriodS;
    }

    const flight::FlightConfig& cfg = input.config;
    const float rollDeg = input.imuValid ? input.attitude.roll * kRadToDeg : 0.0f;
    const float pitchDeg = input.imuValid ? input.attitude.pitch * kRadToDeg : 0.0f;
    const float yawDeg = input.imuValid ? input.attitude.yaw * kRadToDeg : 0.0f;
    const float gxDps = input.imuValid ? input.attitude.rollRate * kRadToDeg : 0.0f;
    const float gyDps = input.imuValid ? input.attitude.pitchRate * kRadToDeg : 0.0f;
    const float gzDps = input.imuValid ? input.attitude.yawRate * kRadToDeg : 0.0f;

    const float rollCmd = _rollSetpoint.apply(input.pilot.roll, dt, 70.0f);
    const float pitchCmd = _pitchSetpoint.apply(input.pilot.pitch, dt, 70.0f);
    const float yawCmd = _yawSetpointFilter.apply(input.pilot.yaw, dt, 70.0f);

    float rO = 0.0f;
    float pO = 0.0f;
    float yO = 0.0f;
    float targetRollRateDps = 0.0f;
    float targetPitchRateDps = 0.0f;
    float targetYawRateDps = 0.0f;
    float targetRollDeg = 0.0f;
    float targetPitchDeg = 0.0f;
    float targetYawDeg = 0.0f;
    float angleErrRollDeg = 0.0f;
    float angleErrPitchDeg = 0.0f;
    float yawErrDeg = 0.0f;
    float rateErrRollDps = 0.0f;
    float rateErrPitchDps = 0.0f;
    float rateErrYawDps = 0.0f;

    if (input.angleMode) {
        targetRollDeg = rollCmd * cfg.maxAngleDeg;
        targetPitchDeg = pitchCmd * cfg.maxAngleDeg;
        angleErrRollDeg = targetRollDeg - rollDeg;
        angleErrPitchDeg = targetPitchDeg - pitchDeg;
        targetRollRateDps = _angleRoll.update(angleErrRollDeg, dt);
        targetPitchRateDps = _anglePitch.update(angleErrPitchDeg, dt);
    } else {
        targetRollRateDps = rollCmd * cfg.maxRateDps;
        targetPitchRateDps = pitchCmd * cfg.maxPitchRateDps;
    }

    rateErrRollDps = targetRollRateDps - gxDps;
    rateErrPitchDps = targetPitchRateDps - gyDps;
    rO = cfg.pidRateRoll_Ff * targetRollRateDps +
         _rateRoll.updateDOnMeasurement(rateErrRollDps, gxDps, dt, cfg.pidRateRoll_DLpfHz);
    pO = cfg.pidRatePitch_Ff * targetPitchRateDps +
         _ratePitch.updateDOnMeasurement(rateErrPitchDps, gyDps, dt, cfg.pidRatePitch_DLpfHz);

    if (input.imuValid && std::fabs(yawCmd) < cfg.yawDeadband) {
        if (!_yawHoldActive) {
            _yawSetpointDeg = yawDeg;
            _yawHoldActive = true;
            _angleYaw.reset();
        }
        targetYawDeg = _yawSetpointDeg;
        yawErrDeg = wrapDeg180(_yawSetpointDeg - yawDeg);
        targetYawRateDps = _angleYaw.update(yawErrDeg, dt);
        targetYawRateDps = clampf(targetYawRateDps, -cfg.yawMaxRateDps, cfg.yawMaxRateDps);
    } else {
        _yawHoldActive = false;
        targetYawDeg = yawDeg;
        targetYawRateDps = -yawCmd * cfg.yawMaxRateDps;
    }

    rateErrYawDps = targetYawRateDps - gzDps;
    yO = cfg.pidRateYaw_Ff * targetYawRateDps +
         _rateYaw.updateDOnMeasurement(rateErrYawDps, gzDps, dt, cfg.pidRateYaw_DLpfHz);

    const bool rollOutputLimited = (rO > cfg.rollOutputLimit) || (rO < -cfg.rollOutputLimit);
    const bool pitchOutputLimited = (pO > cfg.pitchOutputLimit) || (pO < -cfg.pitchOutputLimit);
    const bool yawOutputLimited = (yO > cfg.yawOutputLimit) || (yO < -cfg.yawOutputLimit);

    rO = clampf(rO, -cfg.rollOutputLimit, cfg.rollOutputLimit);
    pO = clampf(pO, -cfg.pitchOutputLimit, cfg.pitchOutputLimit);
    yO = clampf(yO, -cfg.yawOutputLimit, cfg.yawOutputLimit);

    const float throttleRaw = clampf(input.pilot.throttle, 0.0f, 1.0f);
    float throttleTarget = 0.0f;
    if (throttleRaw > cfg.throttleCut) {
        throttleTarget = clampf(throttleExpo(throttleRaw, cfg.throttleExpo), 0.0f, 1.0f);
    }

    const float maxStepUp = cfg.throttleUpRatePerSec * dt;
    const float maxStepDown = cfg.throttleDownRatePerSec * dt;
    if (throttleTarget > _smoothedThrottle) {
        _smoothedThrottle += std::min(throttleTarget - _smoothedThrottle, maxStepUp);
    } else {
        _smoothedThrottle -= std::min(_smoothedThrottle - throttleTarget, maxStepDown);
    }
    const float throttle = clampf(_smoothedThrottle, 0.0f, 1.0f);

    float fl = throttle + rO - pO - yO;
    float fr = throttle - rO - pO + yO;
    float rl = throttle + rO + pO + yO;
    float rr = throttle - rO + pO - yO;

    bool motorSaturated = rollOutputLimited || pitchOutputLimited || yawOutputLimited;
    const float maxMotor = std::max(std::max(fl, fr), std::max(rl, rr));
    if (maxMotor > cfg.motorMax) {
        motorSaturated = true;
        const float excess = maxMotor - cfg.motorMax;
        fl -= excess;
        fr -= excess;
        rl -= excess;
        rr -= excess;
    }

    const float idleDenominator = cfg.idleRampEnd - cfg.throttleCut;
    const float idleBlend = smoothStep01(idleDenominator > 0.000001f
        ? (throttle - cfg.throttleCut) / idleDenominator
        : 1.0f);
    const float motorMin = cfg.motorIdle * idleBlend;

    if (throttle > cfg.throttleCut) {
        fl = clampf(fl, motorMin, cfg.motorMax);
        fr = clampf(fr, motorMin, cfg.motorMax);
        rl = clampf(rl, motorMin, cfg.motorMax);
        rr = clampf(rr, motorMin, cfg.motorMax);
    } else {
        fl = 0.0f;
        fr = 0.0f;
        rl = 0.0f;
        rr = 0.0f;
    }

    out.motors = makeMotorCommand(fl, fr, rl, rr, input.timestampUs);
    out.debug.targetRollDeg = targetRollDeg;
    out.debug.targetPitchDeg = targetPitchDeg;
    out.debug.targetYawDeg = targetYawDeg;
    out.debug.targetRollRateDps = targetRollRateDps;
    out.debug.targetPitchRateDps = targetPitchRateDps;
    out.debug.targetYawRateDps = targetYawRateDps;
    out.debug.attErrRoll = angleErrRollDeg;
    out.debug.attErrPitch = angleErrPitchDeg;
    out.debug.attErrYaw = yawErrDeg;
    out.debug.attPRoll = _angleRoll.lastP;
    out.debug.attIRoll = _angleRoll.lastI;
    out.debug.attDRoll = _angleRoll.lastD;
    out.debug.attPPitch = _anglePitch.lastP;
    out.debug.attIPitch = _anglePitch.lastI;
    out.debug.attDPitch = _anglePitch.lastD;
    out.debug.attPYaw = _angleYaw.lastP;
    out.debug.attIYaw = _angleYaw.lastI;
    out.debug.attDYaw = _angleYaw.lastD;
    out.debug.rateErrRoll = rateErrRollDps;
    out.debug.rateErrPitch = rateErrPitchDps;
    out.debug.rateErrYaw = rateErrYawDps;
    out.debug.ratePRoll = _rateRoll.lastP;
    out.debug.rateIRoll = _rateRoll.lastI;
    out.debug.rateDRoll = _rateRoll.lastD;
    out.debug.ratePPitch = _ratePitch.lastP;
    out.debug.rateIPitch = _ratePitch.lastI;
    out.debug.rateDPitch = _ratePitch.lastD;
    out.debug.ratePYaw = _rateYaw.lastP;
    out.debug.rateIYaw = _rateYaw.lastI;
    out.debug.rateDYaw = _rateYaw.lastD;
    out.debug.rollOutput = rO;
    out.debug.pitchOutput = pO;
    out.debug.yawOutput = yO;
    out.debug.throttle = throttle;
    out.debug.yawHoldActive = _yawHoldActive;
    out.debug.motorSaturated = motorSaturated;
    (void)kDegToRad;
    return out;
}

void SWC_PIDController::reset()
{
    _rateRoll.reset();
    _ratePitch.reset();
    _rateYaw.reset();
    _angleRoll.reset();
    _anglePitch.reset();
    _angleYaw.reset();
    _rollSetpoint.reset();
    _pitchSetpoint.reset();
    _yawSetpointFilter.reset();
    _yawSetpointDeg = 0.0f;
    _smoothedThrottle = 0.0f;
    _yawHoldActive = false;
}
