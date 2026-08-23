# Layered Architecture (AUTOSAR-inspired)

The firmware follows a strict layered architecture modelled on the AUTOSAR
Classic Platform. Dependencies point **downward only**. A layer may only
call layers beneath it; upward or sideways calls are violations.

```
┌──────────────────────────────────────────────────────────┐
│  Sketch / composition root                               │
│  RC_FlightController.ino                                 │
│  FreeRTOS tasks, wiring, task creation, main loop        │
├──────────────────────────────────────────────────────────┤
│  Application Layer            src/App/                   │
│  Flight algorithms: cascaded PID, notch filter,          │
│  AHRS estimators (EKF, Mahony, Madgwick)                 │
├──────────────────────────────────────────────────────────┤
│  Services Layer               src/Services/              │
│  Telemetry/OTA comms, calibration, flight logger,        │
│  FFT spectrum service, diagnostics, CPU monitor          │
├──────────────────────────────────────────────────────────┤
│  ECU Abstraction Layer        src/EcuAbstraction/        │
│  Sensor drivers: IMU, BMP280, ToF, GPS                   │
│  Actuators: ESC motor control    Receiver: iBUS          │
│  Power: battery ADC                                      │
├──────────────────────────────────────────────────────────┤
│  MCAL                         src/Mcal/                  │
│  ESP32 Arduino Core APIs today; portable peripheral      │
│  adapters when porting to another MCU                    │
├──────────────────────────────────────────────────────────┤
│  Hardware                                                │
└──────────────────────────────────────────────────────────┘

  Cross-cutting: src/Core/   (types + port interfaces)
                 src/Cfg/    (tuning + pin configuration)
```

## Layer contracts

| Layer | May include | Must never include |
|---|---|---|
| Core | standard C headers | Arduino, ESP32, FreeRTOS, any driver |
| Cfg | Core | anything else in src |
| App | Core, Cfg | Services, EcuAbstraction, Mcal, Arduino/ESP32/FreeRTOS |
| Services | App, Core, Cfg | EcuAbstraction internals* , Mcal APIs** |
| EcuAbstraction | Core, Mcal APIs** | App, Services |
| Mcal | Core | everything above |

\* Sanctioned exceptions — see below.
\** Until a true Mcal wrapper set exists, EcuAbstraction and Services/Diagnostics
use ESP32 Arduino Core APIs directly; this is tracked as migration debt.

## Sanctioned exceptions

Real systems carry legacy coupling. These are explicitly allowed and
documented rather than hidden:

1. `Services/Diagnostics/Logger` subclasses `AttitudeEKF`
   (`App/Estimation/EKF`) to instrument rate outputs without touching
   estimator math. Diagnostics observing an application component.
2. `Services/Calibration/CalManager` includes the IMU driver header to
   drive sensor calibration sequences directly.

Both are candidates for port-based inversion once `Core/Ports.h`
contracts grow implementations.

## Shared data types

Plain-data structs shared across layers live in Core:

- `Core/FlightTypes.h`, `Core/Ports.h` — existing portable types
- `Core/ImuTypes.h` — `MPU_RawData`, `MPU_SensorData`, `MPU_CalData`;
  consumed by both the IMU driver (producer) and estimation filters
  (consumers), so neither depends on the other

## History

Before this refactor each module lived in its own git repository wired
in as a submodule under `src/Submodules/`. They are vendored here so
layer boundaries are enforceable inside one tree. The original module
repositories remain on GitHub unchanged.

## Build verification

`esp32-arduino-ci.yml` compiles the sketch on every push. The layer
layout itself is checked by the "Verify sketch and layered sources" step;
deeper dependency-rule enforcement (e.g. include-direction linting) is a
planned addition.
