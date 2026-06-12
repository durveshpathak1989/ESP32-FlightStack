/*
  Timing instrumentation patch for RC_FlightController.ino

  1) Insert BLOCK A near your globals, for example after:
       static volatile bool g_pidTrace = false;

  2) Replace your existing taskControl() with BLOCK B.

  What it prints at ~1 Hz:
    [TIME] period=2500us jitter=+0us ctrl=...us full=...us headroom=...us maxCtrl=...us maxFull=...us overCtrl=... overFull=...

  Meaning:
    period   = loop start-to-start time
    jitter   = period - target period
    ctrl     = time from control work start to writeMotors()/motorsOff() done
    full     = time from control work start to end of logging/state publish
    headroom = target period - full execution time
*/

// ============================================================================
// BLOCK A — Add near global variables
// ============================================================================

struct ExecTimingStats {
    uint32_t lastControlUs;
    uint32_t lastFullUs;
    uint32_t maxControlUs;
    uint32_t maxFullUs;
    uint32_t controlOverruns;
    uint32_t fullOverruns;
};

static volatile ExecTimingStats g_execTiming = {0, 0, 0, 0, 0, 0};

static void updateExecTimingAndPrint(uint32_t controlUs,
                                     uint32_t fullUs,
                                     uint32_t periodUs,
                                     uint32_t targetUs)
{
    g_execTiming.lastControlUs = controlUs;
    g_execTiming.lastFullUs    = fullUs;

    if (controlUs > g_execTiming.maxControlUs) {
        g_execTiming.maxControlUs = controlUs;
    }

    if (fullUs > g_execTiming.maxFullUs) {
        g_execTiming.maxFullUs = fullUs;
    }

    if (controlUs > targetUs) {
        g_execTiming.controlOverruns++;
    }

    if (fullUs > targetUs) {
        g_execTiming.fullOverruns++;
    }

    // Print only once per second so Serial does not dominate the control loop.
    static uint32_t lastPrintMs = 0;
    uint32_t nowMs = millis();

    if (nowMs - lastPrintMs >= 1000) {
        lastPrintMs = nowMs;

        int32_t jitterSigned  = (int32_t)periodUs - (int32_t)targetUs;
        int32_t headroomFull  = (int32_t)targetUs - (int32_t)fullUs;

        Serial.printf("[TIME] period=%luus jitter=%+ldus ctrl=%luus full=%luus "
                      "headroom=%+ldus maxCtrl=%luus maxFull=%luus "
                      "overCtrl=%lu overFull=%lu\n",
                      (unsigned long)periodUs,
                      (long)jitterSigned,
                      (unsigned long)controlUs,
                      (unsigned long)fullUs,
                      (long)headroomFull,
                      (unsigned long)g_execTiming.maxControlUs,
                      (unsigned long)g_execTiming.maxFullUs,
                      (unsigned long)g_execTiming.controlOverruns,
                      (unsigned long)g_execTiming.fullOverruns);
    }
}


// ============================================================================
// BLOCK B — Replace your taskControl() with this version
// ============================================================================

