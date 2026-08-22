# ArchV5 README

ArchV5 is the DSAR migration track for ESP32-FlightStack. It adds versioned
software component contracts and a layered path for controller swapping,
failsafe logic, host testing, and future platform ports.

## Local status

- Branch: `ArchV5`
- Flight-tested entry point: `RC_FlightController/RC_FlightController.ino`
- ArchV5 scaffold entry point: `RC_FlightController_ArchV5/RC_FlightController_ArchV5.ino`
- Pure services: `RC_FlightController/src/Services`
- Component wrappers: `RC_FlightController/src/Components`
- Application orchestration: `RC_FlightController/src/Application/FlightApplication.*`

## What changed

- `Core/FlightTypes.h` now carries versioned contracts for attitude, pilot
  command, motor command, battery, GPS, receiver state, and flight config.
- `SWC_PIDController` implements the existing cascaded PID structure with
  setpoint filtering, yaw hold, throttle shaping, desaturation, and idle ramp.
- `ControllerRegistry` defaults to PID and provides an LQR stub that falls back
  safely to PID until a real controller is implemented.
- `SWC_FailsafeLogic` covers battery low/critical, RC timeout, and GPS timeout
  with battery priority and low-voltage hysteresis.
- Host tests and a GitHub Actions workflow document the portable validation
  path.

## Hardware gate

The monolithic sketch has not been rewired to ArchV5 as the active flight path.
That step must be performed with propellers removed, telemetry connected, motor
order checked, and hover regression compared against v6.1.0.
