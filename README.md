# ESP32 Quadcopter Flight Controller

A full-featured quadcopter flight controller built on the **Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)**, featuring a multi-task FreeRTOS architecture, MPU-9250 9-axis IMU, Mahony AHRS sensor fusion, and DShot ESC motor control.

---

## Project Overview

This project implements a complete quadcopter flight controller from the ground up in C++ using the Arduino/ESP32 toolchain. The firmware is structured as a set of concurrent FreeRTOS tasks that handle sensor acquisition, attitude estimation, PID control, motor output, and telemetry in parallel across the ESP32's dual cores.

---

## Hardware Platform

| Component | Part | Notes |
|-----------|------|-------|
| Microcontroller | Adafruit HUZZAH32 Feather (ESP32-WROOM-32E) | Dual-core 240 MHz, Wi-Fi + BT |
| IMU | MPU-9250 (HiLetgo breakout) | Gyro + Accel + Mag via SPI |
| ESC Protocol | DShot300 / DShot600 | Via ESP32 RMT peripheral |
| Power | 3S–4S LiPo | 5 V BEC for FC, 3.3 V for sensors |

### Wiring — MPU-9250 to HUZZAH32 Feather

| MPU-9250 Pin | Feather GPIO | Feather Label |
|---|---|---|
| VCC | 3.3 V | 3V |
| GND | GND | GND |
| SCL/SCLK | GPIO 5 | SCK |
| SDA/MOSI | GPIO 18 | MO |
| AD0/MISO | GPIO 19 | MI |
| NCS | GPIO 33 | 33 |
| INT | GPIO 26 | 26 |

> **Warning:** Use 3.3 V only — the MPU-9250 is NOT 5 V tolerant.

---

## Repository Structure

```
quadcopter-fc/
│
├── MPU9250/                        # Standalone IMU library (SPI, non-FreeRTOS)
│   ├── MPU9250.h
│   ├── MPU9250.cpp
│   └── Drone_IMU_ESP32.ino
│
├── MPU9250_FreeRTOS/               # FreeRTOS-based flight controller
│   ├── MPU9250.h
│   ├── MPU9250.cpp
│   └── Drone_IMU_FreeRTOS.ino
│
└── README.md                       # This file
```

---

## Subsystems

The firmware is divided into the following subsystems, each mapped to one or more FreeRTOS tasks:

---

### 1. IMU Driver (`MPU9250.h` / `MPU9250.cpp`)

Provides low-level register access to the MPU-9250 over **SPI (VSPI)** on the HUZZAH32 Feather.

**Responsibilities:**
- Full register-level initialisation of the MPU-6500 (gyro + accel) and AK8963 (magnetometer) dies
- Configurable full-scale ranges: Gyro (±250–2000 °/s), Accel (±2–16 g)
- Configurable DLPF (10–184 Hz cutoff) for vibration rejection
- AK8963 magnetometer read via I2C master mode (SLV4 write / SLV0 burst read)
- NVS flash storage and retrieval of calibration data

**Key classes/files:**
- `MPU9250` — driver class
- `IMURawData` — struct: raw 16-bit counts for all 9 axes
- `IMUScaledData` — struct: physical units (g, °/s, µT)
- `IMUCalibration` — struct: 15-value bias + scale coefficients

---

### 2. Sensor Calibration Subsystem

Interactive calibration wizard accessed via the Serial monitor (`C` command).

**Three-stage routine:**
| Stage | Sensor | Method | Duration |
|-------|--------|--------|----------|
| 1 | Gyroscope | Static average (drone flat, still) | ~5 s |
| 2 | Accelerometer | 6-orientation average (±X, ±Y, ±Z) | ~60 s |
| 3 | Magnetometer | Min/max sweep (rotate all axes) | 20 s |

**Outputs:** Hard-iron bias (mx/my/mz offset), soft-iron scale, gyro zero-rate offset, accel 6-point bias and scale. All values persist in **NVS flash** via the `save()` / `load()` API.

