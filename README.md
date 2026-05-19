# ESP32 Quadcopter Flight Controller

A full-featured quadcopter flight controller built on the **Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)**, featuring a dual-core FreeRTOS architecture, MPU-9250 9-axis IMU, Mahony AHRS, BMP280 barometric altimeter, u-blox NEO-6M GPS, FlySky iBUS RC integration, PWM motor output, Wi-Fi ground station with live GPS map, CPU load monitoring, and an autonomous RC-triggered calibration system — **no keyboard or laptop required for field operation**.

---

## Hardware Platform

| Component | Part | Notes |
|---|---|---|
| Microcontroller | Adafruit HUZZAH32 Feather (ESP32-WROOM-32E) | Dual-core 240 MHz, Wi-Fi + BT |
| IMU | MPU-9250 (HiLetgo breakout) | Gyro + Accel + Mag via SPI (VSPI) |
| Barometer | BMP280 | Pressure / temperature / altitude via I²C |
| GPS | GY-GPS6MV2 (u-blox NEO-6M) | NMEA 0183 via UART1 at 9600 baud |
| RC Transmitter | FlySky FS-i6X | 2.4 GHz AFHDS 2A, up to 10 channels |
| RC Receiver | FlySky FS-iA6B | iBUS single-wire serial output |
| ESC Protocol | Standard PWM 50 Hz (1000–2000 µs) | Via ESP32 LEDC peripheral |
| Power | 3S–4S LiPo | 5 V BEC for FC, 3.3 V for sensors |

---

## Wiring

### MPU-9250 → HUZZAH32 Feather (SPI / VSPI)

| MPU-9250 Pin | Feather GPIO | Feather Label | Notes |
|---|---|---|---|
| VCC | 3.3 V | 3V | **3.3 V only — NOT 5 V tolerant** |
| GND | GND | GND | |
| SCL / SCLK | GPIO 5 | SCK | SPI clock |
| SDA / MOSI | GPIO 18 | MO | SPI data out |
| AD0 / MISO | GPIO 19 | MI | SPI data in |
| NCS / CS | GPIO 33 | 33 | Chip select |
| INT | GPIO 27 | 27 | Optional — not used in firmware |

### BMP280 → HUZZAH32 Feather (I²C)

| BMP280 Pin | Feather GPIO | Notes |
|---|---|---|
| VCC | 3.3 V | 3.3 V only |
| GND | GND | Common ground |
| SDA / SDI | GPIO 21 | I²C data |
| SCL / SCK | GPIO 22 | I²C clock |
| CSB | 3.3 V | Pull high → forces I²C mode |
| SDO | GND | Sets I²C address to 0x76 (3.3 V → 0x77) |

> The BMP280 uses I²C, keeping the SPI bus exclusively for the MPU-9250.

### GY-GPS6MV2 (NEO-6M) → HUZZAH32 Feather (UART1)

| GPS Pin | Feather GPIO | Notes |
|---|---|---|
| VCC | 3.3 V | Module has onboard regulator, accepts 3.3–5 V |
| GND | GND | Common ground |
| TXD | GPIO 13 | GPS → ESP32 UART1 RX |
| RXD | GPIO 17 | ESP32 UART1 TX → GPS (optional, read-only operation) |

> Cold GPS fix typically takes **30–90 seconds outdoors** with a clear sky view. The blue LED on the module blinks once per second when a fix is acquired.

> **GPIO 17 note:** This pin is shared with the iBUS TX line (unused). Both are TX-only sides that are never driven — no conflict.

### FS-iA6B Receiver → HUZZAH32 Feather (UART2)

| FS-iA6B Pin | Feather GPIO | Notes |
|---|---|---|
| iBUS (S.BUS/iBUS port) | GPIO 16 | UART2 RX, 115200 baud, single-wire |
| VCC | 5 V (from BEC) | 4.0–6.5 V DC |
| GND | GND | Shared ground with FC |

> Use the **S.BUS/iBUS port** (small connector on the end of the receiver) — not the individual CH1–CH6 servo connectors.

