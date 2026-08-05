# FlightStack Architecture

## Goal

The flight algorithms should be understandable and testable without knowing
ESP32, Arduino, FreeRTOS, Wi-Fi, or a particular sensor library. Hardware and
microcontrollers are replaceable adapters around a portable C++ flight core.

Detailed contracts are cataloged in `docs/SWC_INTERFACES.md`; composition and
runtime sequences are maintained in `docs/COMPOSITION_AND_FLOWS.md`.

## Dependency rule

Dependencies point inward:

```text
Board entry point / scheduler
            ↓
Platform adapters (ESP32, STM32, RP2040, host simulator)
            ↓
Core ports (interfaces) ← Application orchestration
            ↓
Flight algorithms, controllers, and plain data types
```

The `Core` directory must never include Arduino, ESP32, FreeRTOS, Wi-Fi, or a
concrete device driver. Platform adapters may depend on the core, never the
reverse.

## Directory structure

```text
RC_FlightController/src/
├── Core/
│   ├── FlightTypes.h        plain portable data
│   ├── Ports.h              portable hardware contracts
│   ├── RtePorts.h           typed S/R ports and SWC lifecycle
│   └── ServicePorts.h       portable C/S service contracts
├── Application/
│   ├── FlightConfig.h       beginner-facing defaults
│   ├── Runtime/             400 Hz flight orchestration SWC
│   ├── Control/             controller and mixer SWCs
│   ├── Estimation/          estimator router SWC
│   ├── Calibration/         application calibration behavior
│   └── Diagnostics/         noncritical diagnostics SWC
├── Composition/             board wiring, task bindings, boot order
├── Platforms/
│   ├── Esp32/               Arduino/FreeRTOS/Preferences/Wi-Fi adapters
│   └── Host/                deterministic test adapters
└── Submodules/              concrete sensors, algorithms, and protocols
```

## Swapping hardware

A hardware implementation supplies the relevant port from `Core/Ports.h`:

- New IMU: implement `ImuPort`.
- New radio: implement `ReceiverPort`.
- PWM, DShot, or another ESC protocol: implement `MotorPort`.
- ESP32 NVS, STM32 flash, or a file in simulation: implement
  `ConfigurationStorePort`.
- New MCU timer: implement `ClockPort` and provide a scheduler adapter.

No controller should include a concrete driver header.

## Swapping microcontrollers

Create a platform folder containing:

1. clock and scheduler adapters;
2. synchronization primitives;
3. persistent storage;
4. networking/diagnostic transport, if available;
5. the composition root that constructs adapters and passes their interfaces
   to the application.

The portable core is built unchanged. A board that has no Wi-Fi can use a null
diagnostic adapter or a serial implementation.

## Adding a controller

Controllers will implement a stable control interface and consume only a
snapshot of pilot command, estimated state, and configuration. They return a
normalized motor command. They do not read sensors, write motors, allocate
memory, use global state, or perform network operations.

Controller selection belongs in one registry in the application layer. Adding
a controller should require:

1. one controller class;
2. one registry entry;
3. one configuration schema entry;
4. host-side unit tests.

## Safety constraints during migration

- Preserve the 400 Hz control period and task priorities.
- Perform no heap allocation in the armed control path.
- Keep motor stop and failsafe behavior independent of optional services.
- Validate configuration at the boundary before it reaches a controller.
- Persist only versioned and checksummed records while disarmed.
- Compile and compare resource usage after every extraction.
- Refactoring commits must not alter gains or flight equations unless explicitly
  labeled and tested as behavioral changes.

## Current implementation status

FlightConfig.h is the beginner-facing location for board pins and safe defaults.
The production estimator, cascaded controller, motor mixer, level trim, receiver,
sensor, Wi-Fi, CPU, flight-control, and diagnostics components implement the
mandatory Init/Periodic lifecycle. Inter-SWC data uses typed S/R ports; ESP32 and
device operations use C/S service contracts implemented by platform adapters.
The 108-line Arduino sketch only assembles the named composition sections.
