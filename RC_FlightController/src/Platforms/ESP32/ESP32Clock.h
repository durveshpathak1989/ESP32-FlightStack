#pragma once

#include <cstdint>

#include "Core/Ports.h"

class ESP32Clock final : public flight::ClockPort {
public:
    std::uint64_t microseconds() const override;
    std::uint32_t milliseconds() const override;
};
