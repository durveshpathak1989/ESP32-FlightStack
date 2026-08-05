#pragma once

#include "Arduino.h"
#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/DebugConfig/DebugConfig.h"
#include "../../../Submodules/GPS/GPSSensor.h"

class GpsServiceTask {
public:
    GpsServiceTask(GPSSensor& sensor, SnapshotRte<FlightState>& rte,
                   FlightState& state)
        : sensor_(sensor), rte_(rte), state_(state) {}

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(20);
        TickType_t lastWake = xTaskGetTickCount();
        std::uint32_t lastPrintMs = 0;
        for (;;) {
            sensor_.update();
            const GPSData data = sensor_.get();
            if (rte_.lock(pdMS_TO_TICKS(2))) {
                state_.gps = data;
                rte_.unlock();
            }
            if (millis() - lastPrintMs >= 5000) {
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
                lastPrintMs = millis();
            }
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    GPSSensor& sensor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
};
