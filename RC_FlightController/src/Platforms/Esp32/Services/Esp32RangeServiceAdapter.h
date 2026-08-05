#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/ToF/FlightToF_VL53L4CX.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Esp32RangeServiceAdapter : public rte::RangeServicePort {
public:
    Esp32RangeServiceAdapter(
        FlightToF_VL53L4CX& sensor, SemaphoreHandle_t& mutex,
        std::uint8_t address, std::uint32_t timingBudgetUs,
        std::uint32_t interMeasurementMs, std::uint32_t staleMs)
        : sensor_(sensor), mutex_(mutex), address_(address),
          timingBudgetUs_(timingBudgetUs),
          interMeasurementMs_(interMeasurementMs), staleMs_(staleMs) {}

    bool InitSensor() override {
        if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE) return false;
        const bool initialized = sensor_.begin(
            address_, FlightToF_VL53L4CX::DISTANCE_MEDIUM,
            timingBudgetUs_, interMeasurementMs_);
        xSemaphoreGive(mutex_);
        return initialized;
    }
    rte::RangeServiceSample ReadRange() override {
        bool ready = sensor_.isReady();
        if (ready && xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
            sensor_.update();
            ready = sensor_.isReady();
            xSemaphoreGive(mutex_);
        }
        const FlightToFReading value = sensor_.reading();
        const std::uint32_t ageMs = ready ? sensor_.ageMs() : UINT32_MAX;
        return {value.distanceMm, value.rangeStatus, value.objectCount,
                value.streamCount, value.signalMcps, value.ambientMcps,
                ageMs, value.lastUpdateMs, ready,
                ready && value.valid && ageMs <= staleMs_};
    }

private:
    FlightToF_VL53L4CX& sensor_;
    SemaphoreHandle_t& mutex_;
    std::uint8_t address_;
    std::uint32_t timingBudgetUs_;
    std::uint32_t interMeasurementMs_;
    std::uint32_t staleMs_;
};
