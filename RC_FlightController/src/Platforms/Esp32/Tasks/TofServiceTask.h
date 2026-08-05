#pragma once

#include <cstdint>

#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

class TofServiceTask : public rte::SoftwareComponent {
public:
    TofServiceTask(rte::RangeServicePort& sensor,
                   SnapshotRte<FlightState>& rte, FlightState& state,
                   std::uint32_t periodMs)
        : sensor_(sensor), rte_(rte), state_(state), periodMs_(periodMs) {}

    void Init() override { sensor_.InitSensor(); }
    void Periodic() override {
        const rte::RangeServiceSample reading = sensor_.ReadRange();
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.tofReady = reading.ready;
            state_.tofValid = reading.valid;
            state_.tofDistance_mm = reading.distanceMm;
            state_.tofDistance_m = reading.distanceMm * 0.001f;
            state_.tofRangeStatus = reading.rangeStatus;
            state_.tofObjectCount = reading.objectCount;
            state_.tofStreamCount = reading.streamCount;
            state_.tofSignalMcps = reading.signalMcps;
            state_.tofAmbientMcps = reading.ambientMcps;
            state_.tofAge_ms = reading.ageMs;
            state_.tofLastUpdate_ms = reading.lastUpdateMs;
            rte_.unlock();
        }
    }

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(periodMs_);
        TickType_t lastWake = xTaskGetTickCount();
        Init();
        for (;;) {
            Periodic();
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    rte::RangeServicePort& sensor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    std::uint32_t periodMs_;
};
