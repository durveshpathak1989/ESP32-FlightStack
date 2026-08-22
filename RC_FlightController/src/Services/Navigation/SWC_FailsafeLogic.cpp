#include "SWC_FailsafeLogic.h"

#include <algorithm>

namespace {

std::uint32_t atLeast(std::uint32_t value, std::uint32_t minimum)
{
    return std::max(value, minimum);
}

std::uint64_t elapsedMs(std::uint64_t nowUs, std::uint64_t thenUs)
{
    return nowUs >= thenUs ? (nowUs - thenUs) / 1000ULL : 0ULL;
}

}  // namespace

void SWC_FailsafeLogic::init(const SWC_FailsafeLogicInput&)
{
    reset();
}

SWC_FailsafeLogicOutput SWC_FailsafeLogic::update(const SWC_FailsafeLogicInput& input)
{
    SWC_FailsafeLogicOutput out;
    out.timestampUs = input.timestampUs;

    if (input.battery.status == flight::BatteryState::DISCONNECTED) {
        return out;
    }

    if (input.battery.voltage > 0.0f &&
        input.battery.voltage < input.config.batteryCriticalVoltage) {
        out.action = SWC_FailsafeLogicOutput::LAND_IMMEDIATELY;
        out.reason = SWC_FailsafeLogicOutput::BATTERY_CRITICAL;
        return out;
    }

    if (input.battery.voltage > 0.0f &&
        input.battery.voltage < input.config.batteryLowVoltage) {
        if (!_batteryLowLatched) {
            _batteryLowLatched = true;
            _batteryLowSinceUs = input.timestampUs;
        }
    } else if (input.battery.voltage > input.config.batteryOkVoltage) {
        _batteryLowLatched = false;
        _batteryLowSinceUs = 0;
    }

    if (_batteryLowLatched && elapsedMs(input.timestampUs, _batteryLowSinceUs) >= 1000ULL) {
        out.action = SWC_FailsafeLogicOutput::DESCEND;
        out.reason = SWC_FailsafeLogicOutput::BATTERY_LOW;
        out.descentRateTarget = 0.5f;
        return out;
    }

    const std::uint32_t rcTimeoutMs = atLeast(input.config.rcLossTimeoutMs, 2000U);
    if (!input.receiver.isConnected ||
        elapsedMs(input.timestampUs, input.receiver.lastUpdateUs) > rcTimeoutMs) {
        out.action = SWC_FailsafeLogicOutput::DESCEND;
        out.reason = SWC_FailsafeLogicOutput::RC_LOSS_TIMEOUT;
        out.descentRateTarget = 0.3f;
        return out;
    }

    const std::uint32_t gpsTimeoutMs = atLeast(input.config.gpsLossTimeoutMs, 5000U);
    if (input.gps.gpsRequired &&
        (!input.gps.hasGpsFix || elapsedMs(input.timestampUs, input.gps.lastFixUpdateUs) > gpsTimeoutMs)) {
        out.action = SWC_FailsafeLogicOutput::DESCEND;
        out.reason = SWC_FailsafeLogicOutput::GPS_LOSS_TIMEOUT;
        out.descentRateTarget = 0.2f;
        return out;
    }

    return out;
}

void SWC_FailsafeLogic::reset()
{
    _batteryLowLatched = false;
    _batteryLowSinceUs = 0;
}
