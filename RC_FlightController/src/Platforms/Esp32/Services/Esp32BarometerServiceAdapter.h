#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/BMP280/BMP280Sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Esp32BarometerServiceAdapter : public rte::BarometerServicePort {
public:
    Esp32BarometerServiceAdapter(BMP280Sensor& sensor, SemaphoreHandle_t& mutex,
                                 std::uint8_t sdaPin, std::uint8_t sclPin,
                                 std::uint32_t busHz)
        : sensor_(sensor), mutex_(mutex), sdaPin_(sdaPin), sclPin_(sclPin),
          busHz_(busHz) {}

    bool InitSensor() override {
        if (xSemaphoreTake(mutex_, portMAX_DELAY) != pdTRUE) return false;
        sensor_.scanI2C(sdaPin_, sclPin_, busHz_);
        const bool initialized = sensor_.beginAuto(sdaPin_, sclPin_, busHz_);
        xSemaphoreGive(mutex_);
        return initialized;
    }
    bool ReadSample(rte::BarometerServiceSample& sample) override {
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) != pdTRUE) return false;
        BMP280Data value{};
        const bool readOk = sensor_.read(value);
        xSemaphoreGive(mutex_);
        sample = {value.temperature_c, value.pressure_hpa,
                  value.altitude_m, value.valid};
        return readOk;
    }

private:
    BMP280Sensor& sensor_;
    SemaphoreHandle_t& mutex_;
    std::uint8_t sdaPin_;
    std::uint8_t sclPin_;
    std::uint32_t busHz_;
};