---

### 3. AHRS — Attitude & Heading Reference System

Implements the **Mahony complementary filter** to fuse gyro, accelerometer, and magnetometer data into Roll / Pitch / Yaw Euler angles.

**Algorithm highlights:**
- Proportional-integral correction using accel + mag reference vectors
- Quaternion integration at 500–1000 Hz
- Configurable `Kp` (2.0–10.0) and `Ki` (0.001–0.01)
- Output: `attitude.roll`, `attitude.pitch`, `attitude.yaw` in degrees

**Axis convention (body frame, NED):**
- X → nose (forward), Y → right, Z → down
- Roll (+) = right wing down, Pitch (+) = nose up, Yaw (+) = clockwise from above

---

### 4. FreeRTOS Task Scheduler

The firmware runs as a set of concurrent tasks pinned to specific ESP32 cores:

| Task | Core | Priority | Rate | Responsibility |
|------|------|----------|------|----------------|
| `taskIMU` | Core 1 | High (5) | 500–1000 Hz | SPI burst read → AHRS update → shared data |
| `taskPID` | Core 1 | High (4) | 500 Hz | Inner + outer PID loop → motor mix |
| `taskMotor` | Core 1 | High (3) | 400 Hz | DShot frame generation via RMT |
| `taskSerial` | Core 0 | Low (1) | 50 Hz | Telemetry output, serial command menu |
| `taskRC` | Core 0 | Medium (2) | 100 Hz | RC receiver decode (SBUS / PPM) |

**Synchronisation primitives:**
- `SemaphoreHandle_t` mutex — protects the shared `IMUScaledData` struct
- `xQueueHandle` — passes PID output to motor task
- `vTaskSuspend` / `vTaskResume` — grants exclusive SPI access during calibration

---

### 5. PID Flight Controller

Implements a **cascaded (inner + outer) PID** loop for stable attitude control.

**Outer loop (angle mode) — 100–250 Hz:**
- Setpoint: RC stick → desired angle (°)
- Feedback: `attitude.roll` / `attitude.pitch` / `attitude.yaw`
- Output: desired angular rate (°/s) → inner loop setpoint

**Inner loop (rate mode) — 500–1000 Hz:**
- Setpoint: desired rate from outer loop (or direct stick in acro mode)
- Feedback: `sensorData.gx_dps` / `gy_dps` / `gz_dps`
- Output: motor correction values

**Features:**
- Independent P/I/D gains per axis
- Anti-windup integrator clamp
- Derivative-on-measurement (avoids setpoint-kick)
- Runtime tuning via Serial commands

---

### 6. Motor Output — DShot ESC Protocol

Controls four brushless motors via the **ESP32 RMT peripheral** for precise digital timing.

| Motor | GPIO | Position |
|-------|------|----------|
| Motor 1 | GPIO 25 | Front-Left |
| Motor 2 | GPIO 26 | Front-Right |
| Motor 3 | GPIO 27 | Rear-Left |
| Motor 4 | GPIO 14 | Rear-Right |

**Motor mix (+ quad geometry):**
```
motorFL = throttle + rollOut - pitchOut - yawOut
motorFR = throttle - rollOut - pitchOut + yawOut
motorRL = throttle + rollOut + pitchOut + yawOut
motorRR = throttle - rollOut + pitchOut - yawOut
```

**DShot throttle range:** 48 (min / arm) to 2047 (full throttle). Values 0–47 are reserved ESC commands (arm, beep, save settings, reverse direction).

**Library:** `DShotRMT` by derdoktor667 — supports DSHOT150/300/600/1200 and bi-directional DShot telemetry (RPM, temperature, voltage).

---

### 7. RC Receiver Interface

Decodes pilot input from the RC receiver.

**Supported protocols:**
- **SBUS** — serial, inverted UART at 100 kbps (Futaba, FrSky)
- **PPM** — pulse-position modulation via GPIO interrupt

