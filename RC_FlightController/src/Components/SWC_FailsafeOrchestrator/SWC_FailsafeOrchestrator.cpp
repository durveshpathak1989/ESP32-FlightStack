#include "SWC_FailsafeOrchestrator.h"

#include <algorithm>

namespace {

float scaleMotor(float value, float scale)
{
    return std::max(0.0f, std::min(value * scale, 1.0f));
}

void zeroMotors(flight::MotorCommand& motors)
{
    motors.frontLeft = 0.0f;
    motors.frontRight = 0.0f;
    motors.rearLeft = 0.0f;
    motors.rearRight = 0.0f;
}

}  // namespace

SWC_FailsafeOrchestratorOutput SWC_FailsafeOrchestrator::update(
    const SWC_FailsafeOrchestratorInput& input)
{
    SWC_FailsafeOrchestratorOutput out;
    out.motors = input.motors;
    out.timestampUs = input.timestampUs;
    out.motors.timestampUs = input.timestampUs;

    if (input.failsafe.action == SWC_FailsafeLogicOutput::NONE) {
        return out;
    }

    out.modified = true;
    if (input.failsafe.action == SWC_FailsafeLogicOutput::DESCEND) {
        const float dt = static_cast<float>(input.dtUs) * 0.000001f;
        const float scale = std::max(0.0f, 1.0f - input.failsafe.descentRateTarget * dt);
        out.motors.frontLeft = scaleMotor(out.motors.frontLeft, scale);
        out.motors.frontRight = scaleMotor(out.motors.frontRight, scale);
        out.motors.rearLeft = scaleMotor(out.motors.rearLeft, scale);
        out.motors.rearRight = scaleMotor(out.motors.rearRight, scale);
        return out;
    }

    zeroMotors(out.motors);
    out.disarm = input.failsafe.action == SWC_FailsafeLogicOutput::DISARM ||
                 input.failsafe.action == SWC_FailsafeLogicOutput::LAND_IMMEDIATELY;
    return out;
}