static void taskControl(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(3);
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastUs = micros();
    const uint32_t TARGET_US = TIMING_TARGET_US;

    pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
    pidAngleRoll.reset(); pidAnglePitch.reset();
    pidAngleYaw.reset();
    g_yawHoldActive = false;

    for (;;) {
        if (g_tuning.dirty) applyTuningToObjects();

        if (g_calibState == CalibState::REQUESTED) {
            motorsOff();
            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            pidAngleYaw.reset();
            g_yawHoldActive = false;
            runAutonomousCalibration();
            g_calibState = CalibState::IDLE;
            lastUs   = micros();
            lastWake = xTaskGetTickCount();
            continue;
        }

        vTaskDelay(1);   // yield — allows WiFi, BMP, GPS, Serial to run
        while ((micros() - lastUs) < TARGET_US) { }   // busy-wait remainder

        uint32_t nowUs    = micros();
        uint32_t periodUs = nowUs - lastUs;
        lastUs = nowUs;
        lastWake = xTaskGetTickCount();

        // Execution timer starts AFTER the period wait.
        // This measures the actual control-cycle work, not the full start-to-start period.
        uint32_t execStartUs = micros();

        float dt = (float)periodUs * 1e-6f;
        if (dt <= 0.0f || dt > 0.05f) {
            dt = (float)TARGET_US * 1e-6f;
            periodUs = TARGET_US;
        }

        // ── Welford timing accumulator (Test 7.1) ────────────
        if (xSemaphoreTake(g_timingMutex, 0) == pdTRUE) {
            uint32_t n = ++g_timing.count;
            double delta  = (double)periodUs - g_timing.wMean;
            g_timing.wMean += delta / n;
            g_timing.wM2   += delta * ((double)periodUs - g_timing.wMean);

            uint32_t jit = (periodUs >= TARGET_US)
                         ? (periodUs - TARGET_US)
                         : (TARGET_US - periodUs);

            double jdelta  = (double)jit - g_timing.jMean;
            g_timing.jMean += jdelta / n;
            g_timing.jM2   += jdelta * ((double)jit - g_timing.jMean);

            if (jit > g_timing.jitterMax)  g_timing.jitterMax = jit;
            if (jit > JITTER_VIOLATION_US) g_timing.jitterViolations++;

            g_timing.buf[g_timing.bufHead] = (uint16_t)min(periodUs, (uint32_t)65535);
            g_timing.bufHead = (g_timing.bufHead + 1) % TIMING_BUF_SIZE;
            if (g_timing.bufHead == 0) g_timing.bufFull = true;

            xSemaphoreGive(g_timingMutex);
        }

        // ── IMU read + Mahony AHRS ────────────────────────────
        MPU_SensorData s;
        MPU_Attitude   att;
        bool imuOk = imu.readScaled(s);
        float gxf = 0, gyf = 0, gzf = 0;
        if (imuOk) {
            imu.mahonyUpdate(s, dt, att);
            gxf = lpfGx.apply(s.gx_dps, dt, GYRO_LPF_HZ);
            gyf = lpfGy.apply(s.gy_dps, dt, GYRO_LPF_HZ);
            gzf = lpfGz.apply(s.gz_dps, dt, GYRO_LPF_HZ);
        }

        RCCommand cmd = rcReceiver.getCommand();

        // ── DISARMED / FAILSAFE ───────────────────────────────
        if (cmd.mode == FlightMode::DISARMED || cmd.mode == FlightMode::FAILSAFE) {
            motorsOff();
            uint32_t controlDoneUs = micros();

            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            lpfGx.reset(); lpfGy.reset(); lpfGz.reset();
            lpfSpRoll.reset(); lpfSpPitch.reset(); lpfSpYaw.reset();
            pidAngleYaw.reset();
            g_yawHoldActive = false;

            if (xSemaphoreTake(g_flightMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
                g_state.pidRollOut = g_state.pidPitchOut = g_state.pidYawOut = 0;
                g_state.rc      = cmd;
                if (imuOk) {
                    g_state.roll_deg = att.roll;  g_state.pitch_deg = att.pitch;
                    g_state.yaw_deg  = att.yaw;
                    g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                    g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                    g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                    g_state.loopCount++;
                }
                xSemaphoreGive(g_flightMutex);
            }

            uint32_t fullDoneUs = micros();
            updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                     fullDoneUs - execStartUs,
                                     periodUs,
                                     TARGET_US);
            continue;
        }

        // ── ARMED — cascaded PID ──────────────────────────────
        float roll  = imuOk ? att.roll  : 0.0f;
        float pitch = imuOk ? att.pitch : 0.0f;
        float gx    = imuOk ? gxf : 0.0f;
        float gy    = imuOk ? gyf : 0.0f;
        float gz    = imuOk ? gzf : 0.0f;

        TuningState tune;
        if (xSemaphoreTake(g_tuneMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
            tune = g_tuning;
            xSemaphoreGive(g_tuneMutex);
        } else {
            // Extremely rare: keep flying with compile-time safe defaults for this cycle.
            memset(&tune, 0, sizeof(tune));
            tune.max_angle_deg              = TUNE_MAX_ANGLE_DEG;
            tune.max_rate_dps               = TUNE_MAX_RATE_DPS;
            tune.max_pitch_rate_dps         = TUNE_MAX_RATE_DPS;
            tune.roll_output_limit          = TUNE_ROLL_OUTPUT_LIMIT;
            tune.pitch_output_limit         = TUNE_PITCH_OUTPUT_LIMIT;
            tune.yaw_output_limit           = TUNE_YAW_OUTPUT_LIMIT;
            tune.throttle_expo              = TUNE_THROTTLE_EXPO;
            tune.throttle_up_rate_per_sec   = TUNE_THROTTLE_UP_RATE_PER_SEC;
            tune.throttle_down_rate_per_sec = TUNE_THROTTLE_DOWN_RATE_PER_SEC;
            tune.motor_idle                 = TUNE_MOTOR_IDLE;
            tune.motor_max                  = TUNE_MOTOR_MAX;
            tune.throttle_cut               = TUNE_THROTTLE_CUT;
            tune.idle_ramp_end              = TUNE_IDLE_RAMP_END;
            tune.yaw_deadband               = TUNE_YAW_DEADBAND;
            tune.yaw_max_rate_dps           = TUNE_YAW_MAX_RATE_DPS;
        }

        const float MAX_ANGLE_DEG      = tune.max_angle_deg;
        const float MAX_RATE_DPS       = tune.max_rate_dps;
        const float MAX_PITCH_RATE_DPS = tune.max_pitch_rate_dps;
        float rO=0, pO=0, yO=0;

        float rollCmd  = lpfSpRoll .apply(cmd.roll,  dt, RC_LPF_HZ);
        float pitchCmd = lpfSpPitch.apply(cmd.pitch, dt, RC_LPF_HZ);
        float yawCmd   = lpfSpYaw  .apply(cmd.yaw,   dt, RC_LPF_HZ);

        if (cmd.mode == FlightMode::ANGLE) {
            float rSP = pidAngleRoll .update(rollCmd *MAX_ANGLE_DEG - roll,  dt);
            float pSP = pidAnglePitch.update(pitchCmd*MAX_ANGLE_DEG - pitch, dt);
            rO = pidRateRoll .update(rSP - gx, dt);
            pO = pidRatePitch.update(pSP - gy, dt);
        } else {   // ACRO
            rO = pidRateRoll .update(rollCmd *MAX_RATE_DPS - gx, dt);
            pO = pidRatePitch.update(pitchCmd*MAX_PITCH_RATE_DPS - gy, dt);
        }

        // ── Yaw: heading-hold when stick centered, rate when moving ──
        if (imuOk && fabsf(yawCmd) < tune.yaw_deadband) {
            if (!g_yawHoldActive) {
                g_yawSetpoint   = att.yaw;
                g_yawHoldActive = true;
                pidAngleYaw.reset();
            }
            float yawErr = g_yawSetpoint - att.yaw;
            while (yawErr >  180.0f) yawErr -= 360.0f;
            while (yawErr < -180.0f) yawErr += 360.0f;
            float yawRateSP = pidAngleYaw.update(yawErr, dt);
            yawRateSP = constrain(yawRateSP, -tune.yaw_max_rate_dps, tune.yaw_max_rate_dps);
            yO = pidRateYaw.update(yawRateSP - gz, dt);
        } else {
            g_yawHoldActive = false;
            yO = pidRateYaw.update(-yawCmd*tune.yaw_max_rate_dps - gz, dt);
        }

        rO = constrain(rO, -tune.roll_output_limit,  tune.roll_output_limit);
        pO = constrain(pO, -tune.pitch_output_limit, tune.pitch_output_limit);
        yO = constrain(yO, -tune.yaw_output_limit,   tune.yaw_output_limit);

        // ── Throttle expo + smoothing ─────────────────────────
        static float thrSmooth = 0.0f;

        const float THROTTLE_EXPO              = tune.throttle_expo;
        const float THROTTLE_UP_RATE_PER_SEC   = tune.throttle_up_rate_per_sec;
        const float THROTTLE_DOWN_RATE_PER_SEC = tune.throttle_down_rate_per_sec;
        const float MOTOR_IDLE                 = tune.motor_idle;
        const float MOTOR_MAX                  = tune.motor_max;
        const float THROTTLE_CUT               = tune.throttle_cut;
        const float IDLE_RAMP_END              = tune.idle_ramp_end;

        float thrRaw = constrain(cmd.throttle, 0.0f, 1.0f);

        float thrTarget = 0.0f;
        if (thrRaw > THROTTLE_CUT) {
            thrTarget = throttleExpo(thrRaw, THROTTLE_EXPO);
        }

        float maxStepUp   = THROTTLE_UP_RATE_PER_SEC * dt;
        float maxStepDown = THROTTLE_DOWN_RATE_PER_SEC * dt;

        if (thrTarget > thrSmooth) {
            thrSmooth += min(thrTarget - thrSmooth, maxStepUp);
        } else {
            thrSmooth -= min(thrSmooth - thrTarget, maxStepDown);
        }

        float thr = constrain(thrSmooth, 0.0f, 1.0f);

        float fl = thr + rO - pO - yO;
        float fr = thr - rO - pO + yO;
        float rl = thr + rO + pO + yO;
        float rr = thr - rO + pO - yO;

        // Desaturate high side first
        float maxMotor = max(max(fl, fr), max(rl, rr));
        if (maxMotor > MOTOR_MAX) {
            float excess = maxMotor - MOTOR_MAX;
            fl -= excess; fr -= excess; rl -= excess; rr -= excess;
        }

        float idleBlend = smoothStep01((thr - THROTTLE_CUT) / (IDLE_RAMP_END - THROTTLE_CUT));
        float motorMin = MOTOR_IDLE * idleBlend;

        if (thr > THROTTLE_CUT) {
            fl = constrain(fl, motorMin, MOTOR_MAX);
            fr = constrain(fr, motorMin, MOTOR_MAX);
            rl = constrain(rl, motorMin, MOTOR_MAX);
            rr = constrain(rr, motorMin, MOTOR_MAX);
        } else {
            fl = fr = rl = rr = 0.0f;
        }

        writeMotors(fl, fr, rl, rr);
        uint32_t controlDoneUs = micros();

        // ── High-speed flight log @ 100 Hz (every 4th cycle) ──
        static uint8_t logDiv = 0;
        if (++logDiv >= 4) {
            logDiv = 0;
            FlightLogRow row;
            row.t_us      = nowUs;
            row.period_us = (uint16_t)min(periodUs, (uint32_t)65535);
            row.mode      = (uint8_t)cmd.mode;
            row.flags     = 0x01 | (imuOk ? 0x02 : 0x00);   // armed here by definition
            row.rcThrottle = thrRaw;
            row.thr        = thr;
            row.rcRoll = rollCmd; row.rcPitch = pitchCmd; row.rcYaw = yawCmd;
            row.roll = roll; row.pitch = pitch; row.yaw = imuOk ? att.yaw : 0.0f;
            row.gxRaw = imuOk ? s.gx_dps : 0.0f;
            row.gyRaw = imuOk ? s.gy_dps : 0.0f;
            row.gzRaw = imuOk ? s.gz_dps : 0.0f;
            row.gxFilt = gx; row.gyFilt = gy; row.gzFilt = gz;
            row.rO = rO; row.pO = pO; row.yO = yO;
            row.mFL = fl; row.mFR = fr; row.mRL = rl; row.mRR = rr;
            pushFlightLog(row);
        }

        // ── Publish flight state (incl. true PID outputs) ─────
        if (xSemaphoreTake(g_flightMutex, 0) == pdTRUE){
            if (imuOk) {
                g_state.roll_deg = att.roll;  g_state.pitch_deg = att.pitch;
                g_state.yaw_deg  = att.yaw;
                g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                g_state.loopCount++;
            }
            g_state.motorFL=fl; g_state.motorFR=fr;
            g_state.motorRL=rl; g_state.motorRR=rr;
            g_state.pidRollOut=rO; g_state.pidPitchOut=pO; g_state.pidYawOut=yO;
            g_state.armed=true; g_state.rc=cmd;
            xSemaphoreGive(g_flightMutex);
        }

        uint32_t fullDoneUs = micros();
        updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                 fullDoneUs - execStartUs,
                                 periodUs,
                                 TARGET_US);
    }
}
