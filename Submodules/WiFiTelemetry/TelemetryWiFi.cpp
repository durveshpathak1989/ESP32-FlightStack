/**
 * ============================================================
 *  TelemetryWiFi.cpp — v2.4  (timing endpoints added)
 * ============================================================
 */

#include "TelemetryWiFi.h"

TelemetryWiFi telemetryWiFi(80);

TelemetryWiFi::TelemetryWiFi(uint16_t port)
    : _server(port),
      _provider(nullptr),
      _tuneHandler(nullptr),
      _timingProvider(nullptr),
      _timingCsvProvider(nullptr),
      _timingResetHandler(nullptr),
      _requestCount(0)
{
    memset(_logBuf, 0, sizeof(_logBuf));
}

bool TelemetryWiFi::begin(const char* ssid, const char* password,
                          IPAddress localIp, IPAddress gateway, IPAddress subnet)
{
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    if (!WiFi.softAPConfig(localIp, gateway, subnet)) {
        Serial.println(F("[WiFi] softAPConfig failed")); return false;
    }
    if (!WiFi.softAP(ssid, password)) {
        Serial.println(F("[WiFi] softAP failed")); return false;
    }
    _setupRoutes();
    _server.begin();
    Serial.printf("[WiFi] SSID=%s  PASS=%s\n", ssid, password);
    Serial.printf("[WiFi] http://%s/\n", WiFi.softAPIP().toString().c_str());
    return true;
}

void TelemetryWiFi::setTelemetryProvider(bool (*p)(TelemetryPacket& out)) { _provider             = p; }
void TelemetryWiFi::setTuneHandler(void (*h)(const TunePacket& in))       { _tuneHandler          = h; }
void TelemetryWiFi::setTimingProvider(String (*p)())                       { _timingProvider       = p; }
void TelemetryWiFi::setTimingCsvProvider(String (*p)())                    { _timingCsvProvider    = p; }
void TelemetryWiFi::setTimingResetHandler(void (*h)())                     { _timingResetHandler   = h; }
void TelemetryWiFi::update()                                               { _server.handleClient(); }
IPAddress TelemetryWiFi::ip()           const { return WiFi.softAPIP(); }
uint32_t  TelemetryWiFi::requestCount() const { return _requestCount; }

void TelemetryWiFi::pushLog(const char* line)
{
    Serial.println(line);
    portENTER_CRITICAL(&_logMux);
    uint8_t slot;
    if (_logCount < WIFI_LOG_CAPACITY) {
        slot = (_logHead + _logCount) % WIFI_LOG_CAPACITY;
        _logCount++;
    } else {
        slot = _logHead;
        _logHead = (_logHead + 1) % WIFI_LOG_CAPACITY;
    }
    _logBuf[slot].seq = ++_logWriteSeq;
    strncpy(_logBuf[slot].text, line, WIFI_LOG_LINE_LEN - 1);
    _logBuf[slot].text[WIFI_LOG_LINE_LEN - 1] = '\0';
    portEXIT_CRITICAL(&_logMux);
}

