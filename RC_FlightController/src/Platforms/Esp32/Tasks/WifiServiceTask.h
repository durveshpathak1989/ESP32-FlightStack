#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

// Owns Wi-Fi adapter callback wiring and its event-service loop. HTTP handlers
// remain adapters; application services own validation and safety policy.
class WifiServiceTask : public rte::SoftwareComponent {
public:
    WifiServiceTask(rte::WifiServicePort& service, rte::ClockServicePort& clock,
                    volatile std::uint32_t& serviceTimeUs)
        : service_(service), clock_(clock), serviceTimeUs_(serviceTimeUs) {}

    void configure(const char* ssid, const char* password) {
        ssid_ = ssid;
        password_ = password;
    }

    void Init() override { service_.InitAccessPoint(ssid_, password_); }

    void Periodic() override {
        const std::uint32_t startUs = clock_.microseconds();
        service_.PeriodicService();
        serviceTimeUs_ = clock_.microseconds() - startUs;
    }

    [[noreturn]] void run() {
        Init();
        for (;;) {
            Periodic();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

private:
    rte::WifiServicePort& service_;
    rte::ClockServicePort& clock_;
    volatile std::uint32_t& serviceTimeUs_;
    const char* ssid_ = "";
    const char* password_ = "";
};
