# HighRateLogger Integration Guide

Add these files to your Arduino project folder:

- `HighRateLogger.h`
- `HighRateLogger.cpp`

## 1. Include the logger in `RC_FlightController.ino`

Add near your other includes:

```cpp
#include "HighRateLogger.h"
```

## 2. Add provider functions in `RC_FlightController.ino`

Place these near `provideTimingCsv()` / `resetTimingStats()`:

```cpp
static String provideFlightLogCsv()
{
    return highRateLogger.toCsv();
}

static String provideFlightLogStatusJson()
{
    String s;
    s.reserve(128);
    s += F("{\"rows\":");
    s += String(highRateLogger.count());
    s += F(",\"capacity\":");
    s += String(HIGH_RATE_LOG_SIZE);
    s += F(",\"full\":");
    s += highRateLogger.isFull() ? F("true") : F("false");
    s += F(",\"seq\":");
    s += String(highRateLogger.sequence());
    s += F("}");
    return s;
}

static void resetFlightLog()
{
    highRateLogger.reset();
}
```

## 3. Register logger endpoints in `taskWiFi()`

Add after timing endpoint registration:

```cpp
telemetryWiFi.setFlightLogCsvProvider(provideFlightLogCsv);
telemetryWiFi.setFlightLogStatusProvider(provideFlightLogStatusJson);
telemetryWiFi.setFlightLogResetHandler(resetFlightLog);
```

## 4. Push log rows inside `taskControl()`

After motor mixing and after `writeMotors(fl, fr, rl, rr);`, add:

```cpp
static uint8_t flightLogDivider = 0;
flightLogDivider++;

if (flightLogDivider >= 4) {   // 400 Hz / 4 = 100 Hz logging
    flightLogDivider = 0;

    HighRateLogRow row = {};
    row.t_us      = micros();
    row.loopCount = g_state.loopCount;
    row.period_us = (uint16_t)min(periodUs, (uint32_t)65535);

    row.mode  = (uint8_t)cmd.mode;
    row.armed = 1;
    row.imuOk = imuOk ? 1 : 0;

    row.rcThrottle = cmd.throttle;
    row.thrRaw     = thrRaw;
    row.thrTarget  = thrTarget;
    row.thrSmooth  = thrSmooth;
    row.thrFinal   = thr;

    row.rcRoll  = rollCmd;
    row.rcPitch = pitchCmd;
    row.rcYaw   = yawCmd;

    row.roll_deg  = roll;
    row.pitch_deg = pitch;
    row.yaw_deg   = imuOk ? att.yaw : 0.0f;

    row.gxRaw_dps  = imuOk ? s.gx_dps : 0.0f;
    row.gyRaw_dps  = imuOk ? s.gy_dps : 0.0f;
    row.gzRaw_dps  = imuOk ? s.gz_dps : 0.0f;
    row.gxFilt_dps = gx;
    row.gyFilt_dps = gy;
    row.gzFilt_dps = gz;

    row.rollOut  = rO;
    row.pitchOut = pO;
    row.yawOut   = yO;

    row.motorFL = fl;
    row.motorFR = fr;
    row.motorRL = rl;
    row.motorRR = rr;

    highRateLogger.push(row);
}
```

For disarmed logging, you can either skip rows or add a smaller log block in the disarmed branch. For flight analysis, logging while armed is usually enough.

## 5. Wi-Fi workflow

Use these endpoints:

- `GET /flightlog/status` — check how many rows are stored
- `POST /flightlog/reset` — reset before a test
- `GET /flightlog.csv` — download after the test

Recommended test flow:

1. Connect to drone Wi-Fi.
2. Open `/flightlog/reset` before test.
3. Run props-off, restrained, or hover test.
4. Disarm.
5. Download `/flightlog.csv`.
6. Save file with a test-specific name.

Do not continuously download `/flightlog.csv` while flying. The logger is designed for post-run download.
