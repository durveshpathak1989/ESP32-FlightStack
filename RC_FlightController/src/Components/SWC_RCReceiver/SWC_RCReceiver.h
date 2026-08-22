#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_RCReceiverInput {
    static constexpr std::uint8_t kChannelCount = 10;

    std::uint16_t channels[kChannelCount] = {
        1500, 1500, 1000, 1500, 1000, 1000, 1000, 1000, 1000, 1000
    };
    bool frameValid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_RCReceiverOutput {
    flight::PilotCommand command;
    flight::ReceiverState receiver;
    std::uint16_t raw[SWC_RCReceiverInput::kChannelCount] = {};
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_RCReceiver {
public:
    void init(const SWC_RCReceiverInput&) { reset(); }
    SWC_RCReceiverOutput update(const SWC_RCReceiverInput& input);
    void reset();

private:
    std::uint64_t _lastValidFrameUs = 0;
};
