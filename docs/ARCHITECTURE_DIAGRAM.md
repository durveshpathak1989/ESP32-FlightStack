# ESP32-FlightStack — Architecture Diagrams

Code-verified against `RC_FlightController.ino` @ v6.1.0 (2893 lines, 8 FreeRTOS tasks).
Mermaid renders natively on GitHub.

## 1. System Overview

```mermaid
flowchart TB
    subgraph PILOT["Pilot"]
        TX["FlySky FS-i6X<br/>10ch transmitter"]
    end

    subgraph AIR["Airborne — F450 Quad"]
        subgraph FC["ESP32-WROOM-32E (HUZZAH32)"]
            CORE1["Core 1<br/>taskControl · 400 Hz · prio 5"]
            CORE0["Core 0<br/>7 support tasks"]
        end
        IMU["MPU-9250<br/>SPI 400 Hz"]
        BARO["BMP280<br/>I2C"]
        TOF["VL53L4CX ToF<br/>optional"]
        GPS["NEO-6M GPS<br/>UART1"]
        ESCS["4x PWM ESC<br/>LEDC 50 Hz"]
        MOTORS["4x 2212 motors<br/>X-frame"]
        BAT["3S LiPo<br/>GPIO34 divider"]
    end

    subgraph LINK["Wi-Fi Link"]
        AP["SoftAP 'ESP32-DRONE'<br/>192.168.4.1"]
    end

    subgraph GROUND["Ground Station Laptop"]
        GCS["DroneGCS.html<br/>dashboard · tuning · OTA"]
        OSM["Local map server<br/>python http.server :8080"]
        MAPF["Area.osm extract"]
    end

    TX -->|"iBUS 142 Hz<br/>UART2"| CORE0
    CORE1 <--> IMU
    CORE0 <--> BARO
    CORE0 <--> TOF
    CORE0 <--> GPS
    CORE1 -->|"PWM"| ESCS --> MOTORS
    BAT -->|"ADC<br/>500 ms"| FC
    CORE0 === AP
    AP -.->|"HTTP JSON"| GCS
    GCS -.->|"/tune POST<br/>disarmed only"| AP
    GCS -.->|".bin OTA upload<br/>safety-gated"| AP
    MAPF --> OSM -.->|"map tiles"| GCS
```

## 2. Firmware Task Architecture (dual-core FreeRTOS)

```mermaid
flowchart LR
    subgraph C0["Core 0 — communication & slow sensors"]
        RC["taskRC<br/>prio 3 · 200 Hz<br/>iBUS parse · arm/cal triggers"]
        GPS["taskGPS<br/>prio 1 · 50 Hz<br/>NMEA drain"]
        BMP["taskBMP<br/>prio 1 · 20 Hz<br/>altitude · vspeed"]
        TOF["taskToF<br/>prio 1 · optional"]
        SER["taskSerial<br/>prio 1 · 20 Hz<br/>status · PID trace"]
        WIF["taskWiFi<br/>prio 1 · event-driven<br/>HTTP · telemetry · OTA"]
        CPUM["taskCPU<br/>prio 1 · 2 Hz<br/>idle-hook utilization"]
    end

    subgraph C1["Core 1 — hard real-time only"]
        CTRL["taskControl<br/>prio 5 · 400 Hz esp_timer ISR wake<br/>IMU → filter → AHRS → PID → motors"]
    end

    IBUS["FS-iA6B"] --> RC
    RC -->|"cmd snapshot<br/>(atomic)"| CTRL
    GPS --> CTRL
    BMP --> CTRL
    TOF --> CTRL
    WIF -->|"tune req<br/>seq handshake<br/>rejected while armed"| CTRL
    CTRL -->|"state JSON<br/>via provider fns"| WIF --> AP2["Wi-Fi AP"]
    CTRL --> LOG["FlightLogger<br/>100 Hz ring buffer<br/>100 rows x ~320 B"]
    CTRL --> TMG["Timing stats<br/>Welford jitter<br/>800-sample ring"]
    CTRL --> SPEC["SpectrumAnalyzer<br/>FFT → dynamic notch<br/>45–170 Hz · 10 Hz update"]
```

