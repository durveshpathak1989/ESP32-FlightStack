#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct Esp32TaskDefinition {
    TaskFunction_t entry;
    const char* name;
    std::uint32_t stackWords;
    UBaseType_t priority;
    BaseType_t core;
};

// ESP32/FreeRTOS platform composition. Application SWCs know only their
// periodic/event activation contract; task handles and esp_timer live here.
class Esp32FlightScheduler {
public:
    bool start(const Esp32TaskDefinition* serviceTasks,
               std::size_t serviceTaskCount,
               const Esp32TaskDefinition& controlTask,
               std::uint64_t controlPeriodUs) {
        if (started_) return false;
        for (std::size_t i = 0; i < serviceTaskCount; ++i) {
            TaskHandle_t ignored = nullptr;
            if (xTaskCreatePinnedToCore(
                    serviceTasks[i].entry, serviceTasks[i].name,
                    serviceTasks[i].stackWords, nullptr,
                    serviceTasks[i].priority, &ignored,
                    serviceTasks[i].core) != pdPASS) {
                return false;
            }
        }
        if (xTaskCreatePinnedToCore(
                controlTask.entry, controlTask.name, controlTask.stackWords,
                nullptr, controlTask.priority, &controlTaskHandle_,
                controlTask.core) != pdPASS) {
            return false;
        }

        esp_timer_create_args_t timerArgs;
        std::memset(&timerArgs, 0, sizeof(timerArgs));
        timerArgs.callback = &Esp32FlightScheduler::releaseControl;
        timerArgs.arg = this;
        timerArgs.dispatch_method = ESP_TIMER_TASK;
        timerArgs.name = "ctrl400";
        if (esp_timer_create(&timerArgs, &controlTimer_) != ESP_OK) return false;
        if (esp_timer_start_periodic(controlTimer_, controlPeriodUs) != ESP_OK)
            return false;
        started_ = true;
        return true;
    }

private:
    static void releaseControl(void* context) {
        auto* scheduler = static_cast<Esp32FlightScheduler*>(context);
        if (scheduler != nullptr && scheduler->controlTaskHandle_ != nullptr)
            xTaskNotifyGive(scheduler->controlTaskHandle_);
    }

    TaskHandle_t controlTaskHandle_ = nullptr;
    esp_timer_handle_t controlTimer_ = nullptr;
    bool started_ = false;
};
