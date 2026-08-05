#pragma once

#include <cstdint>

#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/ToF/FlightToF_VL53L4CX.h"
#include "../../../Core/RtePorts.h"

class TofServiceTask : public rte::SoftwareComponent {
public:
    TofServiceTask(FlightToF_VL53L4CX& sensor, SemaphoreHandle_t& i2cMutex,
                   SnapshotRte<FlightState>& rte, FlightState& state,
                   std::uint32_t periodMs, std::uint32_t staleMs)
        : sensor_(sensor), i2cMutex_(i2cMutex), rte_(rte), state_(state),
          periodMs_(periodMs), staleMs_(staleMs) {}

    void Init() override {}
    void Periodic() override {
        bool ready = sensor_.isReady();
        if (ready && xSemaphoreTake(i2cMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
            sensor_.update();
            ready = sensor_.isReady();
            xSemaphoreGive(i2cMutex_);
        }
        const FlightToFReading reading = sensor_.reading();
        const std::uint32_t ageMs = ready ? sensor_.ageMs() : UINT32_MAX;
        const bool valid = ready && reading.valid && ageMs <= staleMs_;
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.tofReady = ready;
            state_.tofValid = valid;
            state_.tofDistance_mm = reading.distanceMm;
            state_.tofDistance_m = reading.distanceMm * 0.001f;
            state_.tofRangeStatus = reading.rangeStatus;
            state_.tofObjectCount = reading.objectCount;
            state_.tofStreamCount = reading.streamCount;
            state_.tofSignalMcps = reading.signalMcps;
            state_.tofAmbientMcps = reading.ambientMcps;
            state_.tofAge_ms = ageMs;
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
    FlightToF_VL53L4CX& sensor_;
    SemaphoreHandle_t& i2cMutex_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    std::uint32_t periodMs_;
    std::uint32_t staleMs_;
};
