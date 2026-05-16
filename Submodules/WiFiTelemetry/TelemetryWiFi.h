/**
 * ============================================================
 *  TelemetryWiFi.h — ESP32 Wi-Fi Hotspot Telemetry Subsystem
 *  v2.2 — /telemetry (GET) + /tune (POST) + /log (GET)
 * ============================================================
 *
 *  New in v2.2:
 *    • pushLog(line) — call this wherever you Serial.print calibration
 *      messages. The text is stored in a 60-line circular buffer.
 *    • GET /log?since=N — returns JSON array of log lines newer than
 *      sequence number N. HTML polls this at 500 ms when Wi-Fi connected
 *      so calibration messages appear in the ground station log panel
 *      regardless of whether serial is connected.
 * ============================================================
 */

#pragma once
#ifndef TELEMETRY_WIFI_H
#define TELEMETRY_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "FlySkyiBUS.h"

// ─────────────────────────────────────────────────────────────
//  Log buffer config
// ─────────────────────────────────────────────────────────────
#define WIFI_LOG_CAPACITY   60      // number of lines kept in circular buffer
#define WIFI_LOG_LINE_LEN  140      // max characters per line (truncated if longer)

// ─────────────────────────────────────────────────────────────
//  Telemetry packet
// ─────────────────────────────────────────────────────────────
struct TelemetryPacket {
    uint32_t tick;
    const char* mode;
    bool armed;
    bool rc_valid;

    float roll_deg, pitch_deg, yaw_deg;
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float mx_uT, my_uT, mz_uT;

    float throttle, rc_roll, rc_pitch, rc_yaw, rc_hz;

    float motor_fl, motor_fr, motor_rl, motor_rr;
    float rpm_fl,   rpm_fr,   rpm_rl,   rpm_rr;

    float bmp_temp_c, bmp_pressure_hpa, bmp_altitude_m;
    bool  bmp_valid;

    float cpu_core0_pct, cpu_core1_pct;
    bool  cpu_valid;

    float pid_roll_kp,  pid_roll_ki,  pid_roll_kd;
    float pid_pitch_kp, pid_pitch_ki, pid_pitch_kd;
    float pid_yaw_kp,   pid_yaw_ki,   pid_yaw_kd;
    float pid_angle_roll_kp, pid_angle_pitch_kp;
    float mahony_kp, mahony_ki;
};

// ─────────────────────────────────────────────────────────────
//  Tune packet
// ─────────────────────────────────────────────────────────────
struct TunePacket {
    bool has_pid_roll_kp,  has_pid_roll_ki,  has_pid_roll_kd;
    bool has_pid_pitch_kp, has_pid_pitch_ki, has_pid_pitch_kd;
    bool has_pid_yaw_kp,   has_pid_yaw_ki,   has_pid_yaw_kd;
    bool has_pid_angle_roll_kp, has_pid_angle_pitch_kp;
    bool has_mahony_kp, has_mahony_ki;

    float pid_roll_kp,  pid_roll_ki,  pid_roll_kd;
    float pid_pitch_kp, pid_pitch_ki, pid_pitch_kd;
    float pid_yaw_kp,   pid_yaw_ki,   pid_yaw_kd;
    float pid_angle_roll_kp, pid_angle_pitch_kp;
    float mahony_kp, mahony_ki;
};

// ─────────────────────────────────────────────────────────────
//  TelemetryWiFi class
// ─────────────────────────────────────────────────────────────
class TelemetryWiFi {
public:
    explicit TelemetryWiFi(uint16_t port = 80);

    bool begin(const char* ssid     = "ESP32-DRONE",
               const char* password = "12345678",
               IPAddress   localIp  = IPAddress(192, 168, 4, 1),
               IPAddress   gateway  = IPAddress(192, 168, 4, 1),
               IPAddress   subnet   = IPAddress(255, 255, 255, 0));

    void update();

    void setTelemetryProvider(bool (*provider)(TelemetryPacket& out));
    void setTuneHandler(void (*handler)(const TunePacket& in));

    /**
     * pushLog() — store one log line in the Wi-Fi log buffer.
     *
     * Call this everywhere you currently call Serial.println/printf for
     * calibration messages. The line is also printed to Serial automatically
     * so you don't need separate Serial.print calls.
     *
     * Thread-safe: uses a spinlock so it can be called from any FreeRTOS task.
     *
     * Example:
     *   telemetryWiFi.pushLog("[CAL] ✓ GYRO complete.");
     *   // instead of: Serial.println("[CAL] ✓ GYRO complete.");
     */
    void pushLog(const char* line);
    void pushLog(const String& line) { pushLog(line.c_str()); }

    IPAddress ip()           const;
    uint32_t  requestCount() const;

private:
    WebServer  _server;
    bool     (*_provider)(TelemetryPacket& out);
    void     (*_tuneHandler)(const TunePacket& in);
    uint32_t   _requestCount;

    // ── Circular log buffer ───────────────────────────────────
    // Each entry has a monotonically increasing sequence number.
    // The HTML sends its last-seen seq number so we only return new lines.
    struct LogEntry {
        uint32_t seq;                    // sequence number, starts at 1
        char     text[WIFI_LOG_LINE_LEN];
    };
    LogEntry  _logBuf[WIFI_LOG_CAPACITY];
    uint32_t  _logWriteSeq = 0;   // next seq to assign
    uint8_t   _logHead     = 0;   // index of oldest entry (wrap-around)
    uint8_t   _logCount    = 0;   // how many entries are stored (0..CAPACITY)
    portMUX_TYPE _logMux   = portMUX_INITIALIZER_UNLOCKED;

    // Route handlers
    void _setupRoutes();
    void _sendCorsHeaders();
    void _handleRoot();
    void _handleTelemetry();
    void _handleTune();
    void _handleLog();
    void _handleOptions();
    void _handleNotFound();

    String _jsonFromPacket(const TelemetryPacket& p) const;
    bool   _jsonGetFloat(const String& body, const char* key, float& out) const;
};

extern TelemetryWiFi telemetryWiFi;

#endif // TELEMETRY_WIFI_H
