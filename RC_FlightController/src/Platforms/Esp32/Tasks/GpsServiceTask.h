#pragma once

#include "Arduino.h"
#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/DebugConfig/DebugConfig.h"
#include "../../../Submodules/GPS/GPSSensor.h"
#include "../../../Core/RtePorts.h"

class GpsServiceTask : public rte::SoftwareComponent {
public:
    GpsServiceTask(GPSSensor& sensor, SnapshotRte<FlightState>& rte,
                   FlightState& state)
        : sensor_(sensor), rte_(rte), state_(state) {}

    void Init() override { lastPrintMs_ = 0; }

    void Periodic() override {
        sensor_.update();
        const GPSData data = sensor_.get();
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.gps = data;
            rte_.unlock();
        }
        if (millis() - lastPrintMs_ < 5000) return;
        if (data.valid) {
            DBG_PRINTF("[GPS] Fix  Lat=%.6f  Lon=%.6f  Alt=%.1fm  Sats=%d"
                       "  HDOP=%.1f  Speed=%.1fkm/h  UTC=%02d:%02d:%02d\n",
                       data.latitude, data.longitude, data.altitude_m,
                       data.satellites, data.hdop, data.speed_kmh,
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
    GPSSensor& sensor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    std::uint32_t lastPrintMs_ = 0;
};
