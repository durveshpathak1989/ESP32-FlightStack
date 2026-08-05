#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Submodules/BMP280/BMP280Sensor.h"
#include "../../../Core/RtePorts.h"

class BarometerServiceTask : public rte::SoftwareComponent {
public:
    BarometerServiceTask(BMP280Sensor& sensor, SemaphoreHandle_t& i2cMutex,
                         SnapshotRte<FlightState>& rte, FlightState& state)
        : sensor_(sensor), i2cMutex_(i2cMutex), rte_(rte), state_(state) {}

    void Init() override {
        initialized_ = false;
        previousAltitudeM_ = 0.0f;
        previousMs_ = 0;
    }

    void Periodic() override {
        BMP280Data sample{};
        bool sampled = false;
        bool readOk = false;
        if (xSemaphoreTake(i2cMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
            readOk = sensor_.read(sample);
            sampled = true;
            xSemaphoreGive(i2cMutex_);
        }
        if (readOk) publish(sample);
        else if (sampled) publishInvalid();
    }

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(50);
        TickType_t lastWake = xTaskGetTickCount();
        Init();
        for (;;) {
            Periodic();
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    void publish(const BMP280Data& sample) {
        const std::uint32_t nowMs = millis();
        float verticalSpeed = 0.0f;
        if (sample.valid && initialized_ && nowMs > previousMs_) {
            const float dt = (nowMs - previousMs_) * 0.001f;
            verticalSpeed = (sample.altitude_m - previousAltitudeM_) / dt;
        }
        if (sample.valid) {
            previousAltitudeM_ = sample.altitude_m;
            previousMs_ = nowMs;
            initialized_ = true;
        }
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.bmpTemp_c = sample.temperature_c;
            state_.bmpPressure_hpa = sample.pressure_hpa;
            state_.bmpAltitude_m = sample.altitude_m;
            state_.bmpVerticalSpeed_mps = verticalSpeed;
            state_.bmpValid = sample.valid;
            rte_.unlock();
        }
    }

    void publishInvalid() {
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.bmpValid = false;
            state_.bmpVerticalSpeed_mps = 0.0f;
            rte_.unlock();
        }
    }

    BMP280Sensor& sensor_;
    SemaphoreHandle_t& i2cMutex_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    bool initialized_ = false;
    float previousAltitudeM_ = 0.0f;
    std::uint32_t previousMs_ = 0;
};