### Motor ESCs → HUZZAH32 Feather (PWM)

| Motor | GPIO | Position | Rotation | Notes |
|---|---|---|---|---|
| Front-Left | GPIO 25 | FL | CCW | 33 Ω series resistor on signal wire |
| Front-Right | GPIO 15 | FR | CW | 33 Ω series resistor on signal wire |
| Rear-Left | GPIO 14 | RL | CW | 33 Ω series resistor on signal wire |
| Rear-Right | GPIO 32 | RR | CCW | 33 Ω series resistor on signal wire |

**ESC wiring (3-pin connector):**
- BLACK → GND (PDB GND pad)
- RED → 5 V (PDB BEC output)
- WHITE → 33 Ω → GPIO signal pin

> Standard PWM: 1000 µs = minimum / armed, 2000 µs = full throttle, at 50 Hz. ESC endpoint calibration (`motorEscCalibrate()`) must be done **once** before first use — see First-Time Setup.

---

## Repository Structure

```
quadcopter-fc/
│
├── RC_FlightController.ino         # Main sketch — integrates all submodules
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
│   ├── GPS/
│   │   ├── GPSSensor.h             # NEO-6M NMEA parser header
│   │   └── GPSSensor.cpp           # NEO-6M NMEA parser implementation
│   │
│   ├── iFly/
│   │   ├── FlySkyiBUS.h            # iBUS receiver driver header
│   │   └── FlySkyiBUS.cpp          # iBUS receiver driver implementation
│   │
│   ├── WiFiTelemetry/
│   │   ├── TelemetryWiFi.h         # Wi-Fi hotspot + HTTP server header
│   │   └── TelemetryWiFi.cpp       # Wi-Fi hotspot + HTTP server implementation
│   │
│   ├── MotorControl/
│   │   ├── MotorControl.h          # ESC PWM motor control header
│   │   └── MotorControl.cpp        # ESC PWM motor control implementation
│   │
│   └── ESP32Core/
│       ├── CPUUtilization.h        # Dual-core CPU load monitor header
│       └── CPUUtilization.cpp      # Dual-core CPU load monitor implementation
│
├── Simulation/
│   └── DroneGroundStation.html     # Browser-based ground station v2.3
│
├── LICENSE                         # GNU GPL v3
└── README.md                       # This file
```

---

## RC Switch Assignments (FS-i6X)

| Channel | FS-i6X Control | Function |
|---|---|---|
| CH1 | Right stick L/R | Roll |
| CH2 | Right stick U/D | Pitch |
| CH3 | Left stick U/D | Throttle |
| CH4 | Left stick L/R | Yaw |
| CH5 | VrA (knob) | Spare / PID tune |
| CH6 | VrB (knob) | Spare / PID tune |
| **CH7** | **SWA (2-pos)** | **ARM / DISARM** |
| **CH8** | **SWB (2-pos)** | **ANGLE ↔ ACRO flight mode** |
| **CH9** | **SWC (3-pos)** | **Accel orientation confirm during calibration** |
| **CH10** | **SWD (2-pos)** | **CALIBRATION trigger — flip UP while disarmed** |

> **Arming:** Flip SWA (CH7) UP. Disarm: Flip SWA DOWN. The drone will not arm while calibration is in progress.

---

## FreeRTOS Task Architecture

| Task | Core | Priority | Rate | Responsibility |
|---|---|---|---|---|
| `taskIMU` | 1 | 5 (highest) | 500 Hz / 2 ms | SPI burst read → Mahony AHRS → publish FlightState |
| `taskPID` | 1 | 4 | 500 Hz / 2 ms | Cascade PID loops → motor mix → ESC PWM |
| `taskRC` | 0 | 3 | 200 Hz / 5 ms | iBUS frame decode → arm / calibration edge detect |
| `taskBMP` | 0 | 1 | 20 Hz / 50 ms | BMP280 I²C read → pressure / temperature / altitude |
| `taskGPS` | 0 | 1 | 50 Hz / 20 ms | NMEA UART drain → parse GPRMC / GPGGA → fix data |
| `taskCPU` | 0 | 1 | 2 Hz / 500 ms | FreeRTOS idle-hook CPU load estimation |
| `taskWiFi` | 0 | 1 | Event-driven | HTTP server → telemetry / tune / log endpoints |
| `taskSerial` | 0 | 1 | 20 Hz / 50 ms | 1 Hz telemetry print, calibration progress |

