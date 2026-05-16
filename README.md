# ESP32 Quadcopter Flight Controller

A full-featured quadcopter flight controller built on the **Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)**, featuring a dual-core FreeRTOS architecture, MPU-9250 9-axis IMU with Mahony AHRS, BMP280 barometric altimeter, FlySky iBUS RC integration, Wi-Fi ground station, CPU load monitoring, and an autonomous RC-triggered calibration system — **no keyboard required**.

---

## Project Overview

The firmware is a set of concurrent FreeRTOS tasks distributed across both ESP32 cores, handling sensor acquisition, attitude estimation, PID control, motor output, barometric altitude, Wi-Fi telemetry, and CPU utilisation simultaneously. All calibration is triggered via RC switches — the drone can be fully set up and flight-ready without a laptop.

---

## Hardware Platform

| Component | Part | Notes |
|-----------|------|-------|
| Microcontroller | Adafruit HUZZAH32 Feather (ESP32-WROOM-32E) | Dual-core 240 MHz, Wi-Fi + BT |
| IMU | MPU-9250 (HiLetgo breakout) | Gyro + Accel + Mag via SPI (VSPI) |
| Barometer | BMP280 | Pressure / temperature / altitude via I²C |
| RC Transmitter | FlySky FS-i6X | 2.4 GHz AFHDS 2A, up to 10 channels |
| RC Receiver | FlySky FS-iA6B | iBUS single-wire serial output |
| ESC Protocol | DShot300 / DShot600 | Via ESP32 RMT peripheral (stub — wire when ready) |
| Power | 3S–4S LiPo | 5 V BEC for FC, 3.3 V for sensors |

---

## Wiring

### MPU-9250 → HUZZAH32 Feather (SPI / VSPI)

| MPU-9250 Pin | Feather GPIO | Feather Label |
|---|---|---|
| VCC | 3.3 V | 3V |
| GND | GND | GND |
| SCL / SCLK | GPIO 5 | SCK |
| SDA / MOSI | GPIO 18 | MO |
| AD0 / MISO | GPIO 19 | MI |
| NCS / CS | GPIO 33 | 33 |
| INT | GPIO 26 | 26 (optional) |

> **Warning:** 3.3 V only — the MPU-9250 is NOT 5 V tolerant.

### BMP280 → HUZZAH32 Feather (I²C)

| BMP280 Pin | Feather GPIO | Notes |
|---|---|---|
| VCC | 3.3 V | 3.3 V only |
| GND | GND | Common ground |
| SDA / SDI | GPIO 21 | I²C data |
| SCL / SCK | GPIO 22 | I²C clock |
| CSB | 3.3 V | Pull high → I²C mode |
| SDO | GND | Address 0x76 (or 3.3 V for 0x77) |

> The BMP280 uses I²C, keeping the SPI bus exclusively for the MPU-9250.

### FS-iA6B Receiver → HUZZAH32 Feather

| FS-iA6B Pin | Feather GPIO | Notes |
|---|---|---|
| iBUS (S.BUS/iBUS port) | GPIO 16 (UART2 RX) | 115200 baud, single-wire |
| VCC | 5 V (from BEC) | 4.0–6.5 V DC |
| GND | GND | Shared ground with FC |

> Use the **S.BUS/iBUS** port (small connector on the end) — not the individual CH1–CH6 servo connectors.

### Motor ESCs

| Motor | GPIO | Position |
|-------|------|----------|
| Front-Left | GPIO 25 | CCW |
| Front-Right | GPIO 27 | CW |
| Rear-Left | GPIO 14 | CW |
| Rear-Right | GPIO 32 | CCW |

---

## Repository Structure