void TelemetryWiFi::_setupRoutes()
{
    _server.on("/",               HTTP_GET,     [this]() { _handleRoot(); });
    _server.on("/telemetry",      HTTP_GET,     [this]() { _handleTelemetry(); });
    _server.on("/telemetry",      HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.on("/tune",           HTTP_POST,    [this]() { _handleTune(); });
    _server.on("/tune",           HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.on("/log",            HTTP_GET,     [this]() { _handleLog(); });
    _server.on("/log",            HTTP_OPTIONS, [this]() { _handleOptions(); });
    // v2.4 timing endpoints
    _server.on("/timing",         HTTP_GET,     [this]() { _handleTiming(); });
    _server.on("/timing",         HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.on("/timing/reset",   HTTP_POST,    [this]() { _handleTimingReset(); });
    _server.on("/timing/reset",   HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.on("/timing/csv",     HTTP_GET,     [this]() { _handleTimingCsv(); });
    _server.on("/timing/csv",     HTTP_OPTIONS, [this]() { _handleOptions(); });
    _server.onNotFound(                         [this]() { _handleNotFound(); });
}

void TelemetryWiFi::_sendCorsHeaders()
{
    _server.sendHeader("Access-Control-Allow-Origin",  "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    _server.sendHeader("Cache-Control", "no-store");
}

void TelemetryWiFi::_handleRoot()
{
    _sendCorsHeaders();
    _server.send(200, "text/plain",
        "ESP32 Drone Telemetry v2.4\n"
        "GET  /telemetry        — full state JSON\n"
        "POST /tune             — apply PID/Mahony gains (disarmed only)\n"
        "GET  /log?since=N      — calibration log lines\n"
        "GET  /timing           — IMU jitter stats (Welford)\n"
        "POST /timing/reset     — reset jitter stats\n"
        "GET  /timing/csv       — download raw period_us ring buffer\n");
}

void TelemetryWiFi::_handleTelemetry()
{
    _requestCount++;
    _sendCorsHeaders();
    if (!_provider) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"no provider\"}");
        return;
    }
    TelemetryPacket p; memset(&p, 0, sizeof(p));
    if (!_provider(p)) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"unavailable\"}");
        return;
    }
    _server.send(200, "application/json", _jsonFromPacket(p));
}

void TelemetryWiFi::_handleTiming()
{
    _sendCorsHeaders();
    if (!_timingProvider) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"no timing provider\"}");
        return;
    }
    _server.send(200, "application/json", _timingProvider());
}

void TelemetryWiFi::_handleTimingReset()
{
    _sendCorsHeaders();
    if (_timingResetHandler) _timingResetHandler();
    _server.send(200, "application/json", "{\"ok\":true,\"msg\":\"timing stats reset\"}");
}

void TelemetryWiFi::_handleTimingCsv()
{
    _sendCorsHeaders();
    if (!_timingCsvProvider) {
        _server.send(503, "text/plain", "no csv provider");
        return;
    }
    _server.sendHeader("Content-Disposition", "attachment; filename=\"timing.csv\"");
    _server.send(200, "text/csv", _timingCsvProvider());
}

void TelemetryWiFi::_handleLog()
{
    _sendCorsHeaders();
    uint32_t since = 0;
    if (_server.hasArg("since")) since = (uint32_t)_server.arg("since").toInt();

    portENTER_CRITICAL(&_logMux);
    const uint32_t writeSeq = _logWriteSeq;
    const uint8_t  count    = _logCount;
    const uint8_t  head     = _logHead;
    struct Snap { uint32_t seq; char text[WIFI_LOG_LINE_LEN]; };
    Snap snap[WIFI_LOG_CAPACITY];
    uint8_t snapCount = 0;
    for (uint8_t i = 0; i < count; i++) {
        uint8_t idx = (head + i) % WIFI_LOG_CAPACITY;
        if (_logBuf[idx].seq > since) {
            snap[snapCount].seq = _logBuf[idx].seq;
            strncpy(snap[snapCount].text, _logBuf[idx].text, WIFI_LOG_LINE_LEN - 1);
            snap[snapCount].text[WIFI_LOG_LINE_LEN - 1] = '\0';
            snapCount++;
        }
    }
    portEXIT_CRITICAL(&_logMux);

    String json;
    json.reserve(snapCount * (WIFI_LOG_LINE_LEN + 20) + 60);
    json += "{\"ok\":true,\"nextSeq\":"; json += String(writeSeq); json += ",\"lines\":[";
    for (uint8_t i = 0; i < snapCount; i++) {
        if (i > 0) json += ',';
        json += "{\"seq\":"; json += String(snap[i].seq); json += ",\"text\":\"";
        for (const char* p = snap[i].text; *p; p++) {
            if      (*p == '"')  json += "\\\"";
            else if (*p == '\\') json += "\\\\";
            else if (*p == '\n') json += "\\n";
            else if (*p == '\r') {}
            else                 json += *p;
        }
        json += "\"}";
    }
    json += "]}";
    _server.send(200, "application/json", json);
}

