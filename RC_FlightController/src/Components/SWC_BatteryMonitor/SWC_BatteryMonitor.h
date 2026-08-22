#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_BatteryMonitorInput {
    std::uint16_t rawAdcCount = 0;
    std::uint32_t adcMillivolts = 0;
    float currentA = 0.0f;
    float capacityMah = 0.0f;
    std::uint8_t cellCount = 3;
    float dividerScale = 7.7142857f;
    float fallbackVoltage = 11.1f;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_BatteryMonitorOutput {
    flight::BatteryState battery;
    float adcVoltage = 0.0f;
    float cellVoltage = 0.0f;
    float percent = 0.0f;
    bool valid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_BatteryMonitor {
public:
    void init(const SWC_BatteryMonitorInput&) { reset(); }
    SWC_BatteryMonitorOutput update(const SWC_BatteryMonitorInput& input);
    void reset();

private:
    float _filteredVoltage = 11.1f;
    bool _haveFiltered = false;
};