```
quadcopter-fc/
│
├── RC_FlightController.ino         # Main sketch — RC + IMU + BMP280 + PID + Wi-Fi
│
├── Submodules/
│   ├── IMU/
│   │   ├── MPU9250.h               # IMU driver header (SPI, AHRS, calibration)
│   │   └── MPU9250.cpp             # IMU driver implementation
│   │
│   ├── BMP280/
│   │   ├── BMP280Sensor.h          # BMP280 I²C driver header
│   │   └── BMP280Sensor.cpp        # BMP280 I²C driver implementation
│   │
│   ├── iFly/
│   │   ├── FlySkyiBUS.h            # iBUS receiver driver header
│   │   └── FlySkyiBUS.cpp          # iBUS receiver driver implementation
│   │
│   ├── WiFiTelemetry/
│   │   ├── TelemetryWiFi.h         # Wi-Fi hotspot telemetry server header
│   │   └── TelemetryWiFi.cpp       # Wi-Fi hotspot telemetry server implementation
│   │
│   └── ESP32Core/
│       ├── CPUUtilization.h        # Dual-core CPU load monitor header
│       └── CPUUtilization.cpp      # Dual-core CPU load monitor implementation
│
├── Simulation/
│   └── DroneGroundStation.html     # Browser-based ground station (Serial + Wi-Fi)
│
└── README.md                       # This file
```

---

## RC Switch Assignments (FS-i6X)

| Channel | Control | Function |
|---------|---------|----------|
| CH1 | Right stick L/R | Roll |
| CH2 | Right stick U/D | Pitch |
| CH3 | Left stick U/D | Throttle |
| CH4 | Left stick L/R | Yaw |
| CH5 | VrA (knob) | Spare / PID tune |
| CH6 | VrB (knob) | Spare / PID tune |
| CH7 | SWA (2-pos) | **ARM / DISARM** |
| CH8 | SWB (2-pos) | **ANGLE / ACRO** flight mode |
| CH9 | SWC (3-pos) | Accel orientation confirm during calibration |
| CH10 | SWD (2-pos) | **CALIBRATION trigger** (flip UP while disarmed) |

---

## FreeRTOS Task Architecture

| Task | Core | Priority | Rate | Responsibility |
|------|------|----------|------|----------------|
| `taskIMU` | Core 1 | 5 (highest) | 500 Hz | SPI burst read → Mahony AHRS → shared state |
| `taskPID` | Core 1 | 4 | 500 Hz | Cascaded PID loops → motor mix → ESC output |
| `taskRC` | Core 0 | 3 | 200 Hz | iBUS frame decode → arm / calib edge detect |
| `taskBMP` | Core 0 | 1 | 20 Hz | BMP280 I²C read → pressure / temp / altitude |
| `taskCPU` | Core 0 | 1 | 2 Hz | FreeRTOS idle-hook CPU load estimate |
| `taskWiFi` | Core 0 | 1 | event | HTTP server → `/telemetry` JSON endpoint |
| `taskSerial` | Core 0 | 1 | 20 Hz | 1 Hz telemetry print, calibration progress |

All tasks share a single `SemaphoreHandle_t` mutex (`g_flightMutex`) to protect the `FlightState` struct. The PID task is suspended (`vTaskSuspend`) for the entire calibration routine, and motors are stopped before calibration begins.

---

## Subsystems

### 1. IMU Driver (`MPU9250.h` / `MPU9250.cpp`)

Register-level SPI driver for the MPU-9250 (MPU-6500 accel/gyro die + AK8963 magnetometer die).

- Gyro ±500 °/s, Accel ±8 g, DLPF 41 Hz
- AK8963 in continuous mode 2 (100 Hz, 16-bit), read via SLV0 8-byte auto-burst
- SLV4 used for all one-shot AK8963 writes (avoids SLV0 pipeline corruption)
- Factory sensitivity adjustment (ASA) read from AK8963 Fuse ROM at init
- NVS flash persistence via `Preferences` — `saveCalibration()` / `loadCalibration()`

**Key data structures:**

| Struct | Contents |
|--------|----------|
| `MPU_RawData` | Raw 16-bit counts for all 9 axes + temp + magOk flag |
| `MPU_SensorData` | Calibrated floats: ax/ay/az (g), gx/gy/gz (°/s), mx/my/mz (µT), temp (°C) |
| `MPU_Attitude` | roll / pitch / yaw (°), quaternion q0–q3 |
| `MPU_CalData` | 15 bias + scale values + AK8963 ASA + valid flag |

