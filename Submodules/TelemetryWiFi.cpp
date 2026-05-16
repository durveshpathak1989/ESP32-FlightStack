#include "TelemetryWiFi.h"

bool TelemetryWiFi::beginAP(const char* ssid,
                            const char* password,
                            uint16_t port,
                            uint8_t channel,
                            uint8_t maxConn) {
    _ready = false;
    _port = port;
    _clientIP = IPAddress(0, 0, 0, 0);

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ssid, password, channel, 0, maxConn)) {
        return false;
    }

    _broadcastIP = IPAddress(192, 168, 4, 255);
    if (!_udp.begin(_port)) {
        return false;
    }

    _ready = true;
    return true;
}

void TelemetryWiFi::poll() {
    if (!_ready) return;

    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) return;

    // Any packet from a ground station is treated as discovery/heartbeat.
    _clientIP = _udp.remoteIP();

    // Drain packet data (content not used right now).
    while (_udp.available()) {
        (void)_udp.read();
    }
}

void TelemetryWiFi::send(const char* payload) {
    if (!_ready || payload == nullptr) return;

    const IPAddress target = hasClient() ? _clientIP : _broadcastIP;
    _udp.beginPacket(target, _port);
    _udp.print(payload);
    _udp.endPacket();
}

bool TelemetryWiFi::isReady() const {
    return _ready;
}

bool TelemetryWiFi::hasClient() const {
    return _clientIP != IPAddress(0, 0, 0, 0);
}

IPAddress TelemetryWiFi::apIP() const {
    return WiFi.softAPIP();
}

IPAddress TelemetryWiFi::clientIP() const {
    return _clientIP;
}

uint16_t TelemetryWiFi::port() const {
    return _port;
}
