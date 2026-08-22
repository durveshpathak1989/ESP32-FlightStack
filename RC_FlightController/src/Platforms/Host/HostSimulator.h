#pragma once

#include <cstdint>

#include "Application/FlightApplication.h"

class HostSimulator {
public:
    FlightApplicationOutput step(const FlightApplicationInput& input)
    {
        return _application.update(input);
    }

    void reset()
    {
        _application.reset();
        _timeUs = 0;
    }

    std::uint64_t advanceUs(std::uint64_t deltaUs)
    {
        _timeUs += deltaUs;
        return _timeUs;
    }

private:
    FlightApplication _application;
    std::uint64_t _timeUs = 0;
};
