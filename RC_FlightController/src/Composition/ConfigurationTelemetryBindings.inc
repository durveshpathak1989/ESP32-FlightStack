//  Tuning sync helpers
// ─────────────────────────────────────────────────────────────
static void syncTuningFromObjects()
{
    // Copy compile-time defaults into runtime-tunable state at boot.
    g_tuning.max_angle_deg              = TUNE_MAX_ANGLE_DEG;
    g_tuning.max_rate_dps               = TUNE_MAX_RATE_DPS;
    g_tuning.max_pitch_rate_dps         = TUNE_MAX_RATE_DPS;

    g_tuning.roll_output_limit          = TUNE_ROLL_OUTPUT_LIMIT;
    g_tuning.pitch_output_limit         = TUNE_PITCH_OUTPUT_LIMIT;
    g_tuning.yaw_output_limit           = TUNE_YAW_OUTPUT_LIMIT;

    g_tuning.throttle_expo              = TUNE_THROTTLE_EXPO;
    g_tuning.throttle_up_rate_per_sec   = TUNE_THROTTLE_UP_RATE_PER_SEC;
    g_tuning.throttle_down_rate_per_sec = TUNE_THROTTLE_DOWN_RATE_PER_SEC;
    g_tuning.motor_idle                 = TUNE_MOTOR_IDLE;
    g_tuning.motor_max                  = TUNE_MOTOR_MAX;
    g_tuning.throttle_cut               = TUNE_THROTTLE_CUT;
    g_tuning.idle_ramp_end              = TUNE_IDLE_RAMP_END;
    g_tuning.pid_ilimit                 = pidRateRoll.iLimit;

    g_tuning.pid_roll_kp                = pidRateRoll.kp;
    g_tuning.pid_roll_ki                = pidRateRoll.ki;
    g_tuning.pid_roll_kd                = pidRateRoll.kd;
    g_tuning.pid_pitch_kp               = pidRatePitch.kp;
    g_tuning.pid_pitch_ki               = pidRatePitch.ki;
    g_tuning.pid_pitch_kd               = pidRatePitch.kd;
    g_tuning.pid_yaw_kp                 = pidRateYaw.kp;
    g_tuning.pid_yaw_ki                 = pidRateYaw.ki;
    g_tuning.pid_yaw_kd                 = pidRateYaw.kd;
    g_tuning.pid_roll_ff                = TUNE_RATE_ROLL_FF;
    g_tuning.pid_pitch_ff               = TUNE_RATE_PITCH_FF;
    g_tuning.pid_yaw_ff                 = TUNE_RATE_YAW_FF;
    g_tuning.pid_roll_d_lpf_hz          = TUNE_RATE_ROLL_D_LPF_HZ;
    g_tuning.pid_pitch_d_lpf_hz         = TUNE_RATE_PITCH_D_LPF_HZ;
    g_tuning.pid_yaw_d_lpf_hz           = TUNE_RATE_YAW_D_LPF_HZ;

    g_tuning.pid_angle_roll_kp          = pidAngleRoll.kp;
    g_tuning.pid_angle_roll_ki          = pidAngleRoll.ki;
    g_tuning.pid_angle_roll_kd          = pidAngleRoll.kd;
    g_tuning.pid_angle_pitch_kp         = pidAnglePitch.kp;
    g_tuning.pid_angle_pitch_ki         = pidAnglePitch.ki;
    g_tuning.pid_angle_pitch_kd         = pidAnglePitch.kd;
    g_tuning.pid_angle_yaw_kp           = pidAngleYaw.kp;
    g_tuning.yaw_deadband               = TUNE_YAW_DEADBAND;
    g_tuning.yaw_max_rate_dps           = TUNE_YAW_MAX_RATE_DPS;

    g_tuning.mahony_kp                  = mahony.kp();
    g_tuning.mahony_ki                  = mahony.ki();
    g_tuning.ahrs_filter_mode           = TUNE_AHRS_FILTER_MODE;
    g_tuning.madgwick_beta              = TUNE_MADGWICK_BETA;
    g_tuning.notch_enable               = TUNE_NOTCH_ENABLE;
    g_tuning.notch_freq_hz              = TUNE_NOTCH_FREQ_HZ;
    g_tuning.notch_q                    = TUNE_NOTCH_Q;
    g_tuning.ekf_angle_q                = TUNE_EKF_ANGLE_Q;
    g_tuning.ekf_bias_q                 = TUNE_EKF_BIAS_Q;
    g_tuning.ekf_accel_r                = TUNE_EKF_ACCEL_R;
    g_tuning.ekf_mag_r                  = TUNE_EKF_MAG_R;
    g_tuning.ekf_mag_declination_deg    = TUNE_EKF_MAG_DECL_DEG;
    g_tuning.ekf_mag_yaw_offset_deg     = TUNE_EKF_MAG_YAW_OFF_DEG;
    g_tuning.ekf_mag_yaw_sign           = TUNE_EKF_MAG_YAW_SIGN;
    g_tuning.dirty                      = false;
}
// Copy g_tuning into live PID/AHRS objects. Caller already holds tuningRte.
static void applyTuningToObjectsLocked()
{
    pidRateRoll.kp         = g_tuning.pid_roll_kp;
    pidRateRoll.ki         = g_tuning.pid_roll_ki;
    pidRateRoll.kd         = g_tuning.pid_roll_kd;
    pidRatePitch.kp        = g_tuning.pid_pitch_kp;
    pidRatePitch.ki        = g_tuning.pid_pitch_ki;
    pidRatePitch.kd        = g_tuning.pid_pitch_kd;
    pidRateYaw.kp          = g_tuning.pid_yaw_kp;
    pidRateYaw.ki          = g_tuning.pid_yaw_ki;
    pidRateYaw.kd          = g_tuning.pid_yaw_kd;

    pidAngleRoll.kp        = g_tuning.pid_angle_roll_kp;
    pidAngleRoll.ki        = g_tuning.pid_angle_roll_ki;
    pidAngleRoll.kd        = g_tuning.pid_angle_roll_kd;
    pidAnglePitch.kp       = g_tuning.pid_angle_pitch_kp;
    pidAnglePitch.ki       = g_tuning.pid_angle_pitch_ki;
    pidAnglePitch.kd       = g_tuning.pid_angle_pitch_kd;
    pidAngleYaw.kp         = g_tuning.pid_angle_yaw_kp;

    // Apply the global I-limit to every PID object so the Config tab lever works.
    pidRateRoll.iLimit     = g_tuning.pid_ilimit;
    pidRatePitch.iLimit    = g_tuning.pid_ilimit;
    pidRateYaw.iLimit      = g_tuning.pid_ilimit;
    pidAngleRoll.iLimit    = g_tuning.pid_ilimit;
    pidAnglePitch.iLimit   = g_tuning.pid_ilimit;
    pidAngleYaw.iLimit     = g_tuning.pid_ilimit;

    mahony.setGains(g_tuning.mahony_kp, g_tuning.mahony_ki);
    madgwickAHRS.setBeta(constrain(g_tuning.madgwick_beta, 0.001f, 1.000f));
    g_tuning.ahrs_filter_mode = constrain(g_tuning.ahrs_filter_mode, 0.0f, 2.0f);

    const float notchHz = constrain(g_tuning.notch_freq_hz, 1.0f, NOTCH_SAMPLE_HZ * 0.45f);
    const float notchQ  = constrain(g_tuning.notch_q, 0.5f, 50.0f);
    notchAx.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchAy.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchAz.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGx.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGy.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);
    notchGz.configure(NOTCH_SAMPLE_HZ, notchHz, notchQ, g_tuning.notch_enable);

    attitudeEKF.setProcessNoise(constrain(g_tuning.ekf_angle_q, 0.000001f, 0.050000f),
                                constrain(g_tuning.ekf_bias_q,  0.000000001f, 0.001000f));
    attitudeEKF.setAccelMeasurementNoise(constrain(g_tuning.ekf_accel_r, 0.001f, 2.000f));
    attitudeEKF.setMagMeasurementNoise(constrain(g_tuning.ekf_mag_r, 0.001f, 5.000f));
    attitudeEKF.setMagDeclinationDeg(constrain(g_tuning.ekf_mag_declination_deg, -30.0f, 30.0f));
    attitudeEKF.setMagYawOffsetDeg(constrain(g_tuning.ekf_mag_yaw_offset_deg, -180.0f, 180.0f));
    attitudeEKF.setMagYawSign(g_tuning.ekf_mag_yaw_sign);

    g_tuning.dirty         = false;
    g_tuneApplySeq         = g_tuneRequestSeq;
}