---

### 2. Autonomous Calibration System

Triggered by **flipping SWD (CH10) UP** while the drone is disarmed. No keyboard or serial terminal required. The entire routine runs on Core 1 inside `taskIMU`, with the PID task suspended throughout.

#### Stage 1 — Gyroscope (~8 s, fully automatic)

1. Place drone flat and completely still.
2. 3 s settle delay, then 5 s of averaging (2500 samples at 2 ms intervals).
3. Bias applied directly to `cal.gx_b / gy_b / gz_b`.

#### Stage 2 — Accelerometer (~50 s, 6 orientations)

For each of 6 positions (±X, ±Y, ±Z up):
1. Serial prints which face to point upward.
2. Position drone in that orientation.
3. **Flip SWC (CH9) UP** to confirm — sampling begins immediately (3 s, 1500 samples).
4. **Flip SWC DOWN** to advance to the next position.
5. Bias and scale computed from opposite-pair midpoints and half-ranges.

#### Stage 3 — Magnetometer (30 s timed sweep, no button presses)

1. Serial instructs you to rotate the drone through all axes (figure-8 motion).
2. Min/max tracked for 30 s. Time remaining printed every 10 s.
3. Hard-iron bias = (max + min) / 2 per axis.
4. Soft-iron scale = average span / per-axis span.

All results are **automatically saved to NVS flash** on completion. They load on every subsequent boot without any user action.

---

### 3. AHRS — Mahony Complementary Filter

9-DOF sensor fusion producing Roll / Pitch / Yaw Euler angles and a unit quaternion.

- Quaternion state (`_q0`–`_q3`) lives inside the `MPU9250` class — accumulates correctly across calls
- Accel correction normalisation guard prevents NaN during free-fall or cold start
- Magnetometer applied only when `_magValid == true` and mag norm > 0.1 µT — otherwise falls back to 6-DOF (no yaw drift from bad data)
- `dt` clamped to [0.0001, 0.05] s to prevent filter blow-up on first call or after a delay
- Tunable `mahonyKp` (default 2.0) and `mahonyKi` (default 0.005)

**Axis convention (body frame, NED):**  
X → nose, Y → right wing, Z → down  
Roll (+) = right wing down, Pitch (+) = nose up, Yaw = 0–360° from magnetic north

---

### 4. PID Flight Controller

Cascaded (inner + outer) PID running at 500 Hz on Core 1.

**ANGLE mode** (self-level):
- Outer loop: RC stick → desired angle → angle error → rate setpoint (via `pidAngleRoll` / `pidAnglePitch`)
- Inner loop: rate setpoint → rate error → motor correction (via `pidRateRoll` / `pidRatePitch` / `pidRateYaw`)

**ACRO mode** (rate-only):
- Inner loop only: RC stick → desired rate → rate error → motor correction

**Motor mix (+ quad geometry):**
```
motorFL = throttle + rollOut - pitchOut - yawOut   (CCW)
motorFR = throttle - rollOut - pitchOut + yawOut   (CW)
motorRL = throttle + rollOut + pitchOut + yawOut   (CW)
motorRR = throttle - rollOut + pitchOut - yawOut   (CCW)
```

**Recommended starting gains:**

| Axis | Kp | Ki | Kd |
|------|----|----|----|
| Roll rate | 0.5 | 0.002 | 0.010 |
| Pitch rate | 0.5 | 0.002 | 0.010 |
| Yaw rate | 1.0 | 0.005 | 0.000 |
| Roll angle | 4.0 | 0.0 | 0.0 |
| Pitch angle | 4.0 | 0.0 | 0.0 |

Tune the **inner (rate) loop first** before enabling the outer (angle) loop.

---

### 5. BMP280 Barometric Altimeter (`BMP280Sensor.h` / `BMP280Sensor.cpp`)

