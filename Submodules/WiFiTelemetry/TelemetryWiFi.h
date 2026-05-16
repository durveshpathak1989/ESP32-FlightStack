/**
 * ============================================================
 *  TelemetryWiFi.h — ESP32 Wi-Fi Hotspot Telemetry Subsystem
 *  Works with DroneGroundStation.html using HTTP JSON polling
 * ============================================================
 *
 *  What this subsystem does:
 *    1. ESP32 creates its own Wi-Fi hotspot/access point.
 *    2. Ground-station laptop/phone connects to that hotspot.
 *    3. Browser opens DroneGroundStation.html from your computer.
 *    4. HTML polls http://192.168.4.1/telemetry and updates the UI.
 *
 *  No external Arduino libraries are required.
 *  Uses only ESP32 core libraries: WiFi.h and WebServer.h
 * ============================================================
 */

#pragma once
#ifndef TELEMETRY_WIFI_H
#define TELEMETRY_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "FlySkyiBUS.h"

struct TelemetryPacket {
    uint32_t tick;
    const char* mode;
    float roll_deg;
    float pitch_deg;
    float yaw_deg;
    float throttle;
    float rc_roll;
    float rc_pitch;
    float rc_yaw;
    float motor_fl;
    float motor_fr;
    float motor_rl;
    float motor_rr;
    float rc_hz;
    bool armed;
    bool rc_valid;
    float bmp_temp_c;
    float bmp_pressure_hpa;
    float bmp_altitude_m;
    bool bmp_valid;
};

class TelemetryWiFi {
public:
    explicit TelemetryWiFi(uint16_t port = 80);

    bool begin(const char* ssid = "ESP32-DRONE",
               const char* password = "12345678",
               IPAddress localIp = IPAddress(192, 168, 4, 1),
               IPAddress gateway = IPAddress(192, 168, 4, 1),
               IPAddress subnet = IPAddress(255, 255, 255, 0));

    void update();
    void setTelemetryProvider(bool (*provider)(TelemetryPacket& out));

    IPAddress ip() const;
    uint32_t requestCount() const;

private:
    WebServer _server;
    bool (*_provider)(TelemetryPacket& out);
    uint32_t _requestCount;

    void _setupRoutes();
    void _sendCorsHeaders();
    void _handleRoot();
    void _handleTelemetry();
    void _handleOptions();
    void _handleNotFound();
    String _jsonFromPacket(const TelemetryPacket& p) const;
};

extern TelemetryWiFi telemetryWiFi;

#endif // TELEMETRY_WIFI_H
