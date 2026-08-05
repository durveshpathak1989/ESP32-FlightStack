//  Motor + SWC helpers
// ─────────────────────────────────────────────────────────────
static void writeMotors(float fl,float fr,float rl,float rr) {
    motorServiceAdapter.write({fl, fr, rl, rr});
}
static void motorsOff() { motorServiceAdapter.stop(); }
//  TASKS
// ═════════════════════════════════════════════════════════════
// ─────────────────────────────────────────────────────────────
//  taskGPS — Core 0, priority 1, 50 Hz
// ─────────────────────────────────────────────────────────────
static void taskGPS(void* /*pv*/)
{
    gpsServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskRC — Core 0, priority 3
//  Important: do NOT use vTaskDelayUntil(5 ms) here if the FreeRTOS tick
//  is coarse. On this build, one tick is behaving like ~40 ms, which can
//  starve the iBUS parser. Use clockServiceAdapter.microseconds() pacing + taskYIELD() instead.
// ─────────────────────────────────────────────────────────────
static void taskRC(void* /*pv*/)
{
    static ReceiverServiceTask receiverTask(
        receiverServiceAdapter, calibrationServiceAdapter, clockServiceAdapter,
        receiverFramePort,
        calLog, ESC_CALIB_VRB_THRESHOLD, TUNE_THROTTLE_CUT, RC_CH_AUX5);
    receiverTask.run();
}

static void computeControlAttitude(const AttitudeEstimate& att,
                                   float& rollCtrl,
                                   float& pitchCtrl,
                                   float& yawCtrl)
{
    rollCtrl  = levelTrim.controlRollDeg(att.roll_deg);
    pitchCtrl = levelTrim.controlPitchDeg(att.pitch_deg);
    yawCtrl   = levelTrim.controlYawDeg(att.yaw_deg);
}

static LowPassFilter lpfGx, lpfGy, lpfGz;

// Diagnostic-only attitude estimates published to telemetry.
// These are not used for control; they help compare AHRS fusion vs raw accel
// tilt and gyro-only integration on the ground station.
static float g_gyroRollDeg  = 0.0f;
static float g_gyroPitchDeg = 0.0f;
static float g_gyroYawDeg   = 0.0f;
static bool  g_gyroAngleInit = false;

// Tracks whether we have already commanded the ESC outputs off.
// motorOff() is intentionally NOT called every 400 Hz cycle because the
// current MotorControl implementation appears to block for ~40 ms.
static bool  g_motorOutputsActive = false;

static void updateDynamicNotchFromFFT()
{
    if (!TUNE_DYNAMIC_NOTCH_ENABLE) return;

    static uint32_t lastUpdateMs = 0;
    uint32_t nowMs = clockServiceAdapter.milliseconds();

    if (nowMs - lastUpdateMs < DYN_NOTCH_UPDATE_MS) {
        return;
    }

    lastUpdateMs = nowMs;

    // Snapshot flight state inside this function.
    // This avoids Arduino prototype issues with FlightState in the function argument.
    FlightState s;
    if (!flightStateRte.read(s, pdMS_TO_TICKS(5))) {
        return;
    }

    // Do not learn while disarmed, motors off, failsafe, or low throttle.
    if (!s.armed ||
        !g_motorOutputsActive ||
        !s.rc.valid ||
        s.rc.throttle < DYN_NOTCH_MIN_THROTTLE) {

        if (LOG_DYNAMIC_NOTCH_DEBUG) {
            static uint32_t lastGatePrintMs = 0;
            if (nowMs - lastGatePrintMs >= 1000) {
                lastGatePrintMs = nowMs;

                DBG_PRINTF("[DYN_NOTCH_GATE] blocked armed=%d motorOut=%d rcValid=%d throttle=%.3f minThrottle=%.3f\n",
                              s.armed ? 1 : 0,
                              g_motorOutputsActive ? 1 : 0,
                              s.rc.valid ? 1 : 0,
                              s.rc.throttle,
                              DYN_NOTCH_MIN_THROTTLE);
            }
        }

        return;
    }

    bool notchEnabled = false;
    float currentHz = TUNE_NOTCH_FREQ_HZ;

    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        notchEnabled = g_tuning.notch_enable;
        currentHz = g_tuning.notch_freq_hz;
        tuningRte.unlock();
    } else {
        return;
    }

    if (!notchEnabled) {
        return;
    }

    float peakHz = 0.0f;
    float peakScore = 0.0f;
    uint32_t seq = 0;

    bool ok = spectrumAnalyzer.findGyroPeak(DYN_NOTCH_MIN_HZ,
                                            DYN_NOTCH_MAX_HZ,
                                            DYN_NOTCH_MIN_SCORE,
                                            peakHz,
                                            peakScore,
                                            seq);

    if (!ok) {
        if (LOG_DYNAMIC_NOTCH_DEBUG) {
            static uint32_t lastNoPeakPrintMs = 0;
            if (nowMs - lastNoPeakPrintMs >= 1000) {
                lastNoPeakPrintMs = nowMs;

                DBG_PRINTF("[DYN_NOTCH_NO_PEAK] current=%.1fHz min=%.1f max=%.1f minScore=%.2f seq=%lu\n",
                              currentHz,
                              DYN_NOTCH_MIN_HZ,
                              DYN_NOTCH_MAX_HZ,
                              DYN_NOTCH_MIN_SCORE,
                              (unsigned long)seq);
            }
        }

        return;
    }

    // Smooth toward the detected FFT peak.
    float targetHz = currentHz + DYN_NOTCH_ALPHA * (peakHz - currentHz);

    // Limit movement per update so the notch does not chase noise.
    float stepHz = targetHz - currentHz;
    stepHz = constrain(stepHz, -DYN_NOTCH_MAX_STEP_HZ, DYN_NOTCH_MAX_STEP_HZ);

    float requestedHz = currentHz + stepHz;
    requestedHz = constrain(requestedHz, DYN_NOTCH_MIN_HZ, DYN_NOTCH_MAX_HZ);

    // Avoid tiny reconfigurations.
    if (fabsf(requestedHz - currentHz) < 0.5f) {
        return;
    }

    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        g_tuning.notch_freq_hz = requestedHz;
        g_tuning.dirty = true;   // taskControl applies notch safely
        tuningRte.unlock();
    }

    if (LOG_DYNAMIC_NOTCH_DEBUG) {
        DBG_PRINTF("[DYN_NOTCH] peak=%.1fHz score=%.3f current=%.1fHz -> request=%.1fHz seq=%lu\n",
                      peakHz,
                      peakScore,
                      currentHz,
                      requestedHz,
                      (unsigned long)seq);
    }
}

