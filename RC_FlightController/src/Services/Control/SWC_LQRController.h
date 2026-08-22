#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"
#include "Services/Control/SWC_PIDController.h"

struct SWC_LQRControllerOutput {
    flight::MotorCommand motors;
    struct Debug {
        float stateCost = 0.0f;
        float inputCost = 0.0f;
        bool implemented = false;
    } debug;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_LQRController {
public:
    void init(const SWC_PIDControllerInput&) { reset(); }
    SWC_LQRControllerOutput update(const SWC_PIDControllerInput& input)
    {
        SWC_LQRControllerOutput out;
        out.timestampUs = input.timestampUs;
        out.motors.timestampUs = input.timestampUs;
        return out;
    }
    void reset() {}
};
