#pragma once

#include "Arduino.h"
#include "../../../Core/ServicePorts.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Esp32SystemServiceAdapter : public rte::ClockServicePort,
                                  public rte::CoreServicePort {
public:
    std::uint64_t microseconds() const override { return micros(); }
    std::uint32_t milliseconds() const override { return millis(); }
    std::uint8_t currentCore() const override {
        return static_cast<std::uint8_t>(xPortGetCoreID());
    }
    void delayMilliseconds(std::uint32_t durationMs) override {
        vTaskDelay(pdMS_TO_TICKS(durationMs));
    }
    void yield() override { taskYIELD(); }
};
