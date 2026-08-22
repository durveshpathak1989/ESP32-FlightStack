#pragma once

#include "Core/FlightTypes.h"
#include "Services/Control/SWC_PIDController.h"

class SWC_AttitudeController {
public:
    virtual ~SWC_AttitudeController() = default;
    virtual void init(const SWC_PIDControllerInput& input) = 0;
    virtual SWC_PIDControllerOutput update(const SWC_PIDControllerInput& input) = 0;
    virtual void reset() = 0;
};
