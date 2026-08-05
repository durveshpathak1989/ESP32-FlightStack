//  taskControl — Core 1, priority 5, 400 Hz  (2.5 ms period)
// ─────────────────────────────────────────────────────────────
// ============================================================================
// BLOCK B — Replace your taskControl() with this version
// ============================================================================

static FlightControlSwc flightControlSwc;

void FlightControlSwc::Init()
{
    pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
    pidAngleRoll.reset(); pidAnglePitch.reset();
    pidAngleYaw.reset();
    attitudeEstimator.resetEkf();
    flightController.reset();
}

void FlightControlSwc::Periodic()
{
        const uint32_t TARGET_US = TIMING_TARGET_US;
        static uint32_t lastUs = 0;
        if (g_tuning.dirty) applyTuningToObjects();

        if (releases_ > 1) {
            g_execTiming.missedTimerReleases += (releases_ - 1);
        }

        uint32_t nowUs = clockServiceAdapter.microseconds();
        uint32_t periodUs = (lastUs == 0) ? TARGET_US : (nowUs - lastUs);
        lastUs = nowUs;

        // Execution timer starts AFTER the period wait.
        // This measures actual control work, not intentional wait time.
        uint32_t execStartUs = clockServiceAdapter.microseconds();

        float dt = (float)periodUs * 1e-6f;
        if (dt <= 0.0f || dt > 0.05f) {
            dt = (float)TARGET_US * 1e-6f;
            periodUs = TARGET_US;
        }

        const rte::BatteryServiceSample bat =
            batteryServiceAdapter.ReadStatus(clockServiceAdapter.milliseconds(), false);
        const float batteryVoltageForRpm = bat.valid ? bat.batteryVoltage_v : BATTERY_NOMINAL_V;

        // ── Welford timing accumulator (Test 7.1) ────────────
        if (timingRte.lock(0)) {
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

            timingRte.unlock();
        }

        // ── Calibration active: keep motors off and skip flight control ──
        const rte::CalibrationServiceStatus calibrationStatus =
            calibrationServiceAdapter.Status();
        if (FlightSafetyPolicy::controlMustStop(
                false, false, calibrationStatus.blocksFlight)) {
            if (!calibrationStatus.ownsMotors && g_motorOutputsActive) {
                motorsOff();
                g_motorOutputsActive = false;
            }

            pidRateRoll.reset();
            pidRatePitch.reset();
            pidRateYaw.reset();

            pidAngleRoll.reset();
            pidAnglePitch.reset();
            pidAngleYaw.reset();

            flightController.reset();

            updateExecTimingAndPrint(0, 0, periodUs, TARGET_US);

            return;
        }

        // ── IMU read + EKF AHRS ───────────────────────────────
        rte::ImuServiceSample s{};
        AttitudeEstimate   att;
        uint32_t phaseStartUs = clockServiceAdapter.microseconds();
        bool imuOk = imuServiceAdapter.ReadSample(s);
        g_execTiming.lastImuUs = clockServiceAdapter.microseconds() - phaseStartUs;

        float gxf = 0, gyf = 0, gzf = 0;
        float filtAx = 0.0f, filtAy = 0.0f, filtAz = 0.0f;
        float accelRollDeg = 0.0f, accelPitchDeg = 0.0f;
        float rollAngleErrDeg = 0.0f, pitchAngleErrDeg = 0.0f;
        if (imuOk) {
            if (g_notchResetPending) {
                notchAx.reset(s.ax_g);
                notchAy.reset(s.ay_g);
                notchAz.reset(s.az_g);
                notchGx.reset(s.gx_dps);
                notchGy.reset(s.gy_dps);
                notchGz.reset(s.gz_dps);
                g_notchResetPending = false;
            }

            // Apply tunable motor-vibration notch before EKF and before PID.
            // Keep original `s` for raw telemetry/logging; use `sf` for control.
            rte::ImuServiceSample sf = s;
            sf.ax_g   = notchAx.apply(s.ax_g);
            sf.ay_g   = notchAy.apply(s.ay_g);
            sf.az_g   = notchAz.apply(s.az_g);
            sf.gx_dps = notchGx.apply(s.gx_dps);
            sf.gy_dps = notchGy.apply(s.gy_dps);
            sf.gz_dps = notchGz.apply(s.gz_dps);
            filtAx = sf.ax_g; filtAy = sf.ay_g; filtAz = sf.az_g;

            // Feed onboard vibration spectrum with RAW pre-notch IMU data.
            // This lets you see the vibration peak even when the notch is enabled.
            spectrumAnalyzer.push(s.ax_g, s.ay_g, s.az_g,
                                  s.gx_dps, s.gy_dps, s.gz_dps,
                                  g_motorOutputsActive);

            if (LOG_DYNAMIC_NOTCH_DEBUG) {
                static uint32_t dynFftPushCount = 0;
                static uint32_t lastDynFftInputPrintMs = 0;

                dynFftPushCount++;

                uint32_t nowFftInputMs = clockServiceAdapter.milliseconds();
                if (nowFftInputMs - lastDynFftInputPrintMs >= 1000) {
                    lastDynFftInputPrintMs = nowFftInputMs;

                    float gyroMag =
                        sqrtf(s.gx_dps * s.gx_dps +
                              s.gy_dps * s.gy_dps +
                              s.gz_dps * s.gz_dps);

                    float accelVib =
                        fabsf(sqrtf(s.ax_g * s.ax_g +
                                    s.ay_g * s.ay_g +
                                    s.az_g * s.az_g) - 1.0f);

                    DBG_PRINTF("[DYN_FFT_INPUT] pushes=%lu motorOut=%d gyroMag=%.3f dps accelVib=%.4f g\n",
                                  (unsigned long)dynFftPushCount,
                                  g_motorOutputsActive ? 1 : 0,
                                  gyroMag,
                                  accelVib);
                    DBG_PRINTF("[DYN_NOTCH_HOOK] motorOut=%d notchEnable=%d notchHz=%.1f Q=%.1f\n",
                                  g_motorOutputsActive ? 1 : 0,
                                  g_tuning.notch_enable ? 1 : 0,
                                  g_tuning.notch_freq_hz,
                                  g_tuning.notch_q);
                }
            }
            AHRSInput ekfIn;
            ekfIn.ax_g = sf.ax_g; ekfIn.ay_g = sf.ay_g; ekfIn.az_g = sf.az_g;
            ekfIn.gx_dps = sf.gx_dps; ekfIn.gy_dps = sf.gy_dps; ekfIn.gz_dps = sf.gz_dps;
            ekfIn.mx_uT = s.mx_uT; ekfIn.my_uT = s.my_uT; ekfIn.mz_uT = s.mz_uT;
            ekfIn.magValid = s.magneticFieldValid;

            // Runtime-selectable attitude estimator SWC.
            uint8_t ahrsMode = 0;
            if (tuningRte.lock(0)) {
                ahrsMode = (uint8_t)constrain((int)roundf(g_tuning.ahrs_filter_mode), 0, 2);
                tuningRte.unlock();
            }
            uint32_t ahrsStartUs = clockServiceAdapter.microseconds();
            estimatorInputPort.send({ekfIn, dt, ahrsMode}, nowUs);
            attitudeEstimator.Periodic();
            rte::SignalSample<AttitudeEstimate> attitudeSignal{};
            estimatorOutputPort.receive(attitudeSignal);
            att = attitudeSignal.value;
            g_ahrsFilterModeActive = attitudeEstimator.activeMode();
            g_execTiming.lastAhrsUs = clockServiceAdapter.microseconds() - ahrsStartUs;

            gxf = lpfGx.update(sf.gx_dps, dt, GYRO_LPF_HZ);
            gyf = lpfGy.update(sf.gy_dps, dt, GYRO_LPF_HZ);
            gzf = lpfGz.update(sf.gz_dps, dt, GYRO_LPF_HZ);

            // Accel-only tilt estimate. This is noisy under vibration/acceleration,
            // but useful as an independent reference for AHRS roll/pitch.
            const float RAD_TO_DEG_LOCAL = 57.29577951308232f;
            accelRollDeg  = atan2f(s.ay_g, s.az_g) * RAD_TO_DEG_LOCAL;
            accelPitchDeg = atan2f(-s.ax_g, sqrtf(s.ay_g*s.ay_g + s.az_g*s.az_g)) * RAD_TO_DEG_LOCAL;

            // Gyro-only integration. This will drift; that drift is exactly what
            // the AHRS correction is meant to control.
            if (!g_gyroAngleInit) {
                g_gyroRollDeg  = att.roll_deg;
                g_gyroPitchDeg = att.pitch_deg;
                g_gyroYawDeg   = att.yaw_deg;
                g_gyroAngleInit = true;
            } else {
                g_gyroRollDeg  += gxf * dt;
                g_gyroPitchDeg += gyf * dt;
                g_gyroYawDeg   += gzf * dt;
                while (g_gyroYawDeg >= 360.0f) g_gyroYawDeg -= 360.0f;
                while (g_gyroYawDeg <    0.0f) g_gyroYawDeg += 360.0f;
            }

            rollAngleErrDeg  = att.roll_deg  - accelRollDeg;
            pitchAngleErrDeg = att.pitch_deg - accelPitchDeg;
        }

        phaseStartUs = clockServiceAdapter.microseconds();
        rte::SignalSample<flight::ReceiverFrame> receiverSignal{};
        receiverFramePort.receive(receiverSignal);
        flight::ReceiverFrame receiverFrame = receiverSignal.value;
        if (receiverSignal.validity != rte::DataValidity::Valid) {
            receiverFrame.command.mode = flight::FlightMode::Failsafe;
            receiverFrame.command.valid = false;
        }
        const flight::PilotCommand& cmd = receiverFrame.command;
        updateControlTransitionCounters(cmd);
        g_execTiming.lastRcUs = clockServiceAdapter.microseconds() - phaseStartUs;

        float rollCtrlDeg = 0.0f, pitchCtrlDeg = 0.0f, yawCtrlDeg = 0.0f;
        if (imuOk) computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);

        // Level-zero capture: DISARMED + SWB/ACRO high.
        // Uses raw CH8 because cmd.mode is DISARMED while disarmed, even if SWB is high.
        const bool swbAcroHigh = (cmd.valid && cmd.raw[RC_CH_AUX2] >= LEVEL_ZERO_SWB_THRESHOLD);
        const bool canCaptureLevelZero = (cmd.mode == flight::FlightMode::Disarmed && imuOk && swbAcroHigh);
        levelTrimInputPort.send(
            {swbAcroHigh, canCaptureLevelZero, clockServiceAdapter.milliseconds(), LEVEL_ZERO_SAMPLE_MS,
             att.roll_deg, att.pitch_deg, att.yaw_deg},
            nowUs);
        levelTrim.Periodic();
        rte::SignalSample<LevelTrimOutput> levelSignal{};
        levelTrimOutputPort.receive(levelSignal);
        const LevelTrimUpdate& levelEvent = levelSignal.value.event;
        if (levelEvent.captureStarted) {
            DBG_PRINTLN(F("[LEVEL] Capturing software zero — keep drone still..."));
            calLog("[LEVEL] Capturing software zero — keep drone still...");
        }
        if (levelEvent.captureCompleted) {
            computeControlAttitude(att, rollCtrlDeg, pitchCtrlDeg, yawCtrlDeg);
            DBG_PRINTF("[LEVEL] Zero saved: rollOff=%+.2f pitchOff=%+.2f yawOff=%+.2f deg\n",
                       levelTrim.rollOffsetDeg(), levelTrim.pitchOffsetDeg(),
                       levelTrim.yawOffsetDeg());
            calLogf("[LEVEL] Zero saved: R=%+.2f P=%+.2f Y=%+.2f deg",
                    levelTrim.rollOffsetDeg(), levelTrim.pitchOffsetDeg(),
                    levelTrim.yawOffsetDeg());
        }

        // ── DISARMED / FAILSAFE ───────────────────────────────
        if (FlightSafetyPolicy::controlMustStop(
                cmd.mode == flight::FlightMode::Disarmed,
                cmd.mode == flight::FlightMode::Failsafe,
                false)) {
            phaseStartUs = clockServiceAdapter.microseconds();
            if (g_motorOutputsActive) {
                // One-shot stop only. Repeating motorOff() at 400 Hz was measured
                // as ~40 ms in your log, which destroys the loop timing.
                motorsOff();
                g_motorOutputsActive = false;
            }
            g_execTiming.lastMotorUs = clockServiceAdapter.microseconds() - phaseStartUs;
            uint32_t controlDoneUs = clockServiceAdapter.microseconds();

            pidRateRoll.reset();  pidRatePitch.reset();  pidRateYaw.reset();
            pidAngleRoll.reset(); pidAnglePitch.reset();
            lpfGx.reset(); lpfGy.reset(); lpfGz.reset();
            notchAx.reset(); notchAy.reset(); notchAz.reset();
            notchGx.reset(); notchGy.reset(); notchGz.reset();
            pidAngleYaw.reset();
            flightController.reset();

            phaseStartUs = clockServiceAdapter.microseconds();
            if (flightStateRte.lock(0)) {
                g_state.armed   = false;
                g_state.motorFL = g_state.motorFR = g_state.motorRL = g_state.motorRR = 0;
                g_state.motorFLPreSat = g_state.motorFRPreSat = g_state.motorRLPreSat = g_state.motorRRPreSat = 0;
                g_state.batteryVoltage_v = batteryVoltageForRpm;
                g_state.batteryAdcVoltage_v = bat.adcVoltage_v;
                g_state.batteryCellVoltage_v = bat.cellVoltage_v;
                g_state.batteryPercent = bat.percent;
                g_state.batteryValid = bat.valid;
                g_state.batteryLow = bat.low;
                g_state.batteryCritical = bat.critical;
                g_state.pidRollOut = g_state.pidPitchOut = g_state.pidYawOut = 0;
                g_state.loopPeriod_us = periodUs;
                g_state.loopJitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
                g_state.imuRead_us = g_execTiming.lastImuUs;
                g_state.rcRead_us = g_execTiming.lastRcUs;
                g_state.ahrsUpdate_us = g_execTiming.lastAhrsUs;
                g_state.controlUpdate_us = controlDoneUs - execStartUs;
                g_state.motorWrite_us = g_execTiming.lastMotorUs;
                g_state.wifiService_us = g_lastWifiServiceUs;
                g_state.onboardLogWrite_us = g_execTiming.lastLogUs;
                g_state.missedLoopCount = g_execTiming.missedTimerReleases;
                g_state.rc      = cmd;
                g_state.angleModeActive = false;
                g_state.acroModeActive = false;
                g_state.modeSwitchRaw_us = cmd.raw[RC_CH_AUX2];
                g_state.armSwitchRaw_us = cmd.raw[RC_CH_FLIGHTMODE];
                g_state.auxTune1Raw_us = cmd.raw[RC_CH_AUX1];
                g_state.auxTune2Raw_us = cmd.raw[RC_CH_AUX5];
                g_state.rcFailsafeCount = receiverFrame.failsafeCount;
                g_state.rcFrameRateHz = receiverFrame.frameRateHz;
                g_state.angleLoopEnabled = false;
                g_state.rateLoopEnabled = false;
                g_state.actualRollRate_dps = imuOk ? gxf : 0.0f;
                g_state.actualPitchRate_dps = imuOk ? gyf : 0.0f;
                g_state.actualYawRate_dps = imuOk ? gzf : 0.0f;
                g_state.angleRollP = pidAngleRoll.lastP;
                g_state.angleRollI = pidAngleRoll.lastI;
                g_state.angleRollD = pidAngleRoll.lastD;
                g_state.anglePitchP = pidAnglePitch.lastP;
                g_state.anglePitchI = pidAnglePitch.lastI;
                g_state.anglePitchD = pidAnglePitch.lastD;
                g_state.rateRollP = pidRateRoll.lastP;
                g_state.rateRollI = pidRateRoll.lastI;
                g_state.rateRollD = pidRateRoll.lastD;
                g_state.ratePitchP = pidRatePitch.lastP;
                g_state.ratePitchI = pidRatePitch.lastI;
                g_state.ratePitchD = pidRatePitch.lastD;
                g_state.rateYawP = pidRateYaw.lastP;
                g_state.rateYawI = pidRateYaw.lastI;
                g_state.rateYawD = pidRateYaw.lastD;
                g_state.angleRollIterm = pidAngleRoll.integral;
                g_state.anglePitchIterm = pidAnglePitch.integral;
                g_state.pidResetCount = g_pidResetCount;
                g_state.modeTransitionCount = g_modeTransitionCount;
                g_state.lastModeChange_ms = g_lastModeChangeMs;
                g_state.armingTransitionCount = g_armingTransitionCount;
                g_state.lastArmChange_ms = g_lastArmChangeMs;
                g_state.throttleLow = true;
                g_state.controlAuthorityRemaining = 0.0f;
                g_state.rollOutputLimited = false;
                g_state.pitchOutputLimited = false;
                g_state.yawOutputLimited = false;
                g_state.rateOutputLimited = false;
                g_state.cmdRpmFL = g_state.cmdRpmFR = g_state.cmdRpmRL = g_state.cmdRpmRR = 0.0f;
                g_state.actualRpmFL = g_state.actualRpmFR = g_state.actualRpmRL = g_state.actualRpmRR = 0.0f;
                g_state.rpmActualValid = false;
                if (imuOk) {
                    g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                    g_state.yaw_deg  = att.yaw_deg;
                    g_state.q0 = att.q0; g_state.q1 = att.q1; g_state.q2 = att.q2; g_state.q3 = att.q3;
                    g_state.imuValid = imuOk;
                    g_state.magValid = s.magneticFieldValid;
                    g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                    g_state.roll_ctrl_deg = rollCtrlDeg;
                    g_state.pitch_ctrl_deg = pitchCtrlDeg;
                    g_state.yaw_ctrl_deg = yawCtrlDeg;
                    g_state.roll_offset_deg = levelTrim.rollOffsetDeg();
                    g_state.pitch_offset_deg = levelTrim.pitchOffsetDeg();
                    g_state.yaw_offset_deg = levelTrim.yawOffsetDeg();
                    g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                    g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                    g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                    g_state.imuTemp_c=s.temp_c;
                    g_state.accelRoll_deg = accelRollDeg;
                    g_state.accelPitch_deg = accelPitchDeg;
                    g_state.gyroRoll_deg = g_gyroRollDeg;
                    g_state.gyroPitch_deg = g_gyroPitchDeg;
                    g_state.gyroYaw_deg = g_gyroYawDeg;
                    g_state.rollAngleError_deg = rollAngleErrDeg;
                    g_state.pitchAngleError_deg = pitchAngleErrDeg;
                    g_state.loopCount++;
                }
                flightStateRte.unlock();
            }
            g_execTiming.lastStateUs = clockServiceAdapter.microseconds() - phaseStartUs;

            uint32_t fullDoneUs = clockServiceAdapter.microseconds();
            updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                     fullDoneUs - execStartUs,
                                     periodUs,
                                     TARGET_US);
            return;
        }

        // ── ARMED — cascaded PID ──────────────────────────────
        float roll  = imuOk ? rollCtrlDeg  : 0.0f;
        float pitch = imuOk ? pitchCtrlDeg : 0.0f;
        float gx    = imuOk ? gxf : 0.0f;
        float gy    = imuOk ? gyf : 0.0f;
        float gz    = imuOk ? gzf : 0.0f;

        TuningState tune;
        if (tuningRte.read(tune, 0)) {
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
            tune.pid_roll_ff                = TUNE_RATE_ROLL_FF;
            tune.pid_pitch_ff               = TUNE_RATE_PITCH_FF;
            tune.pid_yaw_ff                 = TUNE_RATE_YAW_FF;
            tune.pid_roll_d_lpf_hz          = TUNE_RATE_ROLL_D_LPF_HZ;
            tune.pid_pitch_d_lpf_hz         = TUNE_RATE_PITCH_D_LPF_HZ;
            tune.pid_yaw_d_lpf_hz           = TUNE_RATE_YAW_D_LPF_HZ;
            tune.ahrs_filter_mode           = TUNE_AHRS_FILTER_MODE;
            tune.madgwick_beta              = TUNE_MADGWICK_BETA;
            tune.notch_enable               = TUNE_NOTCH_ENABLE;
            tune.notch_freq_hz              = TUNE_NOTCH_FREQ_HZ;
            tune.notch_q                    = TUNE_NOTCH_Q;
            tune.ekf_angle_q                = TUNE_EKF_ANGLE_Q;
            tune.ekf_bias_q                 = TUNE_EKF_BIAS_Q;
            tune.ekf_accel_r                = TUNE_EKF_ACCEL_R;
            tune.ekf_mag_r                  = TUNE_EKF_MAG_R;
            tune.ekf_mag_declination_deg    = TUNE_EKF_MAG_DECL_DEG;
            tune.ekf_mag_yaw_offset_deg     = TUNE_EKF_MAG_YAW_OFF_DEG;
            tune.ekf_mag_yaw_sign           = TUNE_EKF_MAG_YAW_SIGN;
        }

        controllerConfigPort.send(tune, nowUs);
        controllerInputPort.send(
            {cmd.roll, cmd.pitch, cmd.yaw,
             roll, pitch, yawCtrlDeg,
             gx, gy, gz, dt, RC_LPF_HZ,
             cmd.mode == flight::FlightMode::Angle, imuOk},
            nowUs);
        flightController.Periodic();
        rte::SignalSample<CascadedControlOutput> controlSignal{};
        controllerOutputPort.receive(controlSignal);
        const CascadedControlOutput& control = controlSignal.value;
        const float rollCmd = control.filteredRollCommand;
        const float pitchCmd = control.filteredPitchCommand;
        const float yawCmd = control.filteredYawCommand;
        const float targetRollDeg = control.targetRollDeg;
        const float targetPitchDeg = control.targetPitchDeg;
        const float targetYawDeg = control.targetYawDeg;
        const float targetRollRateDps = control.targetRollRateDps;
        const float targetPitchRateDps = control.targetPitchRateDps;
        const float targetYawRateDps = control.targetYawRateDps;
        const float angleErrRollDeg = control.rollAngleErrorDeg;
        const float angleErrPitchDeg = control.pitchAngleErrorDeg;
        const float yawErrDeg = control.yawErrorDeg;
        const float rateErrRollDps = control.rollRateErrorDps;
        const float rateErrPitchDps = control.pitchRateErrorDps;
        const float rateErrYawDps = control.yawRateErrorDps;
        const float rO = control.rollCorrection;
        const float pO = control.pitchCorrection;
        const float yO = control.yawCorrection;
        const bool rollOutputLimited = control.rollLimited;
        const bool pitchOutputLimited = control.pitchLimited;
        const bool yawOutputLimited = control.yawLimited;
        const bool rateOutputLimited = control.anyRateOutputLimited;

        // The application mixer owns throttle shaping and the Quad-X equations.
        const float MOTOR_MAX                  = tune.motor_max;
        const float THROTTLE_CUT               = tune.throttle_cut;
        const float thrRaw                     = constrain(cmd.throttle, 0.0f, 1.0f);
        const MotorMixerConfig mixerConfig{
            tune.throttle_expo,
            tune.throttle_up_rate_per_sec,
            tune.throttle_down_rate_per_sec,
            tune.motor_idle,
            tune.motor_max,
            tune.throttle_cut,
            tune.idle_ramp_end
        };
        motorMixerConfigPort.send(mixerConfig, nowUs);
        motorMixerInputPort.send({cmd.throttle, rO, pO, yO, dt}, nowUs);
        motorMixer.Periodic();
        rte::SignalSample<MotorMixerOutput> mixedSignal{};
        motorMixerOutputPort.receive(mixedSignal);
        const MotorMixerOutput& mixed = mixedSignal.value;
        const float thr = mixed.shapedThrottle;
        const float flPre = mixed.frontLeftPreSaturation;
        const float frPre = mixed.frontRightPreSaturation;
        const float rlPre = mixed.rearLeftPreSaturation;
        const float rrPre = mixed.rearRightPreSaturation;
        float fl = mixed.frontLeft;
        float fr = mixed.frontRight;
        float rl = mixed.rearLeft;
        float rr = mixed.rearRight;
        const bool motorSaturated = mixed.saturated;
        const float maxMotor = mixed.maximumBeforeDesaturation;
        const float controlAuthorityRemaining = mixed.controlAuthorityRemaining;

        if (LOG_MOTOR_SATURATION && motorSaturated) {
            static uint32_t lastMotorSatPrintMs = 0;
            uint32_t motorSatNowMs = clockServiceAdapter.milliseconds();
            if (motorSatNowMs - lastMotorSatPrintMs >= 250) {
                lastMotorSatPrintMs = motorSatNowMs;
                DBG_PRINTF("[MOTOR_SAT] thr=%.3f rO=%.3f pO=%.3f yO=%.3f "
                              "maxBefore=%.3f MOTOR_MAX=%.3f "
                              "FL=%.3f FR=%.3f RL=%.3f RR=%.3f\n",
                              thr, rO, pO, yO,
                              maxMotor, MOTOR_MAX,
                              fl, fr, rl, rr);
            }
        }

        // ── Motor PWM update ───────────────────────────────────
        // Runs every 400 Hz control cycle so motor output tracks PID immediately.
        static uint32_t motorWriteCount = 0;
        static uint32_t lastMotorRatePrintMs = 0;

        phaseStartUs = clockServiceAdapter.microseconds();
        writeMotors(fl, fr, rl, rr);
        g_motorOutputsActive = true;
        motorWriteCount++;

        g_execTiming.lastMotorUs = clockServiceAdapter.microseconds() - phaseStartUs;

        if (LOG_MOTOR_WRITE_RATE) {
            uint32_t motorRateNowMs = clockServiceAdapter.milliseconds();
            if (motorRateNowMs - lastMotorRatePrintMs >= 1000) {
                lastMotorRatePrintMs = motorRateNowMs;

                DBG_PRINTF("[MOTOR_RATE] target=400Hz writes=%lu lastMotorUs=%lu\n",
                              (unsigned long)motorWriteCount,
                              (unsigned long)g_execTiming.lastMotorUs);

                motorWriteCount = 0;
            }
        } else {
            motorWriteCount = 0;
        }

        uint32_t controlDoneUs = clockServiceAdapter.microseconds();


        const float magNorm_uT = imuOk ? sqrtf(s.mx_uT*s.mx_uT + s.my_uT*s.my_uT + s.mz_uT*s.mz_uT) : 0.0f;
        const float rpmScaleLog = MOTOR_KV * batteryVoltageForRpm;
        const float cmdRpmFL = fl * rpmScaleLog;
        const float cmdRpmFR = fr * rpmScaleLog;
        const float cmdRpmRL = rl * rpmScaleLog;
        const float cmdRpmRR = rr * rpmScaleLog;

        // ── High-speed flight log @ 100 Hz (every 4th cycle) ──
        static uint8_t logDiv = 0;
        if (++logDiv >= 4) {
            logDiv = 0;
            LoggerRow row;
            memset(&row, 0, sizeof(row));
            row.t_us = nowUs;
            row.loop_count = g_state.loopCount;
            auto clampU16 = [](uint32_t v) -> uint16_t {
                return (uint16_t)((v > 65535UL) ? 65535UL : v);
            };
            row.period_us = clampU16(periodUs);
            row.jitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
            row.control_exec_us = clampU16(controlDoneUs - execStartUs);
            row.imu_read_us = clampU16(g_execTiming.lastImuUs);
            row.rc_read_us = clampU16(g_execTiming.lastRcUs);
            row.ahrs_exec_us = 0;
            row.pid_exec_us = 0;
            row.motor_exec_us = clampU16(g_execTiming.lastMotorUs);
            row.mode = (uint8_t)cmd.mode;
            row.flags = ((cmd.mode != flight::FlightMode::Disarmed) ? 0x0001 : 0)
                      | (imuOk ? 0x0002 : 0)
                      | (cmd.valid ? 0x0004 : 0)
                      | ((imuOk && s.magneticFieldValid) ? 0x0008 : 0)
                      | (motorSaturated ? 0x0010 : 0)
                      | (g_state.rpmActualValid ? 0x0020 : 0);

            row.throttle = cmd.throttle;
            row.rc_roll = rollCmd;
            row.rc_pitch = pitchCmd;
            row.rc_yaw = yawCmd;
            row.target_roll_deg = targetRollDeg;
            row.target_pitch_deg = targetPitchDeg;
            row.target_yaw_deg = targetYawDeg;
            row.target_roll_rate_dps = targetRollRateDps;
            row.target_pitch_rate_dps = targetPitchRateDps;
            row.target_yaw_rate_dps = targetYawRateDps;

            row.ekf_roll_deg = imuOk ? att.roll_deg : 0.0f;
            row.ekf_pitch_deg = imuOk ? att.pitch_deg : 0.0f;
            row.ekf_yaw_deg = imuOk ? att.yaw_deg : 0.0f;
            row.ctrl_roll_deg = roll;
            row.ctrl_pitch_deg = pitch;
            row.ctrl_yaw_deg = imuOk ? yawCtrlDeg : 0.0f;
            row.zero_roll_deg = levelTrim.rollOffsetDeg();
            row.zero_pitch_deg = levelTrim.pitchOffsetDeg();
            row.zero_yaw_deg = levelTrim.yawOffsetDeg();

            row.ax_g = imuOk ? s.ax_g : 0.0f;
            row.ay_g = imuOk ? s.ay_g : 0.0f;
            row.az_g = imuOk ? s.az_g : 0.0f;
            row.gx_dps = imuOk ? s.gx_dps : 0.0f;
            row.gy_dps = imuOk ? s.gy_dps : 0.0f;
            row.gz_dps = imuOk ? s.gz_dps : 0.0f;
            row.mx_uT = imuOk ? s.mx_uT : 0.0f;
            row.my_uT = imuOk ? s.my_uT : 0.0f;
            row.mz_uT = imuOk ? s.mz_uT : 0.0f;
            row.accel_norm_g = sqrtf(row.ax_g*row.ax_g + row.ay_g*row.ay_g + row.az_g*row.az_g);
            row.gyro_norm_dps = sqrtf(row.gx_dps*row.gx_dps + row.gy_dps*row.gy_dps + row.gz_dps*row.gz_dps);
            row.mag_norm_uT = sqrtf(row.mx_uT*row.mx_uT + row.my_uT*row.my_uT + row.mz_uT*row.mz_uT);
            row.ekf_bgx_dps = attitudeEstimator.ekfRollBiasDps();
            row.ekf_bgy_dps = attitudeEstimator.ekfPitchBiasDps();
            row.ekf_bgz_dps = attitudeEstimator.ekfYawBiasDps();
            row.ahrs_mode = g_ahrsFilterModeActive;
            row.ekf_mag_used = attitudeEstimator.ekfMagAccepted() ? 1 : 0;

            row.angle_roll_error_deg = angleErrRollDeg;
            row.angle_pitch_error_deg = angleErrPitchDeg;
            row.yaw_error_deg = yawErrDeg;
            row.rate_roll_error_dps = rateErrRollDps;
            row.rate_pitch_error_dps = rateErrPitchDps;
            row.rate_yaw_error_dps = rateErrYawDps;

            row.angle_roll_p = pidAngleRoll.lastP;
            row.angle_roll_i = pidAngleRoll.lastI;
            row.angle_roll_d = pidAngleRoll.lastD;
            row.angle_roll_out = targetRollRateDps;
            row.angle_pitch_p = pidAnglePitch.lastP;
            row.angle_pitch_i = pidAnglePitch.lastI;
            row.angle_pitch_d = pidAnglePitch.lastD;
            row.angle_pitch_out = targetPitchRateDps;
            row.angle_yaw_p = pidAngleYaw.lastP;
            row.angle_yaw_i = pidAngleYaw.lastI;
            row.angle_yaw_d = pidAngleYaw.lastD;
            row.angle_yaw_out = targetYawRateDps;

            row.rate_roll_p = pidRateRoll.lastP;
            row.rate_roll_i = pidRateRoll.lastI;
            row.rate_roll_d = pidRateRoll.lastD;
            row.rate_roll_out = rO;
            row.rate_pitch_p = pidRatePitch.lastP;
            row.rate_pitch_i = pidRatePitch.lastI;
            row.rate_pitch_d = pidRatePitch.lastD;
            row.rate_pitch_out = pO;
            row.rate_yaw_p = pidRateYaw.lastP;
            row.rate_yaw_i = pidRateYaw.lastI;
            row.rate_yaw_d = pidRateYaw.lastD;
            row.rate_yaw_out = yO;

            row.motor_fl_pre = flPre;
            row.motor_fr_pre = frPre;
            row.motor_rl_pre = rlPre;
            row.motor_rr_pre = rrPre;
            row.motor_fl = fl;
            row.motor_fr = fr;
            row.motor_rl = rl;
            row.motor_rr = rr;
            row.motor_diag_a = fl + rr;
            row.motor_diag_b = fr + rl;
            row.motor_diag_diff = row.motor_diag_a - row.motor_diag_b;

            row.rpm_fl = cmdRpmFL;
            row.rpm_fr = cmdRpmFR;
            row.rpm_rl = cmdRpmRL;
            row.rpm_rr = cmdRpmRR;
            row.rpm_diag_a = cmdRpmFL + cmdRpmRR;
            row.rpm_diag_b = cmdRpmFR + cmdRpmRL;
            row.rpm_diag_diff = row.rpm_diag_a - row.rpm_diag_b;

            row.battery_v = batteryVoltageForRpm;
            row.cpu0_pct = g_state.cpuCore0_pct;
            row.cpu1_pct = g_state.cpuCore1_pct;
            row.notch_freq_hz = tune.notch_freq_hz;
            row.notch_q = tune.notch_q;
            row.notch_enable = tune.notch_enable ? 1 : 0;
            row.bmp_alt_m = g_state.bmpAltitude_m;
            row.bmp_vz_mps = g_state.bmpVerticalSpeed_mps;
            row.gps_valid = g_state.gps.valid ? 1 : 0;
            row.gps_sats = g_state.gps.satellites;
            row.gps_hdop = g_state.gps.hdop;

            uint32_t logStartUs = clockServiceAdapter.microseconds();
            flightLogger.push(row);
            g_execTiming.lastLogUs = clockServiceAdapter.microseconds() - logStartUs;
        }

        // ── Publish flight state (incl. true PID outputs) ─────
        phaseStartUs = clockServiceAdapter.microseconds();
        if (flightStateRte.lock(0)){
            if (imuOk) {
                g_state.roll_deg = att.roll_deg;  g_state.pitch_deg = att.pitch_deg;
                g_state.yaw_deg  = att.yaw_deg;
                g_state.q0=att.q0; g_state.q1=att.q1; g_state.q2=att.q2; g_state.q3=att.q3;
                g_state.imuValid = imuOk;
                g_state.magValid = s.magneticFieldValid;
                g_state.ahrsFilterMode = g_ahrsFilterModeActive;
                g_state.roll_ctrl_deg = rollCtrlDeg;
                g_state.pitch_ctrl_deg = pitchCtrlDeg;
                g_state.yaw_ctrl_deg = yawCtrlDeg;
                g_state.roll_offset_deg = levelTrim.rollOffsetDeg();
                g_state.pitch_offset_deg = levelTrim.pitchOffsetDeg();
                g_state.yaw_offset_deg = levelTrim.yawOffsetDeg();
                g_state.ax_g=s.ax_g; g_state.ay_g=s.ay_g; g_state.az_g=s.az_g;
                g_state.gx_dps=s.gx_dps; g_state.gy_dps=s.gy_dps; g_state.gz_dps=s.gz_dps;
                g_state.mx_uT=s.mx_uT; g_state.my_uT=s.my_uT; g_state.mz_uT=s.mz_uT;
                g_state.imuTemp_c=s.temp_c;
                g_state.rawAx_g=s.ax_g; g_state.rawAy_g=s.ay_g; g_state.rawAz_g=s.az_g;
                g_state.rawGx_dps=s.gx_dps; g_state.rawGy_dps=s.gy_dps; g_state.rawGz_dps=s.gz_dps;
                g_state.filtAx_g=filtAx; g_state.filtAy_g=filtAy; g_state.filtAz_g=filtAz;
                g_state.filtGx_dps=gx; g_state.filtGy_dps=gy; g_state.filtGz_dps=gz;
                g_state.magNorm_uT=magNorm_uT;
                g_state.ekfMagUsed=attitudeEstimator.ekfMagAccepted();
                g_state.ekfBgx_dps=attitudeEstimator.ekfRollBiasDps();
                g_state.ekfBgy_dps=attitudeEstimator.ekfPitchBiasDps();
                g_state.ekfBgz_dps=attitudeEstimator.ekfYawBiasDps();
                g_state.targetRollDeg=targetRollDeg; g_state.targetPitchDeg=targetPitchDeg; g_state.targetYawDeg=targetYawDeg;
                g_state.targetRollRateDps=targetRollRateDps; g_state.targetPitchRateDps=targetPitchRateDps; g_state.targetYawRateDps=targetYawRateDps;
                g_state.rollRateError_dps=rateErrRollDps; g_state.pitchRateError_dps=rateErrPitchDps; g_state.yawRateError_dps=rateErrYawDps;
                g_state.yawError_deg=yawErrDeg;
                g_state.motorSaturated=motorSaturated;
                g_state.cmdRpmFL=cmdRpmFL; g_state.cmdRpmFR=cmdRpmFR; g_state.cmdRpmRL=cmdRpmRL; g_state.cmdRpmRR=cmdRpmRR;
                g_state.actualRpmFL=0.0f; g_state.actualRpmFR=0.0f; g_state.actualRpmRL=0.0f; g_state.actualRpmRR=0.0f; g_state.rpmActualValid=false;
                g_state.accelRoll_deg = accelRollDeg;
                g_state.accelPitch_deg = accelPitchDeg;
                g_state.gyroRoll_deg = g_gyroRollDeg;
                g_state.gyroPitch_deg = g_gyroPitchDeg;
                g_state.gyroYaw_deg = g_gyroYawDeg;
                g_state.rollAngleError_deg = rollAngleErrDeg;
                g_state.pitchAngleError_deg = pitchAngleErrDeg;
                g_state.loopCount++;
            }
            g_state.batteryVoltage_v = batteryVoltageForRpm;
            g_state.batteryAdcVoltage_v = bat.adcVoltage_v;
            g_state.batteryCellVoltage_v = bat.cellVoltage_v;
            g_state.batteryPercent = bat.percent;
            g_state.batteryValid = bat.valid;
            g_state.batteryLow = bat.low;
            g_state.batteryCritical = bat.critical;
            g_state.motorFL=fl; g_state.motorFR=fr;
            g_state.motorRL=rl; g_state.motorRR=rr;
            g_state.motorFLPreSat=flPre; g_state.motorFRPreSat=frPre;
            g_state.motorRLPreSat=rlPre; g_state.motorRRPreSat=rrPre;
            g_state.pidRollOut=rO; g_state.pidPitchOut=pO; g_state.pidYawOut=yO;
            g_state.loopPeriod_us = periodUs;
            g_state.loopJitter_us = (int16_t)constrain((int32_t)periodUs - (int32_t)TARGET_US, -32768, 32767);
            g_state.imuRead_us = g_execTiming.lastImuUs;
            g_state.rcRead_us = g_execTiming.lastRcUs;
            g_state.ahrsUpdate_us = g_execTiming.lastAhrsUs;
            g_state.controlUpdate_us = controlDoneUs - execStartUs;
            g_state.motorWrite_us = g_execTiming.lastMotorUs;
            g_state.wifiService_us = g_lastWifiServiceUs;
            g_state.onboardLogWrite_us = g_execTiming.lastLogUs;
            g_state.missedLoopCount = g_execTiming.missedTimerReleases;
            g_state.angleModeActive = (cmd.mode == flight::FlightMode::Angle);
            g_state.acroModeActive = (cmd.mode == flight::FlightMode::Acro);
            g_state.modeSwitchRaw_us = cmd.raw[RC_CH_AUX2];
            g_state.armSwitchRaw_us = cmd.raw[RC_CH_FLIGHTMODE];
            g_state.auxTune1Raw_us = cmd.raw[RC_CH_AUX1];
            g_state.auxTune2Raw_us = cmd.raw[RC_CH_AUX5];
            g_state.rcFailsafeCount = receiverFrame.failsafeCount;
            g_state.rcFrameRateHz = receiverFrame.frameRateHz;
            g_state.angleLoopEnabled = (cmd.mode == flight::FlightMode::Angle) && imuOk;
            g_state.rateLoopEnabled = imuOk && (cmd.mode == flight::FlightMode::Angle || cmd.mode == flight::FlightMode::Acro);
            g_state.actualRollRate_dps = gx;
            g_state.actualPitchRate_dps = gy;
            g_state.actualYawRate_dps = gz;
            g_state.angleRollP = pidAngleRoll.lastP;
            g_state.angleRollI = pidAngleRoll.lastI;
            g_state.angleRollD = pidAngleRoll.lastD;
            g_state.anglePitchP = pidAnglePitch.lastP;
            g_state.anglePitchI = pidAnglePitch.lastI;
            g_state.anglePitchD = pidAnglePitch.lastD;
            g_state.rateRollP = pidRateRoll.lastP;
            g_state.rateRollI = pidRateRoll.lastI;
            g_state.rateRollD = pidRateRoll.lastD;
            g_state.ratePitchP = pidRatePitch.lastP;
            g_state.ratePitchI = pidRatePitch.lastI;
            g_state.ratePitchD = pidRatePitch.lastD;
            g_state.rateYawP = pidRateYaw.lastP;
            g_state.rateYawI = pidRateYaw.lastI;
            g_state.rateYawD = pidRateYaw.lastD;
            g_state.angleRollIterm = pidAngleRoll.integral;
            g_state.anglePitchIterm = pidAnglePitch.integral;
            g_state.pidResetCount = g_pidResetCount;
            g_state.modeTransitionCount = g_modeTransitionCount;
            g_state.lastModeChange_ms = g_lastModeChangeMs;
            g_state.armingTransitionCount = g_armingTransitionCount;
            g_state.lastArmChange_ms = g_lastArmChangeMs;
            g_state.throttleLow = (thrRaw <= THROTTLE_CUT);
            g_state.controlAuthorityRemaining = controlAuthorityRemaining;
            g_state.rollOutputLimited = rollOutputLimited;
            g_state.pitchOutputLimited = pitchOutputLimited;
            g_state.yawOutputLimited = yawOutputLimited;
            g_state.rateOutputLimited = rateOutputLimited;
            g_state.armed=true; g_state.rc=cmd;
            flightStateRte.unlock();
        }
        g_execTiming.lastStateUs = clockServiceAdapter.microseconds() - phaseStartUs;

        uint32_t fullDoneUs = clockServiceAdapter.microseconds();
        updateExecTimingAndPrint(controlDoneUs - execStartUs,
                                 fullDoneUs - execStartUs,
                                 periodUs,
                                 TARGET_US);
}

static void taskControl(void* /*pv*/)
{
    flightControlSwc.Init();
    for (;;) {
        // FreeRTOS activation stays in the platform task adapter; the SWC
        // Periodic runnable contains no scheduler service call.
        const uint32_t releases = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        flightControlSwc.SetActivationCount(releases);
        flightControlSwc.Periodic();
    }
}


// ─────────────────────────────────────────────────────────────