static void applyTuningToObjects()
{
    if (!tuningRte.lock(0)) return;
    applyTuningToObjectsLocked();
    g_notchResetPending = true;
    tuningRte.unlock();
}
// ─────────────────────────────────────────────────────────────
//  Log helpers (push to WiFi ring buffer + Serial)
// ─────────────────────────────────────────────────────────────
static void calLog(const char* msg)  { telemetryWiFi.pushLog(msg); }
static void calLogf(const char* fmt, ...) {
    char buf[WIFI_LOG_LINE_LEN];
    va_list args; va_start(args,fmt); vsnprintf(buf,sizeof(buf),fmt,args); va_end(args);
    telemetryWiFi.pushLog(buf);
}

// ─────────────────────────────────────────────────────────────
//  Timing JSON provider — /timing endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingJson()
{
    if (!timingRte.lock(pdMS_TO_TICKS(20))) {
        return F("{\"ok\":false,\"error\":\"mutex timeout\"}");
    }

    uint32_t count      = g_timing.count;
    double   periodMean = g_timing.wMean;
    double   periodStd  = (count > 1) ? sqrt(g_timing.wM2 / (count - 1)) : 0.0;
    double   jitterMean = g_timing.jMean;
    double   jitterStd  = (count > 1) ? sqrt(g_timing.jM2 / (count - 1)) : 0.0;
    uint32_t jitterMax  = g_timing.jitterMax;
    uint32_t violations = g_timing.jitterViolations;

    static uint16_t sortBuf[TIMING_BUF_SIZE];
    uint16_t n    = g_timing.bufFull ? TIMING_BUF_SIZE : g_timing.bufHead;
    uint16_t head = g_timing.bufFull ? g_timing.bufHead : 0;
    for (uint16_t i = 0; i < n; i++) {
        uint16_t period = g_timing.buf[(head + i) % TIMING_BUF_SIZE];
        int32_t  jit    = (int32_t)period - (int32_t)TIMING_TARGET_US;
        if (jit < 0) jit = -jit;
        sortBuf[i] = (uint16_t)min((uint32_t)jit, (uint32_t)65535);
    }
    timingRte.unlock();

    for (uint16_t i = 1; i < n; i++) {
        uint16_t key = sortBuf[i]; int j = i - 1;
        while (j >= 0 && sortBuf[j] > key) { sortBuf[j+1]=sortBuf[j]; j--; }
        sortBuf[j+1] = key;
    }

    float p99 = 0.0f;
    if (n > 0) {
        uint16_t idx = (uint16_t)(0.99f * (float)(n - 1));
        p99 = (float)sortBuf[idx];
    }

    const float targetHz = 1000000.0f / (float)TIMING_TARGET_US;
    const float violationRatePct = (count > 0)
                                 ? (100.0f * (float)violations / (float)count)
                                 : 0.0f;

    String j;
    j.reserve(520);
    j += F("{\"ok\":true");
    j += F(",\"loop_name\":\"control\"");
    j += F(",\"target_us\":");      j += String(TIMING_TARGET_US);
    j += F(",\"period_target_us\":"); j += String(TIMING_TARGET_US); // alias for GCS clarity
    j += F(",\"target_hz\":");      j += String(targetHz, 1);
    j += F(",\"violation_thresh_us\":"); j += String(JITTER_VIOLATION_US);
    j += F(",\"count\":");          j += String(count);
    j += F(",\"period_mean_us\":"); j += String((float)periodMean, 2);
    j += F(",\"period_std_us\":");  j += String((float)periodStd,  2);
    j += F(",\"jitter_mean_us\":"); j += String((float)jitterMean, 2);
    j += F(",\"jitter_std_us\":");  j += String((float)jitterStd,  2);
    j += F(",\"jitter_max_us\":");  j += String(jitterMax);
    j += F(",\"jitter_p99_us\":");  j += String(p99, 1);
    j += F(",\"violations\":");     j += String(violations);
    j += F(",\"violation_rate_pct\":"); j += String(violationRatePct, 3);
    j += F(",\"buf_samples\":");    j += String(n);
    j += '}';
    return j;
}

