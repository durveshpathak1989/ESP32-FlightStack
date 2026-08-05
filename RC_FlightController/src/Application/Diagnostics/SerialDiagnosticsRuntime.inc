//  taskSerial — Core 0, priority 1, 20 Hz
//  1 Hz status line; optional ~4 Hz [PID] trace (Serial 'p' toggles).
// ─────────────────────────────────────────────────────────────
static SerialDiagnosticsSwc serialDiagnosticsSwc;
static char g_diagnosticMessage[512];

static void diagnosticPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(g_diagnosticMessage, sizeof(g_diagnosticMessage), format, args);
    va_end(args);
    diagnosticServiceAdapter.Write(g_diagnosticMessage);
}

void SerialDiagnosticsSwc::Init()
{
    tick_ = 0;
    diagnosticServiceAdapter.Write(
        "\nFlySky iBUS + selectable IMU + BMP280 + GPS v6.1.0\n"
        "Wi-Fi: ESP32-DRONE / 12345678 -> 192.168.4.1\n"
        "Control: timer-released 400 Hz; type p to toggle PID trace.\n");
}

void SerialDiagnosticsSwc::Periodic()
{
        rte::SignalSample<flight::ReceiverFrame> receiverSignal{};
        receiverFramePort.receive(receiverSignal);
        const flight::PilotCommand& calCmd = receiverSignal.value.command;

        bool safeForCalibration =
            calCmd.valid &&
            calCmd.mode == flight::FlightMode::Disarmed;

        calibrationServiceAdapter.SetSafety(safeForCalibration);

        // SWC / CH9 confirms the current calibration step
        static bool swcPrevForCal = false;
        bool swcNowForCal = calCmd.raw[RC_CH_AUX3] >= SWC_THRESHOLD;

        if (swcNowForCal && !swcPrevForCal) {
            calibrationServiceAdapter.ConfirmStep();
        }

        swcPrevForCal = swcNowForCal;

        calibrationServiceAdapter.PeriodicService();
        // Print CalibrationManager status when active or when state changes
        static uint8_t lastCalState = 0;
        static uint32_t lastCalPrintMs = 0;

        const rte::CalibrationServiceStatus calStatus =
            calibrationServiceAdapter.Status();

        if (calStatus.stateCode != lastCalState ||
            (calStatus.active && clockServiceAdapter.milliseconds() - lastCalPrintMs >= 1000)) {

            lastCalState = calStatus.stateCode;
            lastCalPrintMs = clockServiceAdapter.milliseconds();

            calLogf("[CAL-MGR] run=%lu mode=%s src=%s state=%s active=%d safe=%d confirm=%d progress=%.0f%% msg=%s",
                    (unsigned long)calStatus.runId,
                    calStatus.modeName,
                    calStatus.sourceName,
                    calStatus.stateName,
                    calStatus.active ? 1 : 0,
                    calStatus.safeToRun ? 1 : 0,
                    calStatus.requiresUserConfirm ? 1 : 0,
                    (double)(calStatus.progress * 100.0f),
                    calStatus.message);
            if (calStatus.error != nullptr && calStatus.error[0]) {
                calLogf("[CAL-MGR][ERROR] run=%lu %s",
                        (unsigned long)calStatus.runId,
                        calStatus.error);
            }
        }

        // Handle inbound Serial commands (non-blocking)
        while (diagnosticServiceAdapter.CommandAvailable()) {
            int c = diagnosticServiceAdapter.ReadCommandByte();
            if (c == 'p' || c == 'P') {
                g_pidTrace = !g_pidTrace;
                diagnosticPrintf("[CMD] PID trace %s\n", g_pidTrace ? "ON" : "OFF");
            }
        }

        // Snapshot once per loop; both blocks below use it.
        FlightState s;
        flightStateRte.read(s, pdMS_TO_TICKS(5));
        updateDynamicNotchFromFFT();
        const char* ms = "???";
        switch(s.rc.mode) {
            case flight::FlightMode::Disarmed: ms="DISARMD"; break;
            case flight::FlightMode::Angle:    ms="ANGLE  "; break;
            case flight::FlightMode::Acro:     ms="ACRO   "; break;
            case flight::FlightMode::Failsafe: ms="FAILSFE"; break;
        }

        // 1 Hz status line
        if (tick_ % 20 == 0) {
            diagnosticPrintf("[%6lu] %s | RAW R=%+6.1f P=%+6.1f Y=%6.1f | "
                          "CTRL R=%+6.1f P=%+6.1f Y=%+6.1f | "
                          "OFF R=%+5.2f P=%+5.2f Y=%+5.2f | "
                          "MOT %.2f %.2f %.2f %.2f | "
                          "BMP %.1fC %.1fhPa %.1fm | "
                          "ToF %s %.3fm obj=%u %lums | "
                          "GPS %s Sats=%d | "
                          "Mag:%s Est:%s mKp=%.2f\n",
                          (unsigned long)tick_, ms,
                          s.roll_deg, s.pitch_deg, s.yaw_deg,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.roll_offset_deg, s.pitch_offset_deg, s.yaw_offset_deg,
                          s.motorFL, s.motorFR, s.motorRL, s.motorRR,
                          s.bmpTemp_c, s.bmpPressure_hpa, s.bmpAltitude_m,
                          s.tofValid ? "OK" : (s.tofReady ? "---" : "OFF"),
                          s.tofDistance_m,
                          (unsigned int)s.tofObjectCount,
                          (unsigned long)((s.tofAge_ms == UINT32_MAX) ? 0 : s.tofAge_ms),
                          s.gps.valid ? "FIX" : "---", s.gps.satellites,
                          s.magValid ? "9DOF" : "6DOF",
                          (s.ahrsFilterMode == 1) ? "Mahony" : ((s.ahrsFilterMode == 2) ? "Madgwick" : "EKF"),
                          mahony.kp());
            diagnosticPrintf("          CH: 1=%4u 2=%4u 3=%4u 4=%4u 5=%4u 6=%4u 7=%4u 8=%4u 9=%4u 10=%4u\n",
                          s.rc.raw[0], s.rc.raw[1], s.rc.raw[2], s.rc.raw[3], s.rc.raw[4],
                          s.rc.raw[5], s.rc.raw[6], s.rc.raw[7], s.rc.raw[8], s.rc.raw[9]);

            float magNorm = sqrtf(s.mx_uT * s.mx_uT +
                      s.my_uT * s.my_uT +
                      s.mz_uT * s.mz_uT);

            diagnosticPrintf("          SENS: acc=%+.4f %+.4f %+.4f g | "
                        "gyro=%+.4f %+.4f %+.4f dps | "
                        "mag=%+.2f %+.2f %+.2f uT | norm=%.2f uT\n",
                        s.ax_g, s.ay_g, s.az_g,
                        s.gx_dps, s.gy_dps, s.gz_dps,
                        s.mx_uT, s.my_uT, s.mz_uT,
                        magNorm);
        }

        // ~4 Hz PID tuning trace (sibling of the 1 Hz block; gated by 'p')
        if (g_pidTrace && s.armed && (tick_ % 5 == 0)) {
            diagnosticPrintf("[PID] %s | cmd r=%+.2f p=%+.2f y=%+.2f | "
                          "ctrlAtt R=%+6.2f P=%+6.2f Y=%+6.2f | "
                          "gyro gx=%+7.1f gy=%+7.1f gz=%+7.1f | "
                          "out rO=%+.4f pO=%+.4f yO=%+.4f | "
                          "yawSP=%+6.1f hold=%d\n",
                          ms, s.rc.roll, s.rc.pitch, s.rc.yaw,
                          s.roll_ctrl_deg, s.pitch_ctrl_deg, s.yaw_ctrl_deg,
                          s.gx_dps, s.gy_dps, s.gz_dps,
                          s.pidRollOut, s.pidPitchOut, s.pidYawOut,
                          flightController.yawSetpointDeg(),
                          (int)flightController.yawHoldActive());
        }

        tick_++;
}

static void taskSerial(void* /*pv*/)
{
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t lastWake = xTaskGetTickCount();
    std::uint32_t stackReportDivider = 0;
    serialDiagnosticsSwc.Init();
    for (;;) {
        serialDiagnosticsSwc.Periodic();
        if (++stackReportDivider >= 100) {
            stackReportDivider = 0;
            diagnosticPrintf("[STACK] Serial minimum free stack: %u bytes\n",
                             static_cast<unsigned>(
                                 uxTaskGetStackHighWaterMark(nullptr)));
        }
        vTaskDelayUntil(&lastWake, period);
    }
}

// ─────────────────────────────────────────────────────────────