## 3. 400 Hz Control Pipeline (inside taskControl)

```mermaid
flowchart LR
    IMU["IMU read<br/>SPI gyro+accel+mag"] --> SN["Static notch biquad<br/>all 6 axes"]
    SN --> DN["Dynamic notch<br/>FFT-tracked centre"]
    DN --> AHRS["AHRS<br/>runtime-selectable:<br/>EKF (default) | Mahony | Madgwick"]
    AHRS --> LZ["Level-zero trim<br/>offset applied"]
    RCIN["RC commands<br/>arm · mode · sticks"] --> MODE{"Flight mode"}
    MODE -->|"ANGLE"| AOUT["Angle PID<br/>(outer loop)"]
    MODE -->|"ACRO"| ROUT["Rate PID<br/>(inner loop)"]
    MODE -->|"yaw centred"| YH["Yaw heading hold"]
    AOUT --> ROUT
    YH --> MIX
    ROUT --> MIX["X-mixer<br/>FL FR RL RR"]
    MIX --> LIM["Idle · expo · slew limits<br/>saturation guard"]
    LIM --> PWM["LEDC PWM → ESCs"]
    CAL["Calibration state machine<br/>(runs inside taskControl:<br/>flight loop blocked,<br/>motors inhibited)"]
    CAL -.->|"on completion<br/>(disarmed only)"| NVS[("NVS<br/>persistent cal")]
```

## 4. Software Layering (AUTOSAR-inspired, vendored)

```mermaid
flowchart TD
    INO[".ino sketch<br/>FreeRTOS tasks · composition root"]
    APP["App/ — application algorithms<br/>Control: PidController · Filters: NotchFilter<br/>Estimation: EKF · AHRS · Mahony · Madgwick"]
    SVC["Services/<br/>Com: TelemetryWiFi+OTA · Calibration: CalManager<br/>SignalProcessing: FFT · Diagnostics: Logger+DebugConfig+CPU"]
    ECU["EcuAbstraction/<br/>Sensors: IMU·BMP280·GPS·ToF<br/>Actuators: MotorControl · Receiver: iFly · Power: BatteryMonitor"]
    MCAL["Mcal/<br/>ESP32 Arduino Core API boundary<br/>(SPI · Wire · HardwareSerial · LEDC · ADC · esp_timer · Preferences)"]
    CORE["Core/ + Cfg/<br/>FlightTypes · Ports · ImuTypes · FlightConfig"]
    RULE["Dependency rule: downward only.<br/>Core depends on nothing platform-specific."]
    INO --> APP
    INO --> SVC
    INO --> ECU
    APP --> CORE
    SVC --> CORE
    ECU --> MCAL
    ECU --> CORE
    MCAL --- HW["Hardware"]
    CORE --- RULE
```

Vendored note: all former `Submodules/*` git repositories are now plain
directories inside this repo, so layer boundaries are enforceable in one
tree. Original module repos remain on GitHub unchanged.
See `docs/LAYERS.md` for contracts and sanctioned exceptions.

## 5. Ground-Station Data Flow

```mermaid
flowchart LR
    FW["Firmware providers<br/>setTelemetryProvider() etc."] --> SRV["WiFiTelemetry HTTP server"]
    SRV -->|"GET"| E1["/telemetry /log /timing /timing/csv<br/>/spectrum /identity"]
    E1 --> GCS2["Browser GCS<br/>attitude · gauges · PID traces<br/>battery · GPS · jitter chart"]
    GCS2 -->|"POST /tune"| GATE{"armed?"}
    GATE -->|"yes"| REJ["rejected"]
    GATE -->|"no"| APPLY["apply_seq handshake →<br/>PID/AHRS/notch objects"]
    GCS2 -->|"POST /flightlog/reset<br/>GET /flightlog/csv"| FLIP["freeze-and-stream log"]
    GCS2 -->|"POST /update"| OGATE{"disarmed ∧<br/>throttle ≤ cut ∧<br/>motors ≤ 0.001"}
    OGATE -->|"yes"| FLASH["OTA second app slot"]
```
