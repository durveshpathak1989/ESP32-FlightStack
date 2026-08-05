#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Application/State/FlightState.h"
#include "../Runtime/SnapshotRte.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

class BarometerServiceTask : public rte::SoftwareComponent {
public:
    BarometerServiceTask(rte::BarometerServicePort& sensor,
                         rte::ClockServicePort& clock,
                         SnapshotRte<FlightState>& rte, FlightState& state)
        : sensor_(sensor), clock_(clock), rte_(rte), state_(state) {}

    void Init() override {
        initialized_ = false;
        previousAltitudeM_ = 0.0f;
        previousMs_ = 0;
        sensor_.InitSensor();
    }

    void Periodic() override {
        rte::BarometerServiceSample sample{};
        const bool readOk = sensor_.ReadSample(sample);
        if (readOk) publish(sample);
        else publishInvalid();
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
    void publish(const rte::BarometerServiceSample& sample) {
        const std::uint32_t nowMs = clock_.milliseconds();
        float verticalSpeed = 0.0f;
        if (sample.valid && initialized_ && nowMs > previousMs_) {
            const float dt = (nowMs - previousMs_) * 0.001f;
            verticalSpeed = (sample.altitudeM - previousAltitudeM_) / dt;
        }
        if (sample.valid) {
            previousAltitudeM_ = sample.altitudeM;
            previousMs_ = nowMs;
            initialized_ = true;
        }
        if (rte_.lock(pdMS_TO_TICKS(2))) {
            state_.bmpTemp_c = sample.temperatureC;
            state_.bmpPressure_hpa = sample.pressureHpa;
            state_.bmpAltitude_m = sample.altitudeM;
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

    rte::BarometerServicePort& sensor_;
    rte::ClockServicePort& clock_;
    SnapshotRte<FlightState>& rte_;
    FlightState& state_;
    bool initialized_ = false;
    float previousAltitudeM_ = 0.0f;
    std::uint32_t previousMs_ = 0;
};
