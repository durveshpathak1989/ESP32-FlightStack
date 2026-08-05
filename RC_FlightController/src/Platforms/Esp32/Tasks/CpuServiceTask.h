#pragma once

#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/ESP32Core/CPUUtilization.h"

class CpuServiceTask {
public:
    CpuServiceTask(CPUUtilization& monitor, SnapshotRte<FlightState>& rte,
                   FlightState& state)
        : monitor_(monitor), rte_(rte), state_(state) {}

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(500);
        TickType_t lastWake = xTaskGetTickCount();
        for (;;) {
            monitor_.update();
            const CPUUtilizationData sample = monitor_.get();
            if (rte_.lock(pdMS_TO_TICKS(2))) {
                state_.cpuCore0_pct = sample.core0_pct;
                state_.cpuCore1_pct = sample.core1_pct;
                state_.cpuValid = sample.valid;
                rte_.unlock();
            }
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    CPUUtilization& monitor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
};