void TelemetryWiFi::_handleTune()
{
    _sendCorsHeaders();
    if (!_tuneHandler) {
        _server.send(503, "application/json", "{\"ok\":false,\"error\":\"no handler\"}");
        return;
    }
    String body = _server.arg("plain");
    if (body.length() == 0) {
        _server.send(400, "application/json", "{\"ok\":false,\"error\":\"empty body\"}");
        return;
    }
    TunePacket t; memset(&t, 0, sizeof(t));
    auto tryF = [&](const char* key, bool& has, float& val) {
        float tmp; if (_jsonGetFloat(body, key, tmp)) { has = true; val = tmp; }
    };
    tryF("pid_roll_kp",        t.has_pid_roll_kp,        t.pid_roll_kp);
    tryF("pid_roll_ki",        t.has_pid_roll_ki,        t.pid_roll_ki);
    tryF("pid_roll_kd",        t.has_pid_roll_kd,        t.pid_roll_kd);
    tryF("pid_pitch_kp",       t.has_pid_pitch_kp,       t.pid_pitch_kp);
    tryF("pid_pitch_ki",       t.has_pid_pitch_ki,       t.pid_pitch_ki);
    tryF("pid_pitch_kd",       t.has_pid_pitch_kd,       t.pid_pitch_kd);
    tryF("pid_yaw_kp",         t.has_pid_yaw_kp,         t.pid_yaw_kp);
    tryF("pid_yaw_ki",         t.has_pid_yaw_ki,         t.pid_yaw_ki);
    tryF("pid_yaw_kd",         t.has_pid_yaw_kd,         t.pid_yaw_kd);
    tryF("pid_angle_roll_kp",  t.has_pid_angle_roll_kp,  t.pid_angle_roll_kp);
    tryF("pid_angle_pitch_kp", t.has_pid_angle_pitch_kp, t.pid_angle_pitch_kp);
    tryF("mahony_kp",          t.has_mahony_kp,          t.mahony_kp);
    tryF("mahony_ki",          t.has_mahony_ki,          t.mahony_ki);
    _tuneHandler(t);
    _server.send(200, "application/json", "{\"ok\":true}");
}

void TelemetryWiFi::_handleOptions() { _sendCorsHeaders(); _server.send(204); }
void TelemetryWiFi::_handleNotFound() {
    _sendCorsHeaders();
    _server.send(404, "application/json", "{\"ok\":false,\"error\":\"not found\"}");
}

