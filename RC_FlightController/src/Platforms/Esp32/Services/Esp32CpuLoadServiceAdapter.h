#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/ESP32Core/CPUUtilization.h"

class Esp32CpuLoadServiceAdapter : public rte::CpuLoadServicePort {
public:
    explicit Esp32CpuLoadServiceAdapter(CPUUtilization& monitor)
        : monitor_(monitor) {}

    bool InitMonitor(std::uint32_t samplePeriodMs) override {
        return monitor_.begin(samplePeriodMs);
    }
    rte::CpuLoadSample ReadLoad() override {
        monitor_.update();
        const CPUUtilizationData value = monitor_.get();
        return {value.core0_pct, value.core1_pct, value.valid};
    }

private:
    CPUUtilization& monitor_;
};
