#pragma once

#include "Services/Navigation/SWC_FailsafeLogic.h"

class FailsafePolicy {
public:
    static bool isActive(const SWC_FailsafeLogicOutput& output)
    {
        return output.action != SWC_FailsafeLogicOutput::NONE;
    }

    static bool requiresDisarm(const SWC_FailsafeLogicOutput& output)
    {
        return output.action == SWC_FailsafeLogicOutput::DISARM ||
               output.action == SWC_FailsafeLogicOutput::LAND_IMMEDIATELY;
    }
};
