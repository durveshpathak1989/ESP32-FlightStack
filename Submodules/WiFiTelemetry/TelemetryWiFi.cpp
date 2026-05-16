/**
 * ============================================================
 *  TelemetryWiFi.cpp — ESP32 Wi-Fi Hotspot Telemetry Subsystem
 * ============================================================
 */

#include "TelemetryWiFi.h"

TelemetryWiFi telemetryWiFi(80);

TelemetryWiFi::TelemetryWiFi(uint16_t port)
    : _server(port), _provider(nullptr), _requestCount(0)
{
}

bool TelemetryWiFi::begin(const char* ssid,
                          const char* password,
                          IPAddress localIp,
                          IPAddress gateway,
                          IPAddress subnet)
{
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);  // lower latency for telemetry polling

    if (!WiFi.softAPConfig(localIp, gateway, subnet)) {
        Serial.println(F("[WiFi] softAPConfig failed"));
        return false;
    }

    bool ok = WiFi.softAP(ssid, password);
    if (!ok) {
        Serial.println(F("[WiFi] softAP start failed"));
        return false;
    }

    _setupRoutes();
    _server.begin();

    Serial.println(F("[WiFi] Telemetry hotspot started"));
    Serial.printf("[WiFi] SSID: %s\n", ssid);
    Serial.printf("[WiFi] PASS: %s\n", password);
    Serial.printf("[WiFi] URL : http://%s/telemetry\n", WiFi.softAPIP().toString().c_str());
    return true;
}

void TelemetryWiFi::setTelemetryProvider(bool (*provider)(TelemetryPacket& out))
{
    _provider = provider;
}

void TelemetryWiFi::update()
{
    _server.handleClient();
}

IPAddress TelemetryWiFi::ip() const
{
    return WiFi.softAPIP();
}

uint32_t TelemetryWiFi::requestCount() const
{
    return _requestCount;
}

void TelemetryWiFi::_setupRoutes()
{
    _server.on("/", HTTP_GET, [this]() { _handleRoot(); });
    _server.on("/telemetry", HTTP_GET, [this]() { _handleTelemetry(); });
    _server.on("/telemetry", HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.onNotFound([this]() { _handleNotFound(); });
}

void TelemetryWiFi::_sendCorsHeaders()
{
    // Required because DroneGroundStation.html is usually opened as a local file:// page.
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    _server.sendHeader("Cache-Control", "no-store");
}

void TelemetryWiFi::_handleRoot()
{
    _sendCorsHeaders();
    _server.send(200, "text/plain",
                 "ESP32 Drone Telemetry Hotspot\n"
                 "Endpoint: /telemetry\n"
                 "Open DroneGroundStation.html and click CONNECT WIFI.\n");
}

void TelemetryWiFi::_handleTelemetry()
{
    _requestCount++;
    _sendCorsHeaders();

    if (_provider == nullptr) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"no telemetry provider\"}");
        return;
    }

    TelemetryPacket p;
    memset(&p, 0, sizeof(p));
    if (!_provider(p)) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"telemetry unavailable\"}");
        return;
    }

    _server.send(200, "application/json", _jsonFromPacket(p));
}

void TelemetryWiFi::_handleOptions()
{
    _sendCorsHeaders();
    _server.send(204);
}

void TelemetryWiFi::_handleNotFound()
{
    _sendCorsHeaders();
    _server.send(404, "application/json", "{\"ok\":false,\"error\":\"not found\"}");
}

String TelemetryWiFi::_jsonFromPacket(const TelemetryPacket& p) const
{
    String json;
    json.reserve(560);
    json += "{\"ok\":true";
    json += ",\"tick\":" + String(p.tick);
    json += ",\"mode\":\"" + String(p.mode ? p.mode : "UNKNOWN") + "\"";
    json += ",\"roll\":" + String(p.roll_deg, 2);
    json += ",\"pitch\":" + String(p.pitch_deg, 2);
    json += ",\"yaw\":" + String(p.yaw_deg, 2);
    json += ",\"thr\":" + String(p.throttle, 3);
    json += ",\"rcRoll\":" + String(p.rc_roll, 3);
    json += ",\"rcPitch\":" + String(p.rc_pitch, 3);
    json += ",\"rcYaw\":" + String(p.rc_yaw, 3);
    json += ",\"motFL\":" + String(p.motor_fl, 3);
    json += ",\"motFR\":" + String(p.motor_fr, 3);
    json += ",\"motRL\":" + String(p.motor_rl, 3);
    json += ",\"motRR\":" + String(p.motor_rr, 3);
    json += ",\"rcHz\":" + String(p.rc_hz, 1);
    json += ",\"armed\":" + String(p.armed ? "true" : "false");
    json += ",\"rcValid\":" + String(p.rc_valid ? "true" : "false");
    json += ",\"bmpTempC\":" + String(p.bmp_temp_c, 2);
    json += ",\"bmpPressureHpa\":" + String(p.bmp_pressure_hpa, 2);
    json += ",\"bmpAltitudeM\":" + String(p.bmp_altitude_m, 2);
    json += ",\"bmpValid\":" + String(p.bmp_valid ? "true" : "false");
    json += ",\"clients\":" + String(WiFi.softAPgetStationNum());
    json += ",\"requests\":" + String(_requestCount);
    json += "}";
    return json;
}