static String provideIdentityJson()
{
    String j;
    j.reserve(260);
    j += F("{\"ok\":true");
    j += F(",\"name\":\"");         j += FW_NAME;             j += '"';
    j += F(",\"version\":\"");      j += FW_VERSION;          j += '"';
    j += F(",\"sourceBranch\":\""); j += FW_SOURCE_BRANCH;    j += '"';
    j += F(",\"sourceCommit\":\""); j += FW_SOURCE_COMMIT;    j += '"';
    j += F(",\"sourceDirty\":");    j += FW_SOURCE_DIRTY ? F("true") : F("false");
    j += F(",\"buildId\":\"");      j += FW_BUILD_ID;         j += '"';
    j += F(",\"buildTime\":\"");    j += FW_BUILD_TIME_ISO;   j += '"';
    j += '}';
    return j;
}

// ─────────────────────────────────────────────────────────────
//  Timing CSV provider — /timing/csv endpoint (Test 7.1)
// ─────────────────────────────────────────────────────────────
static String provideTimingCsv()
{
    if (!timingRte.lock(pdMS_TO_TICKS(20))) {
        return F("error,mutex_timeout\n");
    }
    uint16_t n    = g_timing.bufFull ? TIMING_BUF_SIZE : g_timing.bufHead;
    uint16_t head = g_timing.bufFull ? g_timing.bufHead : 0;

    static uint16_t tmp[TIMING_BUF_SIZE];
    for (uint16_t i = 0; i < n; i++) {
        tmp[i] = g_timing.buf[(head + i) % TIMING_BUF_SIZE];
    }
    timingRte.unlock();

    String csv;
    csv.reserve((size_t)n * 18 + 32);
    csv += F("index,period_us,jitter_us\n");
    for (uint16_t i = 0; i < n; i++) {
        int32_t jit = (int32_t)tmp[i] - TIMING_TARGET_US;
        if (jit < 0) jit = -jit;
        csv += String(i);  csv += ',';
        csv += String(tmp[i]); csv += ',';
        csv += String((uint32_t)jit); csv += '\n';
    }
    return csv;
}

