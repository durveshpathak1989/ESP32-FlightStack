#pragma once

#include <cstdint>

#include "../../Core/RtePorts.h"

class FlightControlSwc final : public rte::SoftwareComponent {
public:
    void Init() override;
    void Periodic() override;
    void SetActivationCount(std::uint32_t releases) { releases_ = releases; }

private:
    std::uint32_t releases_ = 1;
};
