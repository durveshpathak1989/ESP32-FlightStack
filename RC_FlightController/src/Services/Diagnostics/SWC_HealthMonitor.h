#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_HealthMonitorInput {
    flight::AttitudeState attitude;
    flight::BatteryState battery;
    flight::ReceiverState receiver;
    std::uint32_t freeHeapBytes = 0;
    std::uint32_t minFreeHeapBytes = 0;
    std::uint32_t loopJitterUs = 0;
    bool imuFresh = true;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_HealthMonitorOutput {
    bool healthy = true;
    bool imuFault = false;
    bool batteryFault = false;
    bool receiverFault = false;
    bool timingFault = false;
    bool heapFault = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_HealthMonitor {
public:
    void init(const SWC_HealthMonitorInput&) {}
    SWC_HealthMonitorOutput update(const SWC_HealthMonitorInput& input)
    {
        SWC_HealthMonitorOutput out;
        out.timestampUs = input.timestampUs;
        out.imuFault = !input.imuFresh;
        out.batteryFault = input.battery.status == flight::BatteryState::CRITICAL ||
                           input.battery.status == flight::BatteryState::DISCONNECTED;
        out.receiverFault = !input.receiver.isConnected;
        out.timingFault = input.loopJitterUs > 150U;
        out.heapFault = input.minFreeHeapBytes != 0U && input.freeHeapBytes < input.minFreeHeapBytes;
        out.healthy = !(out.imuFault || out.batteryFault || out.receiverFault ||
                        out.timingFault || out.heapFault);
        return out;
    }
    void reset() {}
};