**Synchronisation:** All tasks share a single `g_flightMutex` protecting the `FlightState` struct. A separate `g_tuneMutex` protects gain updates. `taskPID` is suspended (`vTaskSuspend`) for the entire calibration sequence and motors are zeroed before calibration begins.

---

## Ground Station (`DroneGroundStation.html`) — v2.3

A single-file browser application with **no server or install required**. Open the file locally in Chrome or Edge.

### Connection modes

**USB Serial** (Chrome / Edge only — Web Serial API):
Connect via USB, click **SERIAL**. Parses the 1 Hz telemetry line from `taskSerial`.

**Wi-Fi JSON polling:**
Connect your laptop to the `ESP32-DRONE` hotspot, open the HTML file, click **WIFI**. Polls `/telemetry` at 10 Hz and `/log` at 2 Hz.

**Simulation:**
Click **SIM** to run synthetic flight and GPS data — no hardware required for UI testing.

### HTTP endpoints

| Endpoint | Method | Description |
|---|---|---|
| `/telemetry` | GET | Full JSON state (attitude, RC, motors, BMP280, GPS, CPU, PID gains) |
| `/tune` | POST | Apply PID / Mahony gains (disarmed only). Body: JSON key-value pairs |
| `/log?since=N` | GET | Streaming calibration log lines since sequence N. Used by the log panel |

### UI panels

| Panel | Content |
|---|---|
| RC Inputs | Dual stick visualisers, channel bars, RC frame-rate gauge |
| Attitude Indicator | Animated artificial horizon, compass heading tape, Euler angles |
| IMU Plotter | Scrolling 300-sample chart — fused R/P/Y, raw gyro, raw accel |
| Motor RPM Gauges | Estimated RPM per motor (throttle × KV × Vbat) |
| Calibration Log | Colour-coded scrolling log (boot / cal / RC / tune / GPS / warn) |
| Telemetry Health | Parser status, tick rate, RC Hz, data age, live gain readback |
| Gain Tuner | Rate PID (roll/pitch/yaw), Angle PID, Mahony Kp/Ki — SET and APPLY ALL |
| GPS / Map | Live Leaflet map with flight track, fix badge, satellite dots, UTC time |

### WiFi credentials

| Field | Value |
|---|---|
| SSID | `ESP32-DRONE` |
| Password | `12345678` |
| IP address | `192.168.4.1` |

---

## Subsystems

### 1. IMU Driver (`Submodules/IMU/`)

