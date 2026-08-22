# ArchV5 Summary

This branch implements the local, testable portion of the ArchV5 Phase 1 plan.
It does not claim hardware flight validation.

## Completed locally

- Versioned core contracts.
- Pure PID, estimator, failsafe, health, and navigation helpers.
- Component wrappers for IMU, RC, battery, GPS, motor output, telemetry, PID,
  and failsafe orchestration.
- Application-level `FlightApplication` and `ModeManager`.
- ESP32 platform scaffold and host simulator scaffold.
- Host test sources and CI workflow.
- DSAR documentation and controller swapping guide.

## Not completed locally

- Real drone arm/hover/failsafe tests.
- GitHub branch protection setup.
- Remote push or release publication.
- Final replacement of `RC_FlightController.ino` with the ArchV5 control path.

Those items require hardware access and/or explicit remote operations.
