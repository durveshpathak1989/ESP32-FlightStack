#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_FailsafeLogicInput {
    flight::BatteryState battery;
    flight::ReceiverState receiver;
    struct GpsMonitor {
        bool hasGpsFix = false;
        bool gpsRequired = false;
        std::uint64_t lastFixUpdateUs = 0;
    } gps;
    std::uint64_t armTimeUs = 0;
    flight::FlightConfig config;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_FailsafeLogicOutput {
    enum Action : std::uint8_t {
        NONE = 0,
        DESCEND = 1,
        LAND_IMMEDIATELY = 2,
        DISARM = 3
    };

    enum Reason : std::uint8_t {
        REASON_NONE = 0,
        BATTERY_CRITICAL = 1,
        BATTERY_LOW = 2,
        RC_LOSS_TIMEOUT = 3,
        GPS_LOSS_TIMEOUT = 4
    };

    Action action = NONE;
    Reason reason = REASON_NONE;
    float descentRateTarget = 0.0f;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_FailsafeLogic {
public:
    void init(const SWC_FailsafeLogicInput& initial);
    SWC_FailsafeLogicOutput update(const SWC_FailsafeLogicInput& input);
    void reset();

private:
    bool _batteryLowLatched = false;
    std::uint64_t _batteryLowSinceUs = 0;
};
