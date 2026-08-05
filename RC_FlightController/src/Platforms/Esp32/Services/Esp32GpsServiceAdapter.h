#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/GPS/GPSSensor.h"

class Esp32GpsServiceAdapter : public rte::GpsServicePort {
public:
    Esp32GpsServiceAdapter(GPSSensor& sensor, std::uint8_t rxPin,
                           std::uint8_t txPin, std::uint32_t baud)
        : sensor_(sensor), rxPin_(rxPin), txPin_(txPin), baud_(baud) {}

    bool InitReceiver() override {
        sensor_.begin(rxPin_, txPin_, baud_);
        return true;
    }
    rte::GpsServiceSample ReadPosition() override {
        sensor_.update();
        const GPSData value = sensor_.get();
        return {value.latitude, value.longitude, value.altitude_m,
                value.geoid_sep_m, value.speed_kmh, value.speed_knots,
                value.course_deg, value.hdop,
                value.sentenceCount, value.satellites, value.fix_quality,
                value.hour, value.minute, value.second,
                value.day, value.month, value.year, value.lastFixMs,
                value.hasFix, value.valid};
    }

private:
    GPSSensor& sensor_;
    std::uint8_t rxPin_;
    std::uint8_t txPin_;
    std::uint32_t baud_;
};