I²C driver for the Bosch BMP280, providing pressure, temperature, and altitude with no external library.

- Auto-detects I²C address (0x76 or 0x77) via `beginAuto()`
- `scanI2C()` debug helper lists all devices on the bus
- IIR filter x4, standby 125 ms, pressure oversampling x16, temp oversampling x2
- Bosch 64-bit integer compensation formulas from datasheet §4.2.3
- Altitude formula: `h = 44330 × (1 − (P / P₀)^0.19029495)`
- Configurable sea-level QNH via `setSeaLevelPressure(hpa)`
- FreeRTOS mutex (`_mutex`) guards I²C reads — safe to call from any task
- Singleton `bmp280` declared in `BMP280Sensor.cpp`

**Output struct `BMP280Data`:**
```cpp
float temperature_c;   // °C
float pressure_hpa;    // hPa
float altitude_m;      // m (relative to configured sea-level pressure)
bool  valid;
uint32_t ts_ms;
```

---

### 6. RC Receiver Interface (`FlySkyiBUS.h` / `FlySkyiBUS.cpp`)

Single-wire iBUS decoder for the FlySky FS-iA6B, running at 115200 baud on UART2.

**iBUS frame format:**
```
Byte  0    : 0x20  (frame length)
Byte  1    : 0x40  (command)
Bytes 2–29 : 14 × 2 bytes, little-endian (1000–2000 µs per channel)
Bytes 30–31: Checksum = 0xFFFF − Σ(bytes 0..29)
Frame rate : ~7 ms (≈142 Hz)
```

**Key API:**
```cpp
rcReceiver.begin(16, 17, 2);           // RX=GPIO16, TX=GPIO17 (unused), UART2
rcReceiver.update();                    // call in FreeRTOS task, drains UART buffer
RCCommand cmd = rcReceiver.getCommand(); // fully normalised pilot intent
// cmd.throttle  0.0–1.0
// cmd.roll / pitch / yaw  −1.0–+1.0  (deadband applied)
// cmd.mode      DISARMED / ANGLE / ACRO / FAILSAFE
// cmd.swdHigh   true when SWD (CH10) is UP — sole calibration trigger
```

**Failsafe:** No valid frame for 500 ms → `FlightMode::FAILSAFE`, motors cut, failsafe counter incremented. Re-arm requires cycling SWA low then high.

---

### 7. Wi-Fi Telemetry (`TelemetryWiFi.h` / `TelemetryWiFi.cpp`)

The ESP32 hosts its own Wi-Fi access point and serves a JSON telemetry endpoint, compatible with the browser-based ground station.

- SSID: `ESP32-DRONE` / Password: `12345678`
- Endpoint: `http://192.168.4.1/telemetry` — returns JSON at up to 10 Hz
- CORS headers included — works when DroneGroundStation.html is opened as a local file
- Callback pattern: `setTelemetryProvider(provideTelemetry)` — flight controller fills `TelemetryPacket` under mutex each request

**`TelemetryPacket` fields:** tick, mode, roll/pitch/yaw, throttle, RC axes, motor outputs, RC frame rate, armed flag, BMP280 temp/pressure/altitude/valid, CPU core loads.

---

### 8. CPU Utilisation Monitor (`CPUUtilization.h` / `CPUUtilization.cpp`)

Lightweight dual-core CPU load estimation using FreeRTOS idle hooks.

- Registers `idleHookCore0` and `idleHookCore1` via `esp_register_freertos_idle_hook_for_cpu()`
- Counts idle-hook invocations per sample window (default 1 s)
- Best observed idle count becomes the local 0%-load baseline — self-calibrating
- Output: `core0_pct` and `core1_pct` (0–100 %) + `valid` flag
- Accuracy improves after the first few seconds of runtime

---

### 9. Ground Station (`DroneGroundStation.html`)

A single-file browser application with two connection modes:

**USB Serial (Web Serial API — Chrome / Edge only):**  
Connect via USB, click **CONNECT SERIAL**. Parses the 1 Hz telemetry line printed by `taskSerial`.

