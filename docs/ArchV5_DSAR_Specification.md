# ArchV5 DSAR Specification

## Purpose

ArchV5 reorganizes the flight stack into a DSAR architecture: Application,
Service, Component, Platform, HAL/Drivers, and Hardware. Phase 1 preserves the
existing manual flight behavior while creating contracts for controller
swapping, host testing, and safety logic.

## Dependency rule

Services and Core are portable C++. They must not include Arduino, FreeRTOS, or
ESP32 headers. Components may schedule and adapt data, Platform owns board
details, and existing drivers remain in HAL/Submodules until the flight sketch
is migrated.

## Phase 1 local deliverables

| Layer | Files |
|---|---|
| Core | `RC_FlightController/src/Core/FlightTypes.h`, `FlightConstants.h` |
| Services | `Services/Control`, `Services/Estimation`, `Services/Navigation`, `Services/Diagnostics` |
| Components | `Components/SWC_*` |
| Application | `FlightApplication.*`, `ModeManager.*`, `FailsafePolicy.h` |
| Platform | `Platforms/ESP32`, `Platforms/Host` |
| Tests | `tests/test_SWC_*.cpp`, `tests/Makefile` |

## Flight behavior constraint

The existing sketch remains the flight-approved firmware until the ArchV5 path
passes bench and hover regression tests. New code preserves the repo's current
v6.1.0 tuning defaults from `Application/FlightConfig.h`.

## Go/no-go gates

- Host tests pass.
- No platform includes in `src/Services` or `src/Core`.
- Arduino build passes with zero warnings.
- Firmware size remains within the existing release budget.
- Props-off motor order and failsafe checks pass.
- Hover regression matches v6.1.0 before replacing the active flight path.
