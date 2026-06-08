# ESP32 Quadcopter Flight Controller

A full-featured quadcopter flight-controller project built on the **Adafruit HUZZAH32 Feather / ESP32-WROOM-32E**. The system integrates an MPU-9250 IMU, Mahony AHRS, cascaded PID control, FlySky iBUS RC input, PWM ESC output, BMP280 barometer, NEO-6M GPS, Wi-Fi telemetry, browser-based ground station, live runtime tuning, onboard logging, **offline OSM map support**, and **browser-based OTA firmware upload**.

The project goal is not only to make a quadcopter fly, but to use it as a systems-engineering platform for studying low-cost multicore embedded flight-control integration, timing, telemetry interference, calibration, and field-test workflow.

---

## Current Capability Summary

| Area | Current capability |
|---|---|
| MCU | Adafruit HUZZAH32 Feather / ESP32-WROOM-32E |
| RTOS | Dual-core FreeRTOS task architecture |
| IMU | MPU-9250 over SPI / VSPI |
| AHRS | Mahony quaternion filter with live Kp/Ki tuning |
| RC | FlySky FS-i6X + FS-iA6B iBUS receiver |
| Control | ANGLE mode and ACRO mode with cascaded PID loops |
| Motors | 4-motor X-frame PWM ESC output |
| Barometer | BMP280 over I²C |
| GPS | u-blox NEO-6M / GY-GPS6MV2 on UART1 |
| Telemetry | ESP32 Wi-Fi AP + HTTP JSON endpoints |
| GCS | Browser HTML dashboard with flight, sensors, motors, GPS, tune, config, OTA, and offline map workflow |
| Tuning | Runtime tuning through `/tune`, rejected while armed |
| OTA | Firmware `.bin` upload through browser to `/update`, gated by disarmed/motors-off safety checks |
| Offline maps | Local `.osm` file can be loaded by the GCS from the laptop using a local web server |
| Logging | Browser CSV logging and onboard high-speed log download endpoint |

---

## Repository Structure

```text
Test_Quad/
├── RC_FlightController.ino              # Main sketch and FreeRTOS task integration
├── Submodules/
│   ├── IMU/                             # MPU9250 driver and AHRS support
│   ├── BMP280/                          # BMP280 I²C driver
│   ├── GPS/                             # NEO-6M parser
│   ├── iFly/                            # FlySky iBUS driver
│   ├── WiFiTelemetry/                   # Wi-Fi AP, HTTP endpoints, tuning, OTA
│   ├── MotorControl/                    # PWM ESC output
│   └── ESP32Core/                       # CPU utilization monitor
├── Simulation/                          # Browser ground-station HTML files
├── Maps/                                # Optional local OSM files, ignored if too large
├── docs/
│   └── Offline_OSM_Map.md               # Offline map workflow
├── LICENSE
└── README.md
```

---

## Flight Modes

### ANGLE Mode

Self-level mode. RC roll and pitch sticks command target lean angle. When sticks return to center, target roll and pitch return to 0 degrees.

```text
RC stick → target angle → angle PID → target rate → rate PID → mixer → motors
```

Use this mode for early hover testing after the rate loop is stable.

### ACRO Mode

Rate mode. RC roll and pitch sticks command rotational rate in deg/s. The quad does **not** self-level when sticks are centered.

```text
RC stick → target rate → rate PID → mixer → motors
```

Tune the inner rate loop in ACRO before increasing ANGLE-mode authority.

### FAILSAFE

If the RC link is lost, the controller enters failsafe and cuts motor outputs. Re-arming requires the arm switch to be cycled back to safe first.

---

## Wi-Fi Ground Station

The browser GCS supports:

- USB serial telemetry through Web Serial
- Wi-Fi telemetry polling from the ESP32 AP
- Artificial horizon and compass
- RC stick visualizers and RC frame-rate gauge
- IMU plotter
- Motor output and estimated RPM gauges
- GPS map / GPS status panel
- Runtime tuning panel
- System log panel
- Configuration panel
- Onboard log download
- OTA firmware upload panel
- Offline OSM map display for field operation without internet

### Wi-Fi credentials

| Field | Value |
|---|---|
| SSID | `ESP32-DRONE` |
| Password | `12345678` |
| Default IP | `192.168.4.1` |

---

## HTTP Endpoints

