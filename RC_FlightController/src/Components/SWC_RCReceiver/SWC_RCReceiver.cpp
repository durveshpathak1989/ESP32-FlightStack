#include "SWC_RCReceiver.h"

#include <algorithm>

namespace {

constexpr std::uint16_t kThrottleMinUs = 1000;
constexpr std::uint16_t kThrottleMaxUs = 2000;
constexpr std::uint16_t kCenterUs = 1500;
constexpr std::uint16_t kDeadbandUs = 50;
constexpr std::uint16_t kArmThresholdUs = 1700;

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

float mapAxis(std::uint16_t raw)
{
    const int centered = static_cast<int>(raw) - static_cast<int>(kCenterUs);
    if (centered > -static_cast<int>(kDeadbandUs) &&
        centered < static_cast<int>(kDeadbandUs)) {
        return 0.0f;
    }
    return clampf(static_cast<float>(centered) / 500.0f, -1.0f, 1.0f);
}

float mapThrottle(std::uint16_t raw)
{
    return clampf(static_cast<float>(static_cast<int>(raw) - static_cast<int>(kThrottleMinUs)) /
                  static_cast<float>(kThrottleMaxUs - kThrottleMinUs),
                  0.0f,
                  1.0f);
}

}  // namespace

SWC_RCReceiverOutput SWC_RCReceiver::update(const SWC_RCReceiverInput& input)
{
    SWC_RCReceiverOutput out;
    out.timestampUs = input.timestampUs;
    for (std::uint8_t i = 0; i < SWC_RCReceiverInput::kChannelCount; ++i) {
        out.raw[i] = input.channels[i];
    }

    if (input.frameValid) {
        _lastValidFrameUs = input.timestampUs;
    }

    out.receiver.isConnected = _lastValidFrameUs != 0 &&
        (input.timestampUs >= _lastValidFrameUs) &&
        ((input.timestampUs - _lastValidFrameUs) <= 500000ULL);
    out.receiver.lastUpdateUs = _lastValidFrameUs;

    out.command.roll = mapAxis(input.channels[0]);
    out.command.pitch = mapAxis(input.channels[1]);
    out.command.throttle = mapThrottle(input.channels[2]);
    out.command.yaw = mapAxis(input.channels[3]);
    out.command.armSwitch = input.channels[6] >= kArmThresholdUs;
    out.command.armed = out.command.armSwitch;
    out.command.mode = input.channels[7] >= kArmThresholdUs ? 1 : 0;
    out.command.angleMode = out.command.mode == 0;
    out.command.valid = out.receiver.isConnected;
    out.command.timestampUs = input.timestampUs;
    return out;
}

void SWC_RCReceiver::reset()
{
    _lastValidFrameUs = 0;
}
