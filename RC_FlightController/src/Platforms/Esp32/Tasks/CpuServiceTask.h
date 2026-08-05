#pragma once

#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

class CpuServiceTask : public rte::SoftwareComponent {
public:
    CpuServiceTask(rte::CpuLoadServicePort& monitor, SnapshotRte<FlightState>& rte,
                   FlightState& state)
        : monitor_(monitor), rte_(rte), state_(state) {}

    void Init() override { monitor_.InitMonitor(1000); }
    void Periodic() override {
        const rte::CpuLoadSample sample = monitor_.ReadLoad();
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.cpuCore0_pct = sample.core0Percent;
            state_.cpuCore1_pct = sample.core1Percent;
            state_.cpuValid = sample.valid;
            rte_.unlock();
        }
    }

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(500);
        TickType_t lastWake = xTaskGetTickCount();
        Init();
        for (;;) {
            Periodic();
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    rte::CpuLoadServicePort& monitor_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
};