| Endpoint | Method | Description |
|---|---|---|
| `/telemetry` | GET | Full JSON state: attitude, RC, motors, sensors, CPU, GPS, tune readback |
| `/tune` | POST | Runtime tuning update. Rejected while armed |
| `/log?since=N` | GET | Ring-buffer system/calibration log |
| `/flightlog/csv` | GET | Download onboard high-speed flight log, if enabled |
| `/timing` | GET | IMU/control-loop timing stats, if enabled |
| `/timing/reset` | POST | Reset timing stats, if enabled |
| `/timing/csv` | GET | Download timing stats CSV, if enabled |
| `/update` | GET | OTA upload web page served by ESP32 |
| `/update` | POST | Firmware `.bin` upload for OTA update |

---

## Runtime Tuning Levers

The GCS can push the following values through `/tune`. Firmware should reject tuning while armed.

### Pilot command limits

```cpp
max_angle_deg
max_rate_dps
max_pitch_rate_dps
yaw_max_rate_dps
yaw_deadband
```

### PID output authority

```cpp
roll_output_limit
pitch_output_limit
yaw_output_limit
```

### Throttle shaping

```cpp
throttle_expo
throttle_up_rate_per_sec
throttle_down_rate_per_sec
motor_idle
motor_max
throttle_cut
idle_ramp_end
```

### PID gains

```cpp
pid_roll_kp / pid_roll_ki / pid_roll_kd
pid_pitch_kp / pid_pitch_ki / pid_pitch_kd
pid_yaw_kp / pid_yaw_ki / pid_yaw_kd
pid_angle_roll_kp / pid_angle_roll_ki / pid_angle_roll_kd
pid_angle_pitch_kp / pid_angle_pitch_ki / pid_angle_pitch_kd
pid_angle_yaw_kp
```

### AHRS gains

```cpp
mahony_kp
mahony_ki
```

Recommended tuning order:

1. Verify motor order, prop direction, IMU signs, and mixer signs.
2. Tune inner rate loop in ACRO mode.
3. Add ANGLE outer-loop gain gradually.
4. Keep angle `I` at zero initially.
5. Use low max angle, conservative motor max, and props off for bench tests.

---

## Offline OSM Map Support

When the laptop is connected to the ESP32 access point, it normally has no internet. Online Leaflet/OpenStreetMap tiles may not load. The GCS supports a local offline `.osm` file workflow.

Recommended folder layout:

```text
DroneGCS/
├── DroneGCS_Apple_OSM_offline.html
└── ColumbusMap.osm
```

Run a local server from the GCS folder:

```bash
python -m http.server 8080
```

Then open:

```text
http://localhost:8080/DroneGCS_Apple_OSM_offline.html
```

With this setup:

```text
GCS HTML          -> loaded from laptop localhost
ColumbusMap.osm  -> loaded from laptop localhost
Telemetry         -> loaded from ESP32 at http://192.168.4.1/telemetry
Tune endpoint     -> http://192.168.4.1/tune
OTA endpoint      -> http://192.168.4.1/update
```

This allows the GPS map to work while the laptop is connected to the ESP32 Wi-Fi network.

---

## OTA Firmware Update

OTA allows future firmware updates over Wi-Fi after the first OTA-capable firmware has been flashed by USB.

### Required partition scheme

Use an OTA-capable partition scheme in Arduino IDE, for example:

```text
Tools → Partition Scheme → Minimal SPIFFS with 1.9MB APP with OTA
```

Do **not** choose a `No OTA` or `Huge APP` partition scheme for browser OTA updates.

### Getting the `.bin` from Arduino IDE

1. Select the ESP32 board and OTA-capable partition scheme.
2. Compile or upload once by USB.
3. Use **Sketch → Export Compiled Binary**.
4. Use **Sketch → Show Sketch Folder**.
5. Open the generated build folder.
6. Upload only the application binary:

```text
RC_FlightController.ino.bin
```

Do **not** upload these through OTA:

```text
boot_app0.bin
RC_FlightController.ino.bootloader.bin
RC_FlightController.ino.partitions.bin
RC_FlightController.ino.merged.bin
RC_FlightController.ino.elf
RC_FlightController.ino.map
```

### OTA safety gate

OTA upload is intended for bench use only. The firmware safety gate should allow OTA only when:

```text
armed == false
throttle is below throttle_cut
all motor outputs are zero or near zero
```

---

## Safety Rules

- Remove propellers for all bench testing, flashing, OTA, calibration, ESC setup, and PID tuning.
- Never OTA update while armed.
- Verify motor order and spin direction before any throttle test.
- Verify mixer sign by hand: tilting the frame should increase the motors that oppose the tilt.
- Use conservative throttle limits and small props during early hover testing.
- GPS map display does not imply GPS-assisted flight control; current GPS is telemetry/status only.
- Offline map display is situational awareness only; do not use it as an autonomous navigation authority.
- Keep USB as a recovery path.

---

## License

GNU General Public License v3.0 — see `LICENSE` for full terms.
