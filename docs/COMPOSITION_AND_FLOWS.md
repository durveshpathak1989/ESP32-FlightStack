# Architecture V4 Composition and Runtime Flows

## AUTOSAR-inspired drone stack

This project uses AUTOSAR's separation principles without claiming AUTOSAR
Classic conformance. The RTE is a small typed runtime contract, and the ESP32/
Arduino/FreeRTOS implementation remains replaceable below that boundary.

```mermaid
flowchart TB
    subgraph APP["Application software components"]
        direction LR
        MODE["Flight mode + safety"]
        EST["Attitude estimator router"]
        CTRL["Cascaded controller"]
        MIX["Quad-X motor mixer"]
        CAL["Calibration service"]
        TUNE["Tuning service"]
        DIAG["Flight logging + diagnostics"]
    end

    RTE["Drone Runtime Environment — typed snapshots, ports, scheduling contracts"]

    subgraph BSW["Basic software / platform services"]
        direction LR
        SYS["FreeRTOS scheduling, clock, watchdog"]
        MEM["NVS configuration + calibration storage"]
        COM["Wi-Fi HTTP, OTA, serial, GPS, iBUS"]
        IO["SPI, I2C, UART, PWM, ADC"]
        HAL["IMU, barometer, ToF, battery, receiver, motor HAL"]
        COMPLEX["EKF, Mahony, Madgwick, FFT, notch filtering"]
    end

    MCU["ESP32 microcontroller / replaceable MCU target"]
    HW["Drone hardware — MPU9250 or BNO085, BMP280, VL53L4CX, GPS, RC, ESCs"]

    APP --> RTE
    RTE --> BSW
    BSW --> MCU
    MCU --> HW
```

Dependency direction is downward only. Application components cannot include
ESP32, Arduino, FreeRTOS, buses, or concrete board pins. Platform adapters
implement application-owned ports. The 108-line .ino file is the include-level
composition root. Concrete object wiring is grouped under src/Composition; SWC
behavior is under src/Application; ESP32 task and service adapters are under
src/Platforms/Esp32. The sketch contains no flight-control equations.

The normative runnable and port definitions—including datatype, direction, sender,
receiver, unit, range, validity, and communication type—are maintained in
[`SWC_PORT_MATRIX.md`](SWC_PORT_MATRIX.md).

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

FlightControlSwc owns the deterministic 400 Hz orchestration. It exchanges
application data with the estimator, cascaded controller, mixer, level trim, receiver,
and state publishers through typed S/R ports. Hardware access is through IMU, battery,
motor, clock, and calibration C/S ports. The production control equations live in their
named controller/estimator/mixer modules rather than in the Arduino sketch.

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

The concrete FreeRTOS task creation table and 400 Hz release timer are owned by
Esp32FlightScheduler. GPS, receiver, barometer, ToF, CPU, and Wi-Fi loops are implemented in
their named Platforms/Esp32/Tasks adapters. The task entry trampolines and board wiring are
isolated in src/Composition/Esp32TaskComposition.inc; each trampoline delegates immediately
to an SWC runnable or platform task adapter.

## Source ownership

| Location | Single responsibility |
| --- | --- |
| RC_FlightController.ino | declare build dependencies and assemble the four composition sections |
| src/Application/Runtime/FlightControlRuntime.inc | deterministic flight-control SWC runnables |
| src/Application/Diagnostics/SerialDiagnosticsRuntime.inc | noncritical serial diagnostics SWC runnables |
| src/Application/Control | controller and motor-mixing algorithms |
| src/Application/Estimation | estimator selection and attitude output |
| src/Composition/RuntimeObjects.inc | instantiate ports, SWCs, adapters, and shared runtime objects |
| src/Composition/ConfigurationTelemetryBindings.inc | translate HTTP/configurator requests and telemetry DTOs |
| src/Composition/Esp32TaskComposition.inc | bind SWC runnables to ESP32 task activations |
| src/Composition/FirmwareLifecycle.inc | boot order and scheduler start |
| src/Platforms/Esp32/Services | C/S adapters for ESP32 and concrete device services |
| src/Platforms/Esp32/Tasks | platform activation loops that call Init and Periodic |
