/*
 * Name: Logger.cpp
 * Use: ESP32-safe high-speed diagnostic flight logger implementation.
 * Version: 1.1.0
 */

#include "Logger.h"

static LoggerEkfRateSnapshot g_loggerEkfRateSnapshot = {0.0f, 0.0f, 0.0f, 0U, false};
static portMUX_TYPE g_loggerEkfRateMux = portMUX_INITIALIZER_UNLOCKED;

void loggerPublishEkfRateSnapshot(float rollRateDps,
                                  float pitchRateDps,
                                  float yawRateDps,
                                  bool valid) {
    portENTER_CRITICAL(&g_loggerEkfRateMux);
    g_loggerEkfRateSnapshot.roll_rate_dps = rollRateDps;
    g_loggerEkfRateSnapshot.pitch_rate_dps = pitchRateDps;
    g_loggerEkfRateSnapshot.yaw_rate_dps = yawRateDps;
    g_loggerEkfRateSnapshot.valid = valid;
    g_loggerEkfRateSnapshot.sequence++;
    portEXIT_CRITICAL(&g_loggerEkfRateMux);
}

LoggerEkfRateSnapshot loggerReadEkfRateSnapshot() {
    LoggerEkfRateSnapshot snapshot;
    portENTER_CRITICAL(&g_loggerEkfRateMux);
    snapshot = g_loggerEkfRateSnapshot;
    portEXIT_CRITICAL(&g_loggerEkfRateMux);
    return snapshot;
}

Logger::Logger(uint16_t capacity)
    : _rows(nullptr), _capacity(capacity), _head(0), _full(false),
      _active(true), _dropped(0), _mux(portMUX_INITIALIZER_UNLOCKED) {}

Logger::~Logger() {
    if (_rows) { delete[] _rows; _rows = nullptr; }
}

bool Logger::begin() {
    if (_rows) return true;
    if (_capacity == 0) return false;
    _rows = new LoggerRow[_capacity];
    if (!_rows) return false;
    memset(_rows, 0, sizeof(LoggerRow) * _capacity);
    reset();
    return true;
}

void Logger::reset() {
    portENTER_CRITICAL(&_mux);
    _head = 0; _full = false; _active = true; _dropped = 0;
    if (_rows) memset(_rows, 0, sizeof(LoggerRow) * _capacity);
    portEXIT_CRITICAL(&_mux);
}

void Logger::freeze() {
    portENTER_CRITICAL(&_mux);
    _active = false;
    portEXIT_CRITICAL(&_mux);
}

void Logger::resume() {
    portENTER_CRITICAL(&_mux);
    _active = true;
    portEXIT_CRITICAL(&_mux);
}

bool Logger::push(const LoggerRow& row) {
    if (!_rows) return false;

    LoggerRow enriched = row;

    // The rate error was formed as target - feedback in the same 400 Hz cycle,
    // so this reconstructs the exact signal seen by each PID axis. With the
    // experimental EKF-rate feedback gate disabled, this is the notch + LPF
    // gyro signal. If that gate is enabled later, these columns still truthfully
    // report the active feedback signal rather than assuming its source.
    enriched.feedback_roll_rate_dps =
        row.target_roll_rate_dps - row.rate_roll_error_dps;
    enriched.feedback_pitch_rate_dps =
        row.target_pitch_rate_dps - row.rate_pitch_error_dps;
    enriched.feedback_yaw_rate_dps =
        row.target_yaw_rate_dps - row.rate_yaw_error_dps;

    enriched.bias_corrected_roll_rate_dps =
        enriched.feedback_roll_rate_dps - row.ekf_bgx_dps;
    enriched.bias_corrected_pitch_rate_dps =
        enriched.feedback_pitch_rate_dps - row.ekf_bgy_dps;
    enriched.bias_corrected_yaw_rate_dps =
        enriched.feedback_yaw_rate_dps - row.ekf_bgz_dps;

    const LoggerEkfRateSnapshot ekfRates = loggerReadEkfRateSnapshot();
    const bool ekfRatesValid = (row.ahrs_mode == 0U) && ekfRates.valid;
    enriched.ekf_rate_valid = ekfRatesValid ? 1U : 0U;
    enriched.ekf_roll_rate_dps = ekfRatesValid ? ekfRates.roll_rate_dps : 0.0f;
    enriched.ekf_pitch_rate_dps = ekfRatesValid ? ekfRates.pitch_rate_dps : 0.0f;
    enriched.ekf_yaw_rate_dps = ekfRatesValid ? ekfRates.yaw_rate_dps : 0.0f;

    portENTER_CRITICAL(&_mux);
    if (!_active) {
        _dropped++;
        portEXIT_CRITICAL(&_mux);
        return false;
    }
    _rows[_head] = enriched;
    if (++_head >= _capacity) { _head = 0; _full = true; }
    portEXIT_CRITICAL(&_mux);
    return true;
}

