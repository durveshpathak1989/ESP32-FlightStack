#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

class TelemetryWiFi {
public:
    bool beginAP(const char* ssid,
                 const char* password,
                 uint16_t port,
                 uint8_t channel = 6,
                 uint8_t maxConn = 2);

    // Optional: call periodically to process discovery packets from a client.
    void poll();

    // Sends telemetry to the last discovered client; falls back to broadcast.
    void send(const char* payload);

    bool isReady() const;
    bool hasClient() const;
    IPAddress apIP() const;
    IPAddress clientIP() const;
    uint16_t port() const;

private:
    WiFiUDP _udp;
    IPAddress _broadcastIP{192, 168, 4, 255};
    IPAddress _clientIP{0, 0, 0, 0};
    uint16_t _port = 0;
    bool _ready = false;
};
