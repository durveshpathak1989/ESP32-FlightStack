#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"
#include "Services/Navigation/SWC_FailsafeLogic.h"

struct SWC_TelemetryInput {
    flight::AttitudeState attitude;
    flight::MotorCommand motors;
    flight::BatteryState battery;
    flight::GpsState gps;
    SWC_FailsafeLogicOutput failsafe;
    std::uint32_t loopJitterUs = 0;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_TelemetryOutput {
    bool shouldPublish = false;
    std::uint32_t sequence = 0;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_Telemetry {
public:
    void init(const SWC_TelemetryInput&) { reset(); }
    SWC_TelemetryOutput update(const SWC_TelemetryInput& input)
    {
        SWC_TelemetryOutput out;
        out.timestampUs = input.timestampUs;
        if (_lastPublishUs == 0 || input.timestampUs - _lastPublishUs >= _periodUs) {
            _lastPublishUs = input.timestampUs;
            out.sequence = ++_sequence;
            out.shouldPublish = true;
        } else {
            out.sequence = _sequence;
        }
        return out;
    }
    void setPeriodUs(std::uint32_t periodUs) { _periodUs = periodUs; }
    void reset()
    {
        _lastPublishUs = 0;
        _sequence = 0;
    }

private:
    std::uint64_t _lastPublishUs = 0;
    std::uint32_t _periodUs = 20000;
    std::uint32_t _sequence = 0;
};
