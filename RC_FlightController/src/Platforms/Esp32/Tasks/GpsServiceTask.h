#pragma once

#include "Arduino.h"
#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/DebugConfig/DebugConfig.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

class GpsServiceTask : public rte::SoftwareComponent {
public:
    GpsServiceTask(rte::GpsServicePort& sensor, SnapshotRte<FlightState>& rte,
                   FlightState& state)
        : sensor_(sensor), rte_(rte), state_(state) {}

    void Init() override {
        lastPrintMs_ = 0;
        sensor_.InitReceiver();
    }

    void Periodic() override {
        const rte::GpsServiceSample data = sensor_.ReadPosition();
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.gps.latitude = data.latitude;
            state_.gps.longitude = data.longitude;
            state_.gps.altitude_m = data.altitudeM;
            state_.gps.geoid_sep_m = data.geoidSeparationM;
            state_.gps.speed_kmh = data.speedKmh;
            state_.gps.speed_knots = data.speedKnots;
            state_.gps.course_deg = data.courseDeg;
            state_.gps.fix_quality = data.fixQuality;
            state_.gps.satellites = data.satellites;
            state_.gps.hdop = data.hdop;
            state_.gps.hour = data.hour;
            state_.gps.minute = data.minute;
            state_.gps.second = data.second;
            state_.gps.day = data.day;
            state_.gps.month = data.month;
            state_.gps.year = data.year;
            state_.gps.valid = data.valid;
            state_.gps.hasFix = data.hasFix;
            state_.gps.lastFixMs = data.lastFixMs;
            state_.gps.sentenceCount = data.sentenceCount;
            rte_.unlock();
        }
        if (millis() - lastPrintMs_ < 5000) return;
        if (data.valid) {
            DBG_PRINTF("[GPS] Fix  Lat=%.6f  Lon=%.6f  Alt=%.1fm  Sats=%d"
                       "  HDOP=%.1f  Speed=%.1fkm/h  UTC=%02d:%02d:%02d\n",
                       data.latitude, data.longitude, data.altitudeM,
                       data.satellites, data.hdop, data.speedKmh,
                       data.hour, data.minute, data.second);
        } else {
            DBG_PRINTF("[GPS] No fix  Sats=%d  Sentences=%lu\n",
                       data.satellites,
                       static_cast<unsigned long>(data.sentenceCount));
        }
        lastPrintMs_ = millis();
    }

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(20);
        TickType_t lastWake = xTaskGetTickCount();
        Init();
        for (;;) {
            Periodic();
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    rte::GpsServicePort& sensor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    std::uint32_t lastPrintMs_ = 0;
};
