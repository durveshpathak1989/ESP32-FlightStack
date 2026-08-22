#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"
#include "Services/Navigation/SWC_FailsafeLogic.h"

struct SWC_FailsafeOrchestratorInput {
    flight::MotorCommand motors;
    SWC_FailsafeLogicOutput failsafe;
    std::uint64_t dtUs = 100000;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_FailsafeOrchestratorOutput {
    flight::MotorCommand motors;
    bool disarm = false;
    bool modified = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_FailsafeOrchestrator {
public:
    void init(const SWC_FailsafeOrchestratorInput&) {}
    SWC_FailsafeOrchestratorOutput update(const SWC_FailsafeOrchestratorInput& input);
    void reset() {}
};