**Channels decoded:**
| Channel | Function |
|---------|----------|
| CH1 | Roll |
| CH2 | Pitch |
| CH3 | Throttle |
| CH4 | Yaw |
| CH5 | Flight mode (Angle / Acro) |
| CH6 | Arm / Disarm |

**Safety:** Motors are disarmed on RC signal loss (failsafe). Re-arming requires throttle-low + arm switch cycle.

---

### 8. Telemetry & Serial Debug Interface

Provides real-time monitoring and in-field tuning via the USB serial port (115200 baud).

**Serial command menu:**

| Command | Action |
|---------|--------|
| `C` | Launch full calibration wizard (Gyro → Accel → Mag) |
| `S` | Save calibration to NVS flash |
| `L` | Load calibration from NVS flash |
| `V` | Verify all sensors (WHO_AM_I + self-test) |
| `D` | Dump 10 raw EXT_SENS_DATA samples + SLV0 config |
| `P` | Print current Roll / Pitch / Yaw + sensor values |
| `R` | Reset / reboot |

**Telemetry output fields (50 Hz):**
- Attitude: Roll, Pitch, Yaw (°)
- Rates: Gx, Gy, Gz (°/s)
- Acceleration: Ax, Ay, Az (g)
- Magnetometer: Mx, My, Mz (µT)
- Motor outputs: M1–M4 (DShot throttle value)
- Loop timing: IMU dt (µs)

---

## Build & Flash

### Requirements

- Arduino IDE 2.x or PlatformIO
- ESP32 Arduino Core ≥ 2.0.0 (Adafruit HUZZAH32 board definition)
- Libraries:
  - `DShotRMT` (install via Library Manager)
  - `Preferences` (built into ESP32 Arduino Core — for NVS flash)

### Arduino IDE Setup

1. **Tools → Board** → Select `Adafruit ESP32 Feather`
2. **Tools → Port** → Select your USB-serial port
3. **Tools → Upload Speed** → 921600
4. Open `MPU9250_FreeRTOS/Drone_IMU_FreeRTOS.ino`
5. Click **Upload**

> Use the **Adafruit ESP32 Feather** board definition — NOT "ESP32 Dev Module". The flash partition map and pin assignments differ.

---

## First-Time Setup

1. Flash the firmware
2. Open Serial Monitor at **115200 baud**
3. Power the drone (no props!) and let the IMU warm up for **30 seconds**
4. Send `C` to run the full calibration wizard
5. Follow the on-screen prompts for each calibration stage
6. Send `V` to verify all sensors pass
7. Send `S` to save calibration to flash
8. Calibration loads automatically on every subsequent boot

> **Re-calibrate** if: motors are repositioned, ESC wiring changes, or the drone is rebuilt. Motor magnets affect the magnetometer.

---

## Recommended PID Starting Points

| Axis | P | I | D |
|------|---|---|---|
| Roll Rate | 0.5 | 0.002 | 0.01 |
| Pitch Rate | 0.5 | 0.002 | 0.01 |
| Yaw Rate | 1.0 | 0.005 | 0.00 |
| Roll Angle | 4.0 | 0.0 | 0.0 |
| Pitch Angle | 4.0 | 0.0 | 0.0 |

Tune the **inner (rate) loop first** before enabling the outer (angle) loop.

---

## Safety Notice

- Always remove propellers during development and calibration
- Never arm the drone over USB power alone — use a battery with props off
- Verify motor spin direction and mixer sign before first flight
- Set a conservative throttle limit (< 50%) for maiden flights

---

## License

MIT License — free to use, modify, and distribute with attribution.

---

## Author & Acknowledgements

- **Platform:** Adafruit HUZZAH32 Feather / ESP32-WROOM-32E
- **IMU:** InvenSense / TDK MPU-9250
- **AHRS:** Mahony filter algorithm
- **ESC:** DShotRMT library by derdoktor667
- **RTOS:** FreeRTOS (integrated into ESP32 Arduino Core)
