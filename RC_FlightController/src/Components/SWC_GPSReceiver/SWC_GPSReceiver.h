#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"

struct SWC_GPSReceiverInput {
    double latitude = 0.0;
    double longitude = 0.0;
    float altitude = 0.0f;
    float groundSpeedMps = 0.0f;
    float courseDeg = 0.0f;
    std::uint8_t satellites = 0;
    float hdop = 99.9f;
    bool valid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_GPSReceiverOutput {
    flight::GpsState gps;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_GPSReceiver {
public:
    void init(const SWC_GPSReceiverInput&) {}
    SWC_GPSReceiverOutput update(const SWC_GPSReceiverInput& input);
    void reset() {}
};
