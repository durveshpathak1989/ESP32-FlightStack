#pragma once

#include <cstdint>

#include "../../../Core/ServicePorts.h"
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

// Server implementation of the Wi-Fi client/server port.
class TelemetryWifiServiceAdapter : public rte::WifiServicePort {
public:
    explicit TelemetryWifiServiceAdapter(TelemetryWiFi& wifi) : wifi_(wifi) {}

    void bind(const WifiServiceBindings& bindings) {
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
    }

    bool InitAccessPoint(const char* ssid, const char* password) override {
        wifi_.begin(ssid, password);
        return true;
    }
    void PeriodicService() override { wifi_.update(); }

private:
    TelemetryWiFi& wifi_;
};
