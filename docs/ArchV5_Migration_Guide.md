# ArchV5 Migration Guide

## Current safe state

The flight-tested `RC_FlightController.ino` remains active. ArchV5 modules are
available for host validation and incremental wiring.

## Recommended wiring order

1. Replace only PID math with `SWC_PIDController`; keep all sensor and motor
   plumbing unchanged.
2. Compare motor command logs against v6.1.0 for the same recorded input.
3. Add `SWC_MotorOutput` as a PWM mapping audit tool before writing motors.
4. Add `SWC_FailsafeLogic` in monitor-only mode and log its output.
5. Enable failsafe orchestration only after false-positive testing.
6. Move sensor readers behind components one at a time.
7. Promote `FlightApplication` after full props-off and hover regression.

## Non-local requirements

- Drone, transmitter, battery, and safe test area.
- Arduino CLI or IDE configured for ESP32 core.
- GitHub permissions for branch protection, push, and release creation.