**Wi-Fi JSON polling:**  
Connect your laptop/phone to the `ESP32-DRONE` hotspot, open the HTML locally, click **CONNECT WIFI**. Polls `/telemetry` at 10 Hz.

**Telemetry line format parsed by the browser:**
```
[  tick] MODE | R=±X.X P=±X.X Y=X.X | T=X.XX R=±X.XX P=±X.XX Y=±X.XX |
MOT X.XX X.XX X.XX X.XX | RC@XXXHz |
BMP T=X.XC P=X.XhPa ALT=X.Xm OK|NO_BMP |
CPU C0=XX% C1=XX% OK|WAIT
```

**UI panels:**

| Panel | Content |
|-------|---------|
| RC Inputs | Stick visualisers, normalised channel bars, manual test sliders |
| Attitude Indicator | Animated artificial horizon, roll/pitch/yaw readout |
| Compass | Heading tape with cardinal directions |
| Motor Outputs | Per-motor percentage bars with colour coding |
| BMP280 | Temperature, pressure, altitude, sensor status |
| CPU Load | Core 0 and Core 1 utilisation bars |
| Telemetry Health | Parser status, tick, RC Hz, lines/s, data age |
| Log | Colour-coded scrolling log (telem / cal / boot / RC / warn) |

---

## Build & Flash

### Requirements

- Arduino IDE 2.x or PlatformIO
- ESP32 Arduino Core ≥ 2.0.0 (Adafruit HUZZAH32 board definition)
- Libraries (all included in ESP32 Arduino Core — no external installs needed):
  - `Preferences` — NVS flash calibration storage
  - `Wire` — I²C for BMP280
  - `WiFi` + `WebServer` — Wi-Fi telemetry

> The `DShotRMT` library is referenced in comments for the motor stub. Install it from Library Manager when ESCs are wired.

### Arduino IDE Setup

1. **Tools → Board** → `Adafruit ESP32 Feather`
2. **Tools → Port** → select your USB-serial port
3. **Tools → Upload Speed** → `921600`
4. Open `RC_FlightController.ino`
5. Ensure all submodule `.h` / `.cpp` files are in the same sketch folder (or configure include paths in PlatformIO)
6. Click **Upload**

> Use **Adafruit ESP32 Feather** — not "ESP32 Dev Module". The partition map and pin assignments differ.

---

## First-Time Setup

1. Flash firmware with props **removed**
2. Open Serial Monitor at **115200 baud**
3. Power the drone; wait ~5 s for IMU warm-up
4. Confirm BMP280 found: `[BOOT] BMP280 OK at I2C address 0x76`
5. If no NVS calibration exists, Serial prints a calibration prompt
6. **Flip SWD (CH10) UP** while SWA is DOWN (disarmed) to start autonomous calibration
7. Follow on-screen instructions for each stage (gyro → accel → mag)
8. Calibration auto-saves to flash on completion
9. On subsequent boots, calibration loads automatically — no action needed

> **Re-calibrate** whenever: motors are repositioned, the frame is rebuilt, or the drone is moved to a magnetically different environment.

---

## Safety Notice

- **Always remove propellers** during development and calibration
- Never arm the drone over USB power alone — use a fully charged LiPo with props off
- Verify motor spin direction and mixer sign before first flight
- Set a conservative throttle limit (< 50 %) for maiden flights
- The motor output in `writeMotors()` is a **stub** — replace with DShotRMT calls before connecting ESCs

---

## License

MIT License — free to use, modify, and distribute with attribution.

---

## Acknowledgements

- **Platform:** Adafruit HUZZAH32 Feather / ESP32-WROOM-32E
- **IMU:** InvenSense / TDK MPU-9250 + AK8963
- **Barometer:** Bosch BMP280
- **AHRS:** Mahony complementary filter
- **RC:** FlySky FS-i6X + FS-iA6B iBUS protocol
- **ESC:** DShotRMT library by derdoktor667
- **RTOS:** FreeRTOS (integrated into ESP32 Arduino Core)