// ─────────────────────────────────────────────────────────────
//  Wi-Fi telemetry provider — /telemetry endpoint
// ─────────────────────────────────────────────────────────────
static const char* flightModeToStr(flight::FlightMode m) {
    switch(m) {
        case flight::FlightMode::Disarmed: return "DISARMED";
        case flight::FlightMode::Angle:    return "ANGLE";
        case flight::FlightMode::Acro:     return "ACRO";
        case flight::FlightMode::Failsafe: return "FAILSAFE";
        default:                   return "UNKNOWN";
    }
}

static void updateControlTransitionCounters(const flight::PilotCommand& cmd)
{
    const bool armedNow = cmd.valid &&
                          cmd.mode != flight::FlightMode::Disarmed &&
                          cmd.mode != flight::FlightMode::Failsafe;
    const uint32_t nowMs = clockServiceAdapter.milliseconds();

    if (!g_transitionTrackerInitialized) {
        g_transitionTrackerInitialized = true;
        g_lastObservedMode = cmd.mode;
        g_lastObservedArmed = armedNow;
        return;
    }

    bool resetEvent = false;
    if (cmd.mode != g_lastObservedMode) {
        g_lastObservedMode = cmd.mode;
        g_modeTransitionCount++;
        g_lastModeChangeMs = nowMs;
        resetEvent = true;
    }

    if (armedNow != g_lastObservedArmed) {
        g_lastObservedArmed = armedNow;
        g_armingTransitionCount++;
        g_lastArmChangeMs = nowMs;
        resetEvent = true;
    }

    if (resetEvent) g_pidResetCount++;
}

