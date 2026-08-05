# AUTOSAR-Inspired FlightStack Design

This project uses AUTOSAR principles where they improve safety and portability,
without claiming AUTOSAR Classic compliance. Arduino libraries and the current
ESP32 runtime are not an AUTOSAR operating environment.

## Layer model

| FlightStack layer | AUTOSAR analogue | Responsibility |
| --- | --- | --- |
| `Core` | Application software components | Portable types, control contracts, algorithms |
| `Application` | SWC composition and calibration data | Flight use cases, validated configuration, controller selection |
| `Platforms/<MCU>` | ECU abstraction / services | Scheduling, storage, diagnostics, communication |
| `Submodules` and drivers | MCAL / complex drivers | Registers, buses, sensors, receivers, motor protocols |
| `.ino` | ECU composition root | Construct modules, connect ports, start scheduler |

## Mandatory dependency rules

1. Core code must compile with a desktop C++17 compiler.
2. Core code cannot include Arduino, ESP32, FreeRTOS, Wi-Fi, or Preferences.
3. Application code depends on ports, never concrete hardware.
4. Platform adapters may depend on vendor APIs and implement application ports.
5. Drivers cannot call controllers or mutate application state directly.
6. Communication handlers validate external data before application use.
7. Persistent formats are versioned and integrity checked.
8. The armed control path performs no dynamic allocation, flash writes, or
   blocking network operations.

## Module contract

Every module should have:

- one responsibility;
- an explicit public header;
- owned state rather than unrelated globals;
- documented units and validity rules;
- deterministic error behavior;
- host tests when it belongs to Core/Application;
- a board build when it belongs to a platform or driver.

## Required interface documentation

Every new or changed SWC/module must be added to `docs/SWC_INTERFACES.md` and
must document:

1. responsibility and owning layer;
2. provided interfaces (outputs/services);
3. required interfaces (inputs/services);
4. data type, unit, range, validity, and update rate for each port;
5. caller/callee execution context and concurrency assumptions;
6. state ownership and lifetime;
7. errors, fallback behavior, and side effects;
8. permitted dependencies and prohibited dependencies;
9. verification method and safety notes.

A module extraction is incomplete until its interface contract and its place in
the composition/runtime-flow diagrams are documented.

## Main-file target

The final `.ino` should contain only:

```cpp
void setup() {
    board.construct();
    application.initialize(board.ports());
    scheduler.start(application.tasks());
}

void loop() {
    scheduler.idle();
}
```

Until migration reaches that point, each extraction must preserve control
equations, timing, task priorities, gains, failsafe behavior, and telemetry.

## Configuration ownership

- `FlightConfig.h`: compile-time board and safe fallback defaults.
- `TuningState.h`: platform-neutral runtime tuning data.
- input adapter: range and relationship validation.
- `PreferencesTuningStore`: ESP32 persistence only.
- future MCU ports: implement the same configuration-store port using their
  native flash/NVM service.

The storage adapter must not decide flight behavior, and controllers must not
know how configuration was transported or persisted.