// ─────────────────────────────────────────────────────────────
//  JSON serialiser
// ─────────────────────────────────────────────────────────────
String TelemetryWiFi::_jsonFromPacket(const TelemetryPacket& p) const
{
    String j;
    j.reserve(1100);
    j += "{\"ok\":true";
    j += ",\"tick\":"    + String(p.tick);
    j += ",\"mode\":\""  + String(p.mode ? p.mode : "UNKNOWN") + "\"";
    j += ",\"armed\":"   + String(p.armed    ? "true" : "false");
    j += ",\"rcValid\":" + String(p.rc_valid ? "true" : "false");
    j += ",\"roll\":"    + String(p.roll_deg,  2);
    j += ",\"pitch\":"   + String(p.pitch_deg, 2);
    j += ",\"yaw\":"     + String(p.yaw_deg,   2);
    j += ",\"ax\":"      + String(p.ax_g,   4);
    j += ",\"ay\":"      + String(p.ay_g,   4);
    j += ",\"az\":"      + String(p.az_g,   4);
    j += ",\"gx\":"      + String(p.gx_dps, 3);
    j += ",\"gy\":"      + String(p.gy_dps, 3);
    j += ",\"gz\":"      + String(p.gz_dps, 3);
    j += ",\"thr\":"     + String(p.throttle, 3);
    j += ",\"rcRoll\":"  + String(p.rc_roll,  3);
    j += ",\"rcPitch\":" + String(p.rc_pitch, 3);
    j += ",\"rcYaw\":"   + String(p.rc_yaw,   3);
    j += ",\"rcHz\":"    + String(p.rc_hz,    1);
    j += ",\"motFL\":"   + String(p.motor_fl, 3);
    j += ",\"motFR\":"   + String(p.motor_fr, 3);
    j += ",\"motRL\":"   + String(p.motor_rl, 3);
    j += ",\"motRR\":"   + String(p.motor_rr, 3);
    j += ",\"rpmFL\":"   + String(p.rpm_fl,   0);
    j += ",\"rpmFR\":"   + String(p.rpm_fr,   0);
    j += ",\"rpmRL\":"   + String(p.rpm_rl,   0);
    j += ",\"rpmRR\":"   + String(p.rpm_rr,   0);
    j += ",\"bmpTempC\":"       + String(p.bmp_temp_c,       2);
    j += ",\"bmpPressureHpa\":" + String(p.bmp_pressure_hpa, 2);
    j += ",\"bmpAltitudeM\":"   + String(p.bmp_altitude_m,   2);
    j += ",\"bmpValid\":"       + String(p.bmp_valid ? "true" : "false");
    j += ",\"cpuCore0\":"       + String(p.cpu_core0_pct, 1);
    j += ",\"cpuCore1\":"       + String(p.cpu_core1_pct, 1);
    j += ",\"cpuValid\":"       + String(p.cpu_valid ? "true" : "false");
    j += ",\"pidRollKp\":"       + String(p.pid_roll_kp,        4);
    j += ",\"pidRollKi\":"       + String(p.pid_roll_ki,        5);
    j += ",\"pidRollKd\":"       + String(p.pid_roll_kd,        4);
    j += ",\"pidPitchKp\":"      + String(p.pid_pitch_kp,       4);
    j += ",\"pidPitchKi\":"      + String(p.pid_pitch_ki,       5);
    j += ",\"pidPitchKd\":"      + String(p.pid_pitch_kd,       4);
    j += ",\"pidYawKp\":"        + String(p.pid_yaw_kp,         4);
    j += ",\"pidYawKi\":"        + String(p.pid_yaw_ki,         5);
    j += ",\"pidYawKd\":"        + String(p.pid_yaw_kd,         4);
    j += ",\"pidAngleRollKp\":"  + String(p.pid_angle_roll_kp,  3);
    j += ",\"pidAnglePitchKp\":" + String(p.pid_angle_pitch_kp, 3);
    j += ",\"mahonyKp\":"        + String(p.mahony_kp,          4);
    j += ",\"mahonyKi\":"        + String(p.mahony_ki,          5);
    j += ",\"clients\":"         + String(WiFi.softAPgetStationNum());
    j += ",\"requests\":"        + String(_requestCount);
    j += ",\"gpsValid\":"    + String(p.gps_valid ? "true" : "false");
    j += ",\"gpsLat\":"      + String(p.gps_lat,        6);
    j += ",\"gpsLon\":"      + String(p.gps_lon,        6);
    j += ",\"gpsAltM\":"     + String(p.gps_altitude_m, 2);
    j += ",\"gpsSpeedKmh\":" + String(p.gps_speed_kmh,  2);
    j += ",\"gpsCourse\":"   + String(p.gps_course_deg, 1);
    j += ",\"gpsSats\":"     + String(p.gps_satellites);
    j += ",\"gpsHdop\":"     + String(p.gps_hdop,       2);
    j += ",\"gpsQuality\":"  + String(p.gps_fix_quality);
    j += ",\"gpsHour\":"     + String(p.gps_hour);
    j += ",\"gpsMin\":"      + String(p.gps_minute);
    j += ",\"gpsSec\":"      + String(p.gps_second);
    j += "}";
    return j;
}

bool TelemetryWiFi::_jsonGetFloat(const String& body, const char* key, float& out) const
{
    String pat = "\""; pat += key; pat += "\"";
    int idx = body.indexOf(pat);
    if (idx < 0) return false;
    int pos = idx + pat.length();
    while (pos < (int)body.length() && (body[pos]==' '||body[pos]==':')) pos++;
    String num = "";
    while (pos < (int)body.length()) {
        char c = body[pos];
        if (c=='-'||c=='+'||c=='.'||c=='e'||c=='E'||(c>='0'&&c<='9')) { num+=c; pos++; }
        else break;
    }
    if (!num.length()) return false;
    out = num.toFloat();
    return true;
}
