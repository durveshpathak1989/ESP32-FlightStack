#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_MotorOutputInput {
    flight::MotorCommand motors;
    bool isArmed = false;
    std::uint64_t dtUs = 2500;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_MotorOutputOutput {
    struct Debug {
        std::uint32_t pwmFL = 1000;
        std::uint32_t pwmFR = 1000;
        std::uint32_t pwmRL = 1000;
        std::uint32_t pwmRR = 1000;
        std::uint32_t lastUpdateUs = 0;
        bool shouldWrite = true;
    } debug;
    std::uint8_t version = 1;
};

class SWC_MotorOutput {
public:
    void init(const SWC_MotorOutputInput&) { reset(); }
    SWC_MotorOutputOutput update(const SWC_MotorOutputInput& input);
    void reset();

private:
    std::uint64_t _lastWriteUs = 0;
    SWC_MotorOutputOutput::Debug _lastDebug;
};
