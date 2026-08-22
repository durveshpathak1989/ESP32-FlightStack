#include "ESP32Clock.h"

#include <Arduino.h>

std::uint64_t ESP32Clock::microseconds() const
{
    return static_cast<std::uint64_t>(micros());
}

std::uint32_t ESP32Clock::milliseconds() const
{
    return static_cast<std::uint32_t>(millis());
}
