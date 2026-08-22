#include "SWC_MotorOutput.h"

#include <algorithm>

namespace {

constexpr std::uint32_t kMinPwmUs = 1000;
constexpr std::uint32_t kMaxPwmUs = 2000;
constexpr std::uint64_t kWritePeriodUs = 5000;

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

std::uint32_t pwmFromThrottle(float throttle)
{
    throttle = clampf(throttle, 0.0f, 1.0f);
    return static_cast<std::uint32_t>(kMinPwmUs + throttle * static_cast<float>(kMaxPwmUs - kMinPwmUs));
}

}  // namespace

SWC_MotorOutputOutput SWC_MotorOutput::update(const SWC_MotorOutputInput& input)
{
    SWC_MotorOutputOutput out;
    const bool firstWrite = _lastWriteUs == 0;
    const bool due = firstWrite ||
        (input.timestampUs >= _lastWriteUs && (input.timestampUs - _lastWriteUs) >= kWritePeriodUs);

    if (!due) {
        out.debug = _lastDebug;
        out.debug.shouldWrite = false;
        return out;
    }

    const float maxThrottle = std::max(std::max(input.motors.frontLeft, input.motors.frontRight),
                                       std::max(input.motors.rearLeft, input.motors.rearRight));
    if (!input.isArmed || maxThrottle <= 0.05f) {
        out.debug.pwmFL = kMinPwmUs;
        out.debug.pwmFR = kMinPwmUs;
        out.debug.pwmRL = kMinPwmUs;
        out.debug.pwmRR = kMinPwmUs;
    } else {
        out.debug.pwmFL = pwmFromThrottle(input.motors.frontLeft);
        out.debug.pwmFR = pwmFromThrottle(input.motors.frontRight);
        out.debug.pwmRL = pwmFromThrottle(input.motors.rearLeft);
        out.debug.pwmRR = pwmFromThrottle(input.motors.rearRight);
    }

    _lastWriteUs = input.timestampUs;
    out.debug.lastUpdateUs = static_cast<std::uint32_t>(input.timestampUs);
    out.debug.shouldWrite = true;
    _lastDebug = out.debug;
    return out;
}

void SWC_MotorOutput::reset()
{
    _lastWriteUs = 0;
    _lastDebug = SWC_MotorOutputOutput::Debug{};
}
