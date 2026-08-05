#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/BatteryMonitor/BatteryMonitor.h"

class Esp32BatteryServiceAdapter : public rte::BatteryServicePort {
public:
    Esp32BatteryServiceAdapter(
        BatteryMonitor& monitor, std::uint8_t adcPin, std::uint8_t cellCount,
        float topOhms, float bottomOhms, float nominalVoltage, float scale)
        : monitor_(monitor), adcPin_(adcPin), cellCount_(cellCount),
          topOhms_(topOhms), bottomOhms_(bottomOhms),
          nominalVoltage_(nominalVoltage), scale_(scale) {}

    bool InitMonitor() override {
        monitor_.begin(adcPin_, cellCount_, topOhms_, bottomOhms_,
                       nominalVoltage_, scale_);
        return true;
    }
    rte::BatteryServiceSample ReadStatus(std::uint32_t nowMs,
                                         bool force) override {
        const BatteryStatus value = force ? monitor_.forceRead()
                                          : (monitor_.update(nowMs), monitor_.status());
        return {value.batteryVoltage_v, value.adcVoltage_v,
                value.cellVoltage_v, value.percent, value.valid,
                value.low, value.critical};
    }
    float CalibrationScale() const override { return monitor_.scale(); }

private:
    BatteryMonitor& monitor_;
    std::uint8_t adcPin_;
    std::uint8_t cellCount_;
    float topOhms_;
    float bottomOhms_;
    float nominalVoltage_;
    float scale_;
};
