#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Submodules/WiFiTelemetry/TelemetryWiFi.h"

struct WifiServiceBindings {
    bool (*telemetry)(TelemetryPacket&);
    bool (*tune)(const TunePacket&);
    bool (*otaAllowed)();
    String (*timingJson)();
    String (*timingCsv)();
    void (*timingReset)();
    String (*spectrumJson)();
    String (*identityJson)();
    std::uint16_t (*flightLogCount)();
    String (*flightLogHeader)();
    String (*flightLogRow)(std::uint16_t);
    void (*flightLogReset)();
};

// Owns Wi-Fi adapter callback wiring and its event-service loop. HTTP handlers
// remain adapters; application services own validation and safety policy.
class WifiServiceTask {
public:
    WifiServiceTask(TelemetryWiFi& wifi, volatile std::uint32_t& serviceTimeUs)
        : wifi_(wifi), serviceTimeUs_(serviceTimeUs) {}

    [[noreturn]] void run(const WifiServiceBindings& bindings,
                          const char* ssid, const char* password) {
        wifi_.setTelemetryProvider(bindings.telemetry);
        wifi_.setTuneHandler(bindings.tune);
        wifi_.setOtaAllowedProvider(bindings.otaAllowed);
        wifi_.setTimingProvider(bindings.timingJson);
        wifi_.setTimingCsvProvider(bindings.timingCsv);
        wifi_.setTimingResetHandler(bindings.timingReset);
        wifi_.setSpectrumProvider(bindings.spectrumJson);
        wifi_.setIdentityProvider(bindings.identityJson);
        wifi_.setFlightLogCountProvider(bindings.flightLogCount);
        wifi_.setFlightLogHeaderProvider(bindings.flightLogHeader);
        wifi_.setFlightLogRowProvider(bindings.flightLogRow);
        wifi_.setFlightLogResetHandler(bindings.flightLogReset);
        wifi_.begin(ssid, password);
        for (;;) {
            const std::uint32_t startUs = micros();
            wifi_.update();
            serviceTimeUs_ = micros() - startUs;
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

private:
    TelemetryWiFi& wifi_;
    volatile std::uint32_t& serviceTimeUs_;
};