static bool provideTelemetry(TelemetryPacket& out)
{
    FlightState s;
    if (!flightStateRte.read(s, pdMS_TO_TICKS(15))) return false;

    TuningState t;
    memset(&t, 0, sizeof(t));
    if (tuningRte.read(t, pdMS_TO_TICKS(5))) {
    }

    out.tick      = s.loopCount;
    out.mode      = flightModeToStr(s.rc.mode);
    out.armed     = s.armed;
    out.rc_valid  = s.rc.valid;
    out.imu_valid = s.imuValid;
    out.mag_valid = s.magValid;
    out.ahrs_filter_mode = s.ahrsFilterMode;
    out.ahrs_filter = (s.ahrsFilterMode == 1) ? "Mahony" : ((s.ahrsFilterMode == 2) ? "Madgwick" : "EKF");
    out.roll_deg  = s.roll_deg;  out.pitch_deg = s.pitch_deg; out.yaw_deg = s.yaw_deg;
    out.q0=s.q0; out.q1=s.q1; out.q2=s.q2; out.q3=s.q3;
    out.roll_ctrl_deg  = s.roll_ctrl_deg;
    out.pitch_ctrl_deg = s.pitch_ctrl_deg;
    out.yaw_ctrl_deg   = s.yaw_ctrl_deg;
    out.roll_offset_deg  = s.roll_offset_deg;
    out.pitch_offset_deg = s.pitch_offset_deg;
    out.yaw_offset_deg   = s.yaw_offset_deg;
    out.ax_g=s.ax_g; out.ay_g=s.ay_g; out.az_g=s.az_g;
    out.gx_dps=s.gx_dps; out.gy_dps=s.gy_dps; out.gz_dps=s.gz_dps;
    out.mx_uT=s.mx_uT;   out.my_uT=s.my_uT;   out.mz_uT=s.mz_uT;
    out.imu_temp_c=s.imuTemp_c;
    out.raw_ax_g=s.rawAx_g; out.raw_ay_g=s.rawAy_g; out.raw_az_g=s.rawAz_g;
    out.raw_gx_dps=s.rawGx_dps; out.raw_gy_dps=s.rawGy_dps; out.raw_gz_dps=s.rawGz_dps;
    out.filt_ax_g=s.filtAx_g; out.filt_ay_g=s.filtAy_g; out.filt_az_g=s.filtAz_g;
    out.filt_gx_dps=s.filtGx_dps; out.filt_gy_dps=s.filtGy_dps; out.filt_gz_dps=s.filtGz_dps;
    out.mag_norm_uT=s.magNorm_uT;
    out.ekf_mag_used=s.ekfMagUsed;
    out.ekf_bgx_dps=s.ekfBgx_dps; out.ekf_bgy_dps=s.ekfBgy_dps; out.ekf_bgz_dps=s.ekfBgz_dps;
    out.target_roll_deg=s.targetRollDeg; out.target_pitch_deg=s.targetPitchDeg; out.target_yaw_deg=s.targetYawDeg;
    out.target_roll_rate_dps=s.targetRollRateDps; out.target_pitch_rate_dps=s.targetPitchRateDps; out.target_yaw_rate_dps=s.targetYawRateDps;
    out.roll_rate_error_dps=s.rollRateError_dps; out.pitch_rate_error_dps=s.pitchRateError_dps; out.yaw_rate_error_dps=s.yawRateError_dps;
    out.yaw_error_deg=s.yawError_deg;
    out.motor_saturated=s.motorSaturated;
    out.angle_mode_active=s.angleModeActive; out.acro_mode_active=s.acroModeActive;
    out.rc_ch1_us=s.rc.raw[0]; out.rc_ch2_us=s.rc.raw[1]; out.rc_ch3_us=s.rc.raw[2]; out.rc_ch4_us=s.rc.raw[3]; out.rc_ch5_us=s.rc.raw[4];
    out.rc_ch6_us=s.rc.raw[5]; out.rc_ch7_us=s.rc.raw[6]; out.rc_ch8_us=s.rc.raw[7]; out.rc_ch9_us=s.rc.raw[8]; out.rc_ch10_us=s.rc.raw[9];
    out.rc_failsafe_count=s.rcFailsafeCount;
    out.mode_switch_raw_us=s.modeSwitchRaw_us; out.arm_switch_raw_us=s.armSwitchRaw_us;
    out.aux_tune1_raw_us=s.auxTune1Raw_us; out.aux_tune2_raw_us=s.auxTune2Raw_us;
    out.angle_loop_enabled=s.angleLoopEnabled; out.rate_loop_enabled=s.rateLoopEnabled;
    out.actual_roll_rate_dps=s.actualRollRate_dps;
    out.actual_pitch_rate_dps=s.actualPitchRate_dps;
    out.actual_yaw_rate_dps=s.actualYawRate_dps;
    out.angle_roll_p=s.angleRollP; out.angle_roll_i=s.angleRollI; out.angle_roll_d=s.angleRollD;
    out.angle_pitch_p=s.anglePitchP; out.angle_pitch_i=s.anglePitchI; out.angle_pitch_d=s.anglePitchD;
    out.rate_roll_p=s.rateRollP; out.rate_roll_i=s.rateRollI; out.rate_roll_d=s.rateRollD;
    out.rate_pitch_p=s.ratePitchP; out.rate_pitch_i=s.ratePitchI; out.rate_pitch_d=s.ratePitchD;
    out.rate_yaw_p=s.rateYawP; out.rate_yaw_i=s.rateYawI; out.rate_yaw_d=s.rateYawD;
    out.angle_roll_iterm=s.angleRollIterm; out.angle_pitch_iterm=s.anglePitchIterm;
    out.pid_reset_count=s.pidResetCount;
    out.mode_transition_count=s.modeTransitionCount; out.last_mode_change_ms=s.lastModeChange_ms;
    out.arming_transition_count=s.armingTransitionCount; out.last_arm_change_ms=s.lastArmChange_ms;
    out.throttle_low=s.throttleLow;
    out.control_authority_remaining=s.controlAuthorityRemaining;
    out.roll_output_limited=s.rollOutputLimited;
    out.pitch_output_limited=s.pitchOutputLimited;
    out.yaw_output_limited=s.yawOutputLimited;
    out.rate_output_limited=s.rateOutputLimited;
    out.bmp_vertical_speed_mps=s.bmpVerticalSpeed_mps;

    out.accel_roll_deg  = s.accelRoll_deg;
    out.accel_pitch_deg = s.accelPitch_deg;
    out.gyro_roll_deg   = s.gyroRoll_deg;
    out.gyro_pitch_deg  = s.gyroPitch_deg;
    out.gyro_yaw_deg    = s.gyroYaw_deg;
    out.roll_angle_error_deg  = s.rollAngleError_deg;
    out.pitch_angle_error_deg = s.pitchAngleError_deg;
    out.pid_roll_out  = s.pidRollOut;
    out.pid_pitch_out = s.pidPitchOut;
    out.pid_yaw_out   = s.pidYawOut;
    out.throttle=s.rc.throttle; out.rc_roll=s.rc.roll;
    out.rc_pitch=s.rc.pitch;    out.rc_yaw=s.rc.yaw;
    out.rc_hz=s.rcFrameRateHz;
    out.battery_voltage_v=s.batteryVoltage_v;
    out.battery_adc_v=s.batteryAdcVoltage_v;
    out.battery_cell_v=s.batteryCellVoltage_v;
    out.battery_percent=s.batteryPercent;
    out.battery_valid=s.batteryValid;
    out.battery_low=s.batteryLow;
    out.battery_critical=s.batteryCritical;
    out.motor_fl=s.motorFL; out.motor_fr=s.motorFR;
    out.motor_rl=s.motorRL; out.motor_rr=s.motorRR;
    out.motor_fl_pre_sat=s.motorFLPreSat; out.motor_fr_pre_sat=s.motorFRPreSat;
    out.motor_rl_pre_sat=s.motorRLPreSat; out.motor_rr_pre_sat=s.motorRRPreSat;
    out.loop_period_us=s.loopPeriod_us;
    out.loop_jitter_us=s.loopJitter_us;
    out.imu_read_us=s.imuRead_us;
    out.rc_read_us=s.rcRead_us;
    out.ahrs_update_us=s.ahrsUpdate_us;
    out.control_update_us=s.controlUpdate_us;
    out.motor_write_us=s.motorWrite_us;
    out.wifi_service_us=s.wifiService_us;
    out.onboard_log_write_us=s.onboardLogWrite_us;
    out.missed_loop_count=s.missedLoopCount;
    out.cmd_rpm_fl=s.cmdRpmFL; out.cmd_rpm_fr=s.cmdRpmFR;
    out.cmd_rpm_rl=s.cmdRpmRL; out.cmd_rpm_rr=s.cmdRpmRR;
    // Backward-compatible aliases: until ESC RPM telemetry is added, rpm* means commanded/estimated RPM.
    out.rpm_fl=out.cmd_rpm_fl; out.rpm_fr=out.cmd_rpm_fr;
    out.rpm_rl=out.cmd_rpm_rl; out.rpm_rr=out.cmd_rpm_rr;
    out.actual_rpm_fl=s.actualRpmFL; out.actual_rpm_fr=s.actualRpmFR;
    out.actual_rpm_rl=s.actualRpmRL; out.actual_rpm_rr=s.actualRpmRR;
    out.rpm_actual_valid=s.rpmActualValid;
    out.bmp_temp_c=s.bmpTemp_c; out.bmp_pressure_hpa=s.bmpPressure_hpa;
    out.bmp_altitude_m=s.bmpAltitude_m; out.bmp_valid=s.bmpValid;
    out.cpu_core0_pct=s.cpuCore0_pct; out.cpu_core1_pct=s.cpuCore1_pct;
    out.cpu_valid=s.cpuValid;

    out.max_angle_deg              = t.max_angle_deg;
    out.max_rate_dps               = t.max_rate_dps;
    out.max_pitch_rate_dps         = t.max_pitch_rate_dps;
    out.roll_output_limit          = t.roll_output_limit;
    out.pitch_output_limit         = t.pitch_output_limit;
    out.yaw_output_limit           = t.yaw_output_limit;
    out.throttle_expo              = t.throttle_expo;
    out.throttle_up_rate_per_sec   = t.throttle_up_rate_per_sec;
    out.throttle_down_rate_per_sec = t.throttle_down_rate_per_sec;
    out.motor_idle                 = t.motor_idle;
    out.motor_max                  = t.motor_max;
    out.throttle_cut               = t.throttle_cut;
    out.idle_ramp_end              = t.idle_ramp_end;
    out.pid_ilimit                 = t.pid_ilimit;

    out.pid_roll_kp=t.pid_roll_kp;   out.pid_roll_ki=t.pid_roll_ki;   out.pid_roll_kd=t.pid_roll_kd;
    out.pid_pitch_kp=t.pid_pitch_kp; out.pid_pitch_ki=t.pid_pitch_ki; out.pid_pitch_kd=t.pid_pitch_kd;
    out.pid_yaw_kp=t.pid_yaw_kp;     out.pid_yaw_ki=t.pid_yaw_ki;     out.pid_yaw_kd=t.pid_yaw_kd;
    out.pid_roll_ff=t.pid_roll_ff;   out.pid_pitch_ff=t.pid_pitch_ff; out.pid_yaw_ff=t.pid_yaw_ff;
    out.pid_roll_d_lpf_hz=t.pid_roll_d_lpf_hz; out.pid_pitch_d_lpf_hz=t.pid_pitch_d_lpf_hz; out.pid_yaw_d_lpf_hz=t.pid_yaw_d_lpf_hz;
    out.pid_angle_roll_kp=t.pid_angle_roll_kp;
    out.pid_angle_roll_ki=t.pid_angle_roll_ki;
    out.pid_angle_roll_kd=t.pid_angle_roll_kd;
    out.pid_angle_pitch_kp=t.pid_angle_pitch_kp;
    out.pid_angle_pitch_ki=t.pid_angle_pitch_ki;
    out.pid_angle_pitch_kd=t.pid_angle_pitch_kd;
    out.pid_angle_yaw_kp=t.pid_angle_yaw_kp;
    out.yaw_deadband=t.yaw_deadband;
    out.yaw_max_rate_dps=t.yaw_max_rate_dps;
    out.mahony_kp=t.mahony_kp; out.mahony_ki=t.mahony_ki;
    out.ahrs_filter_mode=t.ahrs_filter_mode;
    out.madgwick_beta=t.madgwick_beta;
    out.notch_enable=t.notch_enable;
    out.notch_freq_hz=t.notch_freq_hz;
    out.notch_q=t.notch_q;
    out.ekf_angle_q=t.ekf_angle_q;
    out.ekf_bias_q=t.ekf_bias_q;
    out.ekf_accel_r=t.ekf_accel_r;
    out.ekf_mag_r=t.ekf_mag_r;
    out.ekf_mag_declination_deg=t.ekf_mag_declination_deg;
    out.ekf_mag_yaw_offset_deg=t.ekf_mag_yaw_offset_deg;
    out.ekf_mag_yaw_sign=t.ekf_mag_yaw_sign;
    out.tune_request_seq = g_tuneRequestSeq;
    out.tune_apply_seq   = g_tuneApplySeq;
    out.tune_reject_seq  = g_tuneRejectSeq;
    out.tune_dirty       = t.dirty;
    out.gps_valid       = s.gps.valid;
    out.gps_lat         = s.gps.latitude;
    out.gps_lon         = s.gps.longitude;
    out.gps_altitude_m  = s.gps.altitude_m;
    out.gps_speed_kmh   = s.gps.speed_kmh;
    out.gps_course_deg  = s.gps.course_deg;
    out.gps_satellites  = s.gps.satellites;
    out.gps_hdop        = s.gps.hdop;
    out.gps_fix_quality = s.gps.fix_quality;
    out.gps_hour        = s.gps.hour;
    out.gps_minute      = s.gps.minute;
    out.gps_second      = s.gps.second;
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Wi-Fi tune handler — /tune endpoint (Test 7.5)
// ─────────────────────────────────────────────────────────────
static bool handleTune(const TunePacket& in)
{
    bool armed = false;
    if (flightStateRte.lock(pdMS_TO_TICKS(5))) {
        armed = g_state.armed; flightStateRte.unlock();
    }
    if (armed) {
        g_tuneRejectSeq++;
        calLog("[TUNE] REJECTED — disarm first.");
        return false;
    }

    if (!tuningRte.lock(pdMS_TO_TICKS(10))) {
        g_tuneRejectSeq++;
        calLog("[TUNE] REJECTED — tune mutex timeout.");
        return false;
    }

    TelemetryTuneAdapter::apply(in, g_tuning, NOTCH_SAMPLE_HZ);
    // Accept and apply immediately while disarmed. This removes the confusing
    // one-cycle handshake where the POST succeeded but telemetry still showed
    // old values until taskControl got around to applying dirty tuning.
    g_tuneRequestSeq++;
    g_tuning.dirty = true;
    applyTuningToObjectsLocked();
    g_notchResetPending = true;
    const bool persisted = g_tuningStore.save(g_tuning);
    uint32_t appliedSeq = g_tuneApplySeq;
    tuningRte.unlock();

    if (!persisted) {
        DBG_PRINTF("[TUNE] Applied seq=%lu but NVS save verification FAILED.\n",
                   (unsigned long)appliedSeq);
        calLogf("[TUNE][ERROR] Applied seq=%lu but permanent save failed.",
                (unsigned long)appliedSeq);
        return false;
    }
    DBG_PRINTF("[TUNE] Applied and saved to NVS seq=%lu.\n", (unsigned long)appliedSeq);
    calLogf("[TUNE] Applied and saved permanently seq=%lu.", (unsigned long)appliedSeq);
    return true;
}

// ─────────────────────────────────────────────────────────────
