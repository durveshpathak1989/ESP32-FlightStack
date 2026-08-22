#pragma once

#include "Services/Control/SWC_PIDController.h"

class SWC_PIDControl {
public:
    void init(const SWC_PIDControllerInput& input) { _controller.init(input); }
    SWC_PIDControllerOutput update(const SWC_PIDControllerInput& input)
    {
        return _controller.update(input);
    }
    void reset() { _controller.reset(); }

private:
    SWC_PIDController _controller;
};
