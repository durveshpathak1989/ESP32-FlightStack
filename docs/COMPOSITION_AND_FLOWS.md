# Architecture V4 Composition and Runtime Flows

## Static composition

```mermaid
flowchart TB
    MAIN["Board composition root (.ino)"]
    APP["Flight application / state machine"]
    CFG["TuningState + FlightConfig"]
    CTRL["Controller SWCs"]
    EST["Estimator SWC"]
    PORTS["Core ports"]
    ESP["ESP32 platform services"]
    IMU["Selected IMU adapter"]
    RX["iBUS receiver adapter"]
    MOT["Motor adapter"]
    NVS["PreferencesTuningStore"]
    WIFI["Wi-Fi telemetry adapter"]

    MAIN --> APP
    MAIN --> ESP
    APP --> CFG
    APP --> CTRL
    APP --> EST
    APP --> PORTS
    ESP -.implements.-> PORTS
    ESP --> IMU
    ESP --> RX
    ESP --> MOT
    ESP --> NVS
    ESP --> WIFI
```

Solid arrows are allowed dependencies. Dashed arrows represent port implementation.
Core/Application must never point back to ESP32 or concrete drivers.

## Boot and configuration composition

```mermaid
sequenceDiagram
    participant Main as Composition root
    participant HW as ESP32 adapters
    participant Defaults as FlightConfig
    participant Store as PreferencesTuningStore
    participant App as Flight application
    participant Sched as FreeRTOS scheduler

    Main->>HW: construct and initialize safe hardware
    Main->>Defaults: initialize runtime tuning defaults
    Main->>Store: load(TuningState)
    alt record valid
        Store-->>Main: verified tuning snapshot
    else missing/corrupt/incompatible
        Store-->>Main: failure; retain defaults
    end
    Main->>App: apply configuration and connect ports
    Main->>Sched: create tasks and start 400 Hz timer
```

## IMU-to-motor control flow

```mermaid
flowchart LR
    SENSOR["MPU9250 or BNO085"] --> ADAPTER["SelectedImu adapter"]
    ADAPTER --> FILTER["Notch + gyro LPF"]
    ADAPTER --> EST["EKF / Mahony / Madgwick"]
    RX["Receiver"] --> CMD["Validated PilotCommand"]
    CMD --> MODE["Flight mode state machine"]
    EST --> MODE
    FILTER --> RATE["Rate controller"]
    MODE --> ANGLE["Optional angle controller"]
    ANGLE --> RATE
    RATE --> MIX["Motor mixer + limits"]
    MIX --> SAFE["Arming/failsafe gate"]
    SAFE --> MOTOR["MotorPort / ESCs"]
```

At present these responsibilities are still partially co-located in the main control task;
the diagram is the required decomposition target and preserves the current signal path.

## Runtime tuning and permanent save flow

```mermaid
sequenceDiagram
    participant UI as HTML/configurator
    participant WiFi as Wi-Fi tune adapter
    participant App as Tuning service
    participant Ctrl as Controller/estimator objects
    participant NVS as PreferencesTuningStore

    UI->>WiFi: POST /tune JSON
    WiFi->>App: parsed TunePacket
    App->>App: require disarmed and constrain fields
    App->>Ctrl: atomically apply complete snapshot
    App->>NVS: save verified snapshot
    NVS->>NVS: write, read back, validate CRC
    alt verified
        NVS-->>App: success
        App-->>UI: applied and saved
    else failure
        NVS-->>App: failure
        App-->>UI: persistence error
    end
```

## Task composition

| Task/service | Core | Nominal rate | Inputs | Outputs |
| --- | ---: | ---: | --- | --- |
| Control | 1 | 400 Hz | IMU, command snapshot, tuning | estimator state, motor command, flight state |
| RC | 0 | receiver paced | iBUS bytes | validated command snapshot |
| ToF | 0 | 40 Hz | I2C range | timestamped range state |
| BMP280 | 0 | 20 Hz | I2C pressure | altitude/vertical-speed state |
| GPS | 0 | 50 Hz service | UART NMEA | GPS state |
| Wi-Fi | 0 | event driven | state snapshots, HTTP | telemetry and validated requests |
| Diagnostics/serial | 0 | low rate | state/log snapshots | noncritical output |

Shared snapshots must have one owner and bounded synchronization. No low-priority service may
hold a lock or perform an operation that blocks the 400 Hz control path.