uint16_t Logger::_availableUnsafe() const { return _full ? _capacity : _head; }

uint16_t Logger::countAndFreeze() {
    portENTER_CRITICAL(&_mux);
    _active = false;
    uint16_t n = _availableUnsafe();
    portEXIT_CRITICAL(&_mux);
    return n;
}

uint16_t Logger::_mapChronologicalIndex(uint16_t chronologicalIndex) const {
    uint16_t n = _availableUnsafe();
    if (n == 0 || chronologicalIndex >= n) return 0;
    return _full ? (uint16_t)((_head + chronologicalIndex) % _capacity) : chronologicalIndex;
}

void Logger::_appendFloat(String& s, float v, uint8_t dp) {
    s += ',';
    if (isnan(v) || isinf(v)) s += '0';
    else s += String(v, (unsigned int)dp);
}

String Logger::csvHeader() const {
    return F("t_us,loop_count,period_us,jitter_us,control_exec_us,imu_read_us,rc_read_us,ahrs_exec_us,pid_exec_us,motor_exec_us,"
             "mode,armed,imuOk,rcValid,magValid,motorSaturated,rpmActualValid,"
             "thr,rcRoll,rcPitch,rcYaw,"
             "targetRollDeg,targetPitchDeg,targetYawDeg,targetRollRateDps,targetPitchRateDps,targetYawRateDps,"
             "ekfRoll,ekfPitch,ekfYaw,ctrlRoll,ctrlPitch,ctrlYaw,zeroRoll,zeroPitch,zeroYaw,"
             "ax,ay,az,gxRaw,gyRaw,gzRaw,mx,my,mz,accelNorm,gyroNorm,magNorm,ekfBgx,ekfBgy,ekfBgz,"
             "feedbackRollRateDps,feedbackPitchRateDps,feedbackYawRateDps,"
             "biasCorrectedRollRateDps,biasCorrectedPitchRateDps,biasCorrectedYawRateDps,"
             "ekfRollRateDps,ekfPitchRateDps,ekfYawRateDps,ekfRateValid,ahrsMode,ekfMagUsed,"
             "angleRollErr,anglePitchErr,yawErr,rateRollErr,ratePitchErr,rateYawErr,"
             "angleRollP,angleRollI,angleRollD,angleRollOut,anglePitchP,anglePitchI,anglePitchD,anglePitchOut,angleYawP,angleYawI,angleYawD,angleYawOut,"
             "rateRollP,rateRollI,rateRollD,rateRollOut,ratePitchP,ratePitchI,ratePitchD,ratePitchOut,rateYawP,rateYawI,rateYawD,rateYawOut,"
             "motorFLPre,motorFRPre,motorRLPre,motorRRPre,motFL,motFR,motRL,motRR,motorDiagA,motorDiagB,motorDiagDiff,"
             "rpmFL,rpmFR,rpmRL,rpmRR,rpmDiagA,rpmDiagB,rpmDiagDiff,batteryV,cpu0,cpu1,notchFreqHz,notchQ,notchEnable,bmpAltM,bmpVzMps,gpsValid,gpsSats,gpsHdop\n");
}