Register-level SPI driver for the MPU-9250 (MPU-6500 accel/gyro die + AK8963 magnetometer die accessed through the MPU's internal I²C master).

- Gyro ±500 °/s, Accel ±8 g, DLPF 41 Hz
- AK8963 in continuous mode 2 (100 Hz, 16-bit) — SLV0 auto-burst reads 8 bytes (ST1 + 6 data + ST2) appended to each SPI transaction
- SLV4 used for all one-shot AK8963 writes (avoids corrupting the SLV0 read pipeline)
- Factory ASA sensitivity adjustment read from AK8963 Fuse ROM at boot
- Mahony quaternion state lives inside the class — accumulates correctly across calls
- Calibration stored in ESP32 NVS flash via `Preferences` — survives power cycles

**Key data structures:**

| Struct | Contents |
|---|---|
| `MPU_RawData` | 16-bit counts for all 9 axes + temperature + magOk flag |
| `MPU_SensorData` | Calibrated floats: ax/ay/az (g), gx/gy/gz (°/s), mx/my/mz (µT), temp (°C) |
| `MPU_Attitude` | roll / pitch / yaw (°), unit quaternion q0–q3 |
| `MPU_CalData` | 15 bias + scale values + AK8963 ASA + valid flag |

---

### 2. BMP280 Barometric Altimeter (`Submodules/BMP280/`)

Custom I²C driver — no external library required.

- Auto-detects I²C address 0x76 or 0x77 via `beginAuto()`
- `scanI2C()` debug helper prints all detected devices on the bus
- IIR filter x4, standby 125 ms, pressure oversampling x16, temperature oversampling x2
- Bosch 64-bit integer compensation formulas (datasheet §4.2.3)
- Altitude: `h = 44330 × (1 − (P / P₀)^0.19029)` — configurable sea-level QNH
- FreeRTOS mutex guards I²C reads — safe from any task

---

### 3. GPS Subsystem (`Submodules/GPS/`)

NMEA 0183 parser for the GY-GPS6MV2 (u-blox NEO-6M) module on UART1.

- Parses `$GPRMC` and `$GPGGA` sentences
- Validates NMEA checksum before accepting any sentence
- Provides: latitude, longitude, altitude MSL, speed (km/h + knots), true course, fix quality, satellite count, HDOP, UTC time and date
- 2-second fix timeout — marks data stale if no valid sentence received
- FreeRTOS mutex protects `GPSData` struct — safe from any task
- `taskGPS` drains UART at 50 Hz so no bytes are dropped between the GPS module's 1 Hz updates

---

### 4. RC Receiver (`Submodules/iFly/`)

Single-wire iBUS decoder for the FlySky FS-iA6B on UART2.

**iBUS frame (32 bytes):**
```
Byte 0    : 0x20  (frame length)
Byte 1    : 0x40  (command)
Bytes 2–29: 14 × 2 bytes little-endian (1000–2000 µs)
Bytes 30–31: Checksum = 0xFFFF − Σ(bytes 0..29)
Frame rate: ~7 ms ≈ 142 Hz
```

**Key API:**
```cpp
rcReceiver.begin(16, 17, 2);          // RX=GPIO16, TX=GPIO17 (unused), UART2
rcReceiver.update();                   // call in dedicated FreeRTOS task
RCCommand cmd = rcReceiver.getCommand();
// cmd.throttle  0.0–1.0
// cmd.roll / pitch / yaw  −1.0–+1.0  (deadband ±30 µs applied)
// cmd.mode      DISARMED / ANGLE / ACRO / FAILSAFE
// cmd.swdHigh   true when SWD (CH10) is UP — sole calibration trigger
```

**Failsafe:** No valid frame for 500 ms → `FlightMode::FAILSAFE`, motors cut. Re-arm requires cycling SWA low then high.

---

### 5. Wi-Fi Telemetry (`Submodules/WiFiTelemetry/`)

ESP32 Access Point + HTTP server. Serves `DroneGroundStation.html` data without any router.

- Callback pattern: `setTelemetryProvider()` and `setTuneHandler()` decouple the server from flight logic
- `pushLog(line)` adds a line to the 60-entry ring buffer served by `/log`
- CORS headers on all responses — works when the HTML is opened as a `file://` URL
- `TelemetryPacket` carries all flight state including GPS block (v2.3+)
- `/tune` rejects requests while armed; accepted values written to `TuningState` with a dirty flag; `taskIMU` applies them at the next loop boundary

---

### 6. Motor Control (`Submodules/MotorControl/`)

ESC PWM driver using the ESP32 LEDC peripheral. No analog calibration after ESC endpoint calibration is complete.

**PWM parameters:**

| Parameter | Value |
|---|---|
| Frequency | 50 Hz (20 ms period) |
| Resolution | 16-bit (0–65535 counts) |
| Minimum / armed | 1000 µs (3276 counts) |
| Full throttle | 2000 µs (6553 counts) |
| Arm pulse | 900 µs (below minimum) |

**Motor mixer (X-frame):**
```
FL = throttle + pitch + roll - yaw   (CCW)
FR = throttle - pitch - roll - yaw   (CW)
RL = throttle - pitch + roll + yaw   (CW)
RR = throttle + pitch - roll + yaw   (CCW)
```

**Sign conventions:** pitch+ = nose down, roll+ = right wing down, yaw+ = nose right (clockwise from above).

> **ESC calibration** (`motorEscCalibrate()`) must be done **once** per ESC before first use. The routine blocks and prints step-by-step instructions over Serial. Never call it with propellers attached.

---

### 7. Autonomous Calibration System

Triggered by **flipping SWD (CH10) UP** while SWA is DOWN (disarmed). No laptop required. The entire routine runs inside `taskIMU` on Core 1, with `taskPID` suspended and motors zeroed throughout.

#### Stage 1 — Gyroscope (~8 s, fully automatic)

1. Place drone flat and completely still.
2. 3 s settle delay, then 5 s averaging (~2500 samples).
3. Bias written directly to `cal.gx_b / gy_b / gz_b`.

#### Stage 2 — Accelerometer (~50 s, 6 orientations via SWC)

For each of the 6 positions (±X, ±Y, ±Z face up):
1. Serial prints which face to point upward.
2. Orient the drone accordingly.
3. **Flip SWC (CH9) UP** to confirm — 3 s hold, 1.5 s sampling.
4. **Flip SWC DOWN** to advance to the next position.
5. Bias = midpoint of opposite-pair averages. Scale = 1 g / half-range.

#### Stage 3 — Magnetometer (30 s timed sweep)

1. Rotate the drone through all axes (figure-8 motion) for 30 s.
2. Min/max tracked. Remaining time printed every 10 s.
3. Hard-iron bias = (max + min) / 2. Soft-iron scale = average span / per-axis span.

All results **auto-save to NVS flash** on completion. Calibration loads automatically on every subsequent boot.

---

### 8. AHRS — Mahony Complementary Filter

9-DOF sensor fusion producing roll, pitch, and yaw Euler angles and a unit quaternion.

- Quaternion state (`_q0`–`_q3`) lives inside the `MPU9250` class — accumulates correctly across calls
- Accelerometer normalisation guard prevents NaN during free-fall or cold start
- Magnetometer correction applied only when `_magValid == true` and mag norm > 0.1 µT — otherwise falls back to 6-DOF (no yaw corruption from bad mag data)
- `dt` clamped to [0.0001, 0.05] s to prevent filter blow-up on the first call or after a delay
- Tunable `mahonyKp` (default 2.0) and `mahonyKi` (default 0.005) — adjustable live via the ground station

**Axis convention (body frame, NED):**
X → nose (forward), Y → right wing, Z → down
Roll (+) = right wing down, Pitch (+) = nose up, Yaw = 0–360° from magnetic north

---

### 9. PID Flight Controller

Cascaded inner + outer PID running at 500 Hz on Core 1.

**ANGLE mode** (self-level):
- Outer loop: RC stick → desired angle → angle error → rate setpoint (pidAngleRoll / pidAnglePitch)
- Inner loop: rate setpoint → rate error → motor correction (pidRateRoll / pidRatePitch / pidRateYaw)

**ACRO mode** (rate-only):
- Inner loop only: RC stick → desired rate → rate error → motor correction

**Recommended starting gains:**

| Axis | Kp | Ki | Kd |
|---|---|---|---|
| Roll rate (inner) | 0.5 | 0.002 | 0.010 |
| Pitch rate (inner) | 0.5 | 0.002 | 0.010 |
| Yaw rate (inner) | 1.0 | 0.005 | 0.000 |
| Roll angle (outer) | 4.0 | 0.0 | 0.0 |
| Pitch angle (outer) | 4.0 | 0.0 | 0.0 |

Tune the **inner (rate) loop first** in ACRO mode before enabling the outer angle loop.

---

### 10. CPU Utilisation Monitor (`Submodules/ESP32Core/`)

Lightweight dual-core load estimation via FreeRTOS idle hooks.

- Registers `idleHookCore0` and `idleHookCore1` via `esp_register_freertos_idle_hook_for_cpu()`
- Counts idle-hook invocations per 1-second window
- Highest observed idle count becomes the local 0% load baseline — self-calibrating
- Output: `core0_pct`, `core1_pct`, `valid` — displayed in the ground station CPU bars
- Accuracy improves after the first few seconds of runtime

---

## Build & Flash

### Requirements

- Arduino IDE 2.x or PlatformIO
- ESP32 Arduino Core ≥ 2.0.0 — **Adafruit HUZZAH32 board definition**
- All libraries below are included in the ESP32 Arduino Core — **no external installs required**:
  - `Preferences` — NVS flash calibration storage
  - `Wire` — I²C for BMP280
  - `WiFi` + `WebServer` — Wi-Fi telemetry
  - `HardwareSerial` — UART2 for iBUS, UART1 for GPS

### Arduino IDE Setup

1. **Tools → Board** → `Adafruit ESP32 Feather`
2. **Tools → Port** → select your USB-serial port
3. **Tools → Upload Speed** → `921600`
4. Open `RC_FlightController.ino`
5. Place all Submodule `.h` / `.cpp` files in the same sketch folder, or configure include paths in PlatformIO
6. Click **Upload**

> Use **Adafruit ESP32 Feather** — not "ESP32 Dev Module". The flash partition map and default pin assignments differ.

---

## First-Time Setup

### Step 1 — ESC Calibration (once per ESC, props OFF)

1. Flash firmware
2. Uncomment `motorEscCalibrate()` in `setup()` (or call it temporarily)
3. Open Serial Monitor at **115200 baud** and follow the on-screen prompts
4. Re-comment the calibration call after completion — ESC endpoints are saved in ESC EEPROM

### Step 2 — Sensor Calibration (first flight and after any rebuild)

1. Flash normal firmware (calibration call removed)
2. Power the drone with props removed
3. Wait ~5 s for IMU warm-up
4. **Flip SWD (CH10) UP** while SWA is DOWN (disarmed) to start the autonomous calibration sequence
5. Follow Serial prompts: gyro (automatic) → accel (6 positions via SWC) → mag (rotate 30 s)
6. Calibration auto-saves to NVS flash on completion
7. On all subsequent boots, calibration loads automatically — no action needed

### Step 3 — Ground Station

1. Connect your laptop's WiFi to `ESP32-DRONE` / password `12345678`
2. Open `Simulation/DroneGroundStation.html` in Chrome or Edge
3. Click **WIFI** — telemetry, GPS map, and calibration log appear immediately
4. For USB serial monitoring: click **SERIAL** and select the ESP32 COM port

> Alternatively, connect via **Serial** if you do not have WiFi available.

---

## Safety

- **Always remove propellers** during development, calibration, and ESC setup
- Never arm the drone over USB power alone — use a fully charged LiPo with props off
- Verify motor spin direction and mixer sign against the X-frame layout before first flight
- The FAILSAFE state cuts all motor outputs within 500 ms of RC signal loss
- Gain updates via `/tune` are rejected while the drone reports `armed = true`
- Set a conservative throttle limit (< 50 %) for maiden flights
- Cold GPS fix takes 30–90 s outdoors — do not fly until the fix indicator is green

---

## License

GNU General Public License v3.0 — see `LICENSE` for full terms.

---

## Acknowledgements

- **Platform:** Adafruit HUZZAH32 Feather / ESP32-WROOM-32E
- **IMU:** InvenSense / TDK MPU-9250 + AK8963
- **Barometer:** Bosch BMP280
- **GPS:** u-blox NEO-6M (GY-GPS6MV2 breakout)
- **AHRS:** Mahony complementary filter
- **RC:** FlySky FS-i6X + FS-iA6B iBUS protocol
- **RTOS:** FreeRTOS (integrated into ESP32 Arduino Core)
- **Maps:** Leaflet.js + OpenStreetMap contributors