// ═════════════════════════════════════════════════════════════
//  HIGH-SPEED ON-BOARD FLIGHT LOG  (Test 7.1 / 7.3 / 7.4 capture)
//
//  Expanded debug row, logged at 100 Hz (every 4th 400 Hz cycle).
//  Workflow: POST /flightlog/reset  →  arm + fly  →  disarm  →
//            GET /flightlog/csv (freezes the buffer, streams it out).
//
//  FLIGHT_LOG_SIZE=300 → ~3.0 s at 100 Hz with extended fields. Raise only if
//  free heap allows (check the 'I'-style resource report first).
//
//  Concurrency: writer (taskControl, Core 1) and the freeze flag
//  share one portMUX spinlock. GET freezes logging first, so the
//  WiFi task (Core 0) reads a static buffer with no further writes
//  — no second copy buffer needed, and rows are streamed (chunked)
//  so no giant CSV String is ever built in heap.
// ═════════════════════════════════════════════════════════════

static void resetFlightLog() {
    flightLogger.reset();
}

static uint16_t flightLogCount() {
    return flightLogger.countAndFreeze();
}

static String flightLogHeader() {
    return flightLogger.csvHeader();
}

static String flightLogRowCsv(uint16_t i) {
    return flightLogger.csvRow(i);
}




// ─────────────────────────────────────────────────────────────
#include "src/Application/Runtime/FlightControlRuntime.inc"
#include "src/Application/Diagnostics/SerialDiagnosticsRuntime.inc"
//  taskToF — Core 0, priority 1, 40 Hz
// ─────────────────────────────────────────────────────────────
static void taskToF(void* /*pv*/)
{
    tofServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskBMP — Core 0, priority 1, 20 Hz
// ─────────────────────────────────────────────────────────────
static void taskBMP(void* /*pv*/)
{
    barometerServiceTask.run();
}

// ─────────────────────────────────────────────────────────────
//  taskCPU — Core 0, priority 1, 2 Hz
// ─────────────────────────────────────────────────────────────
static void taskCPU(void* /*pv*/)
{
    cpuServiceTask.run();
}


// ─────────────────────────────────────────────────────────────
//  Wi-Fi spectrum provider — /spectrum endpoint
// ─────────────────────────────────────────────────────────────
static String provideSpectrumJson()
{
    TuningState t;
    memset(&t, 0, sizeof(t));
    if (tuningRte.lock(pdMS_TO_TICKS(5))) {
        t = g_tuning;
        tuningRte.unlock();
    } else {
        t.notch_enable  = TUNE_NOTCH_ENABLE;
        t.notch_freq_hz = TUNE_NOTCH_FREQ_HZ;
        t.notch_q       = TUNE_NOTCH_Q;
    }
    return spectrumAnalyzer.toJson(t.notch_freq_hz, t.notch_q, t.notch_enable);
}

// ─────────────────────────────────────────────────────────────
//  OTA safety gate — Web firmware update is bench-only.
//  OTA is allowed only when DISARMED, throttle low, and motors inactive.
// ─────────────────────────────────────────────────────────────
static bool otaIsSafeToStart()
{
    FlightState s;
    memset(&s, 0, sizeof(s));

    if (!flightStateRte.read(s, pdMS_TO_TICKS(5))) return false;

    return FlightSafetyPolicy::otaAllowed(
        {s.armed, s.rc.throttle, s.motorFL, s.motorFR, s.motorRL, s.motorRR});
}

// ─────────────────────────────────────────────────────────────
//  taskWiFi — Core 0, priority 1, event-driven
// ─────────────────────────────────────────────────────────────
static void taskWiFi(void* /*pv*/)
{
    const WifiServiceBindings bindings{
        provideTelemetry, handleTune, otaIsSafeToStart,
        provideTimingJson, provideTimingCsv, resetTimingStats,
        provideSpectrumJson, provideIdentityJson,
        flightLogCount, flightLogHeader, flightLogRowCsv, resetFlightLog
    };
    wifiServiceAdapter.bind(bindings);
    wifiServiceTask.configure("ESP32-DRONE", "12345678");
    wifiServiceTask.run();
}

// ═════════════════════════════════════════════════════════════
