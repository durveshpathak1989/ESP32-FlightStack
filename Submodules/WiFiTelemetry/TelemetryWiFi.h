/**
 * ============================================================
 *  TelemetryWiFi.h — ESP32 Wi-Fi Hotspot Telemetry Subsystem
 *  v2.4 — adds /timing, /timing/reset, /timing/csv endpoints
 * ============================================================
 */

#pragma once
#ifndef TELEMETRY_WIFI_H
#define TELEMETRY_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "FlySkyiBUS.h"

#define WIFI_LOG_CAPACITY   60
#define WIFI_LOG_LINE_LEN  140

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

    // GPS block
    double  gps_lat;
    double  gps_lon;
    float   gps_altitude_m;
    float   gps_speed_kmh;
    float   gps_course_deg;
    uint8_t gps_satellites;
    float   gps_hdop;
    uint8_t gps_fix_quality;
    bool    gps_valid;
    uint8_t gps_hour, gps_minute, gps_second;
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

    // Providers / handlers registered by the main sketch
    void setTelemetryProvider(bool (*provider)(TelemetryPacket& out));
    void setTuneHandler(void (*handler)(const TunePacket& in));

    // v2.4 — timing endpoint providers
    void setTimingProvider(String (*provider)());
    void setTimingCsvProvider(String (*provider)());
    void setTimingResetHandler(void (*handler)());

    void pushLog(const char* line);
    void pushLog(const String& line) { pushLog(line.c_str()); }

    IPAddress ip()           const;
    uint32_t  requestCount() const;

private:
    WebServer  _server;
    bool     (*_provider)(TelemetryPacket& out);
    void     (*_tuneHandler)(const TunePacket& in);

    // v2.4 timing callbacks
    String   (*_timingProvider)();
    String   (*_timingCsvProvider)();
    void     (*_timingResetHandler)();

    uint32_t   _requestCount;

    struct LogEntry {
        uint32_t seq;
        char     text[WIFI_LOG_LINE_LEN];
    };
    LogEntry  _logBuf[WIFI_LOG_CAPACITY];
    uint32_t  _logWriteSeq = 0;
    uint8_t   _logHead     = 0;
    uint8_t   _logCount    = 0;
    portMUX_TYPE _logMux   = portMUX_INITIALIZER_UNLOCKED;

    void _setupRoutes();
    void _sendCorsHeaders();
    void _handleRoot();
    void _handleTelemetry();
    void _handleTune();
    void _handleLog();
    void _handleTiming();
    void _handleTimingReset();
    void _handleTimingCsv();
    void _handleOptions();
    void _handleNotFound();

    String _jsonFromPacket(const TelemetryPacket& p) const;
    bool   _jsonGetFloat(const String& body, const char* key, float& out) const;
};

extern TelemetryWiFi telemetryWiFi;

#endif // TELEMETRY_WIFI_H
