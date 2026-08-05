#pragma once

#include <cstdint>

#include "../../Core/RtePorts.h"

class SerialDiagnosticsSwc final : public rte::SoftwareComponent {
public:
    void Init() override;
    void Periodic() override;

private:
    std::uint32_t tick_ = 0;
};