String Logger::csvRow(uint16_t chronologicalIndex) const {
    if (!_rows) return String();
    LoggerRow r; bool ok = false;
    portENTER_CRITICAL(const_cast<portMUX_TYPE*>(&_mux));
    uint16_t n = _availableUnsafe();
    if (chronologicalIndex < n) { r = _rows[_mapChronologicalIndex(chronologicalIndex)]; ok = true; }
    portEXIT_CRITICAL(const_cast<portMUX_TYPE*>(&_mux));
    if (!ok) return String();

    String s; s.reserve(1800);
    s += String(r.t_us); s += ','; s += String(r.loop_count); s += ','; s += String(r.period_us); s += ',';
    s += String(r.jitter_us); s += ','; s += String(r.control_exec_us); s += ','; s += String(r.imu_read_us); s += ',';
    s += String(r.rc_read_us); s += ','; s += String(r.ahrs_exec_us); s += ','; s += String(r.pid_exec_us); s += ',';
    s += String(r.motor_exec_us); s += ','; s += String(r.mode); s += ',';
    s += String((r.flags & 0x0001) ? 1 : 0); s += ','; s += String((r.flags & 0x0002) ? 1 : 0); s += ',';
    s += String((r.flags & 0x0004) ? 1 : 0); s += ','; s += String((r.flags & 0x0008) ? 1 : 0); s += ',';
    s += String((r.flags & 0x0010) ? 1 : 0); s += ','; s += String((r.flags & 0x0020) ? 1 : 0);

    _appendFloat(s,r.throttle,4); _appendFloat(s,r.rc_roll,4); _appendFloat(s,r.rc_pitch,4); _appendFloat(s,r.rc_yaw,4);
    _appendFloat(s,r.target_roll_deg,3); _appendFloat(s,r.target_pitch_deg,3); _appendFloat(s,r.target_yaw_deg,3);
    _appendFloat(s,r.target_roll_rate_dps,3); _appendFloat(s,r.target_pitch_rate_dps,3); _appendFloat(s,r.target_yaw_rate_dps,3);
    _appendFloat(s,r.ekf_roll_deg,3); _appendFloat(s,r.ekf_pitch_deg,3); _appendFloat(s,r.ekf_yaw_deg,3);
    _appendFloat(s,r.ctrl_roll_deg,3); _appendFloat(s,r.ctrl_pitch_deg,3); _appendFloat(s,r.ctrl_yaw_deg,3);
    _appendFloat(s,r.zero_roll_deg,3); _appendFloat(s,r.zero_pitch_deg,3); _appendFloat(s,r.zero_yaw_deg,3);
    _appendFloat(s,r.ax_g,5); _appendFloat(s,r.ay_g,5); _appendFloat(s,r.az_g,5);
    _appendFloat(s,r.gx_dps,3); _appendFloat(s,r.gy_dps,3); _appendFloat(s,r.gz_dps,3);
    _appendFloat(s,r.mx_uT,3); _appendFloat(s,r.my_uT,3); _appendFloat(s,r.mz_uT,3);
    _appendFloat(s,r.accel_norm_g,5); _appendFloat(s,r.gyro_norm_dps,3); _appendFloat(s,r.mag_norm_uT,3);
    _appendFloat(s,r.ekf_bgx_dps,5); _appendFloat(s,r.ekf_bgy_dps,5); _appendFloat(s,r.ekf_bgz_dps,5);

    _appendFloat(s,r.feedback_roll_rate_dps,3);
    _appendFloat(s,r.feedback_pitch_rate_dps,3);
    _appendFloat(s,r.feedback_yaw_rate_dps,3);
    _appendFloat(s,r.bias_corrected_roll_rate_dps,3);
    _appendFloat(s,r.bias_corrected_pitch_rate_dps,3);
    _appendFloat(s,r.bias_corrected_yaw_rate_dps,3);
    _appendFloat(s,r.ekf_roll_rate_dps,3);
    _appendFloat(s,r.ekf_pitch_rate_dps,3);
    _appendFloat(s,r.ekf_yaw_rate_dps,3);
    s += ','; s += String(r.ekf_rate_valid);
    s += ','; s += String(r.ahrs_mode); s += ','; s += String(r.ekf_mag_used);

    _appendFloat(s,r.angle_roll_error_deg,3); _appendFloat(s,r.angle_pitch_error_deg,3); _appendFloat(s,r.yaw_error_deg,3);
    _appendFloat(s,r.rate_roll_error_dps,3); _appendFloat(s,r.rate_pitch_error_dps,3); _appendFloat(s,r.rate_yaw_error_dps,3);

    _appendFloat(s,r.angle_roll_p,6); _appendFloat(s,r.angle_roll_i,6); _appendFloat(s,r.angle_roll_d,6); _appendFloat(s,r.angle_roll_out,6);
    _appendFloat(s,r.angle_pitch_p,6); _appendFloat(s,r.angle_pitch_i,6); _appendFloat(s,r.angle_pitch_d,6); _appendFloat(s,r.angle_pitch_out,6);
    _appendFloat(s,r.angle_yaw_p,6); _appendFloat(s,r.angle_yaw_i,6); _appendFloat(s,r.angle_yaw_d,6); _appendFloat(s,r.angle_yaw_out,6);
    _appendFloat(s,r.rate_roll_p,6); _appendFloat(s,r.rate_roll_i,6); _appendFloat(s,r.rate_roll_d,6); _appendFloat(s,r.rate_roll_out,6);
    _appendFloat(s,r.rate_pitch_p,6); _appendFloat(s,r.rate_pitch_i,6); _appendFloat(s,r.rate_pitch_d,6); _appendFloat(s,r.rate_pitch_out,6);
    _appendFloat(s,r.rate_yaw_p,6); _appendFloat(s,r.rate_yaw_i,6); _appendFloat(s,r.rate_yaw_d,6); _appendFloat(s,r.rate_yaw_out,6);

    _appendFloat(s,r.motor_fl_pre,4); _appendFloat(s,r.motor_fr_pre,4); _appendFloat(s,r.motor_rl_pre,4); _appendFloat(s,r.motor_rr_pre,4);
    _appendFloat(s,r.motor_fl,4); _appendFloat(s,r.motor_fr,4); _appendFloat(s,r.motor_rl,4); _appendFloat(s,r.motor_rr,4);
    _appendFloat(s,r.motor_diag_a,4); _appendFloat(s,r.motor_diag_b,4); _appendFloat(s,r.motor_diag_diff,4);
    _appendFloat(s,r.rpm_fl,0); _appendFloat(s,r.rpm_fr,0); _appendFloat(s,r.rpm_rl,0); _appendFloat(s,r.rpm_rr,0);
    _appendFloat(s,r.rpm_diag_a,0); _appendFloat(s,r.rpm_diag_b,0); _appendFloat(s,r.rpm_diag_diff,0);
    _appendFloat(s,r.battery_v,3); _appendFloat(s,r.cpu0_pct,1); _appendFloat(s,r.cpu1_pct,1);
    _appendFloat(s,r.notch_freq_hz,2); _appendFloat(s,r.notch_q,2); s += ','; s += String(r.notch_enable);
    _appendFloat(s,r.bmp_alt_m,3); _appendFloat(s,r.bmp_vz_mps,3); s += ','; s += String(r.gps_valid); s += ','; s += String(r.gps_sats); _appendFloat(s,r.gps_hdop,2);
    s += '\n';
    return s;
}

String Logger::statusJson() const {
    uint16_t n; bool active; bool full; uint16_t head; uint32_t dropped;
    portENTER_CRITICAL(const_cast<portMUX_TYPE*>(&_mux));
    n = _availableUnsafe(); active = _active; full = _full; head = _head; dropped = _dropped;
    portEXIT_CRITICAL(const_cast<portMUX_TYPE*>(&_mux));
    String j; j.reserve(180);
    j += F("{\"ok\":true");
    j += F(",\"active\":"); j += active ? F("true") : F("false");
    j += F(",\"frozen\":"); j += active ? F("false") : F("true");
    j += F(",\"full\":"); j += full ? F("true") : F("false");
    j += F(",\"count\":"); j += String(n);
    j += F(",\"capacity\":"); j += String(_capacity);
    j += F(",\"head\":"); j += String(head);
    j += F(",\"dropped\":"); j += String(dropped);
    j += '}';
    return j;
}
