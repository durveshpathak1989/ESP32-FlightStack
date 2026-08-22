# ArchV5 Phase 1 Execution Roadmap

## Local execution plan

1. Create `ArchV5` branch.
2. Add versioned core contracts.
3. Add PID controller SWC and host tests.
4. Add controller registry with LQR stub.
5. Add estimator, failsafe, and health services.
6. Add component wrappers for sensors, control, motor output, telemetry, and
   failsafe orchestration.
7. Add application and platform composition scaffolds.
8. Add documentation and CI validation.
9. Run local validations that are available.
10. Mark hardware and remote operations as blocked until the user executes
    them with the drone and GitHub access.

## Hardware validation sequence

1. Compile existing sketch and ArchV5 candidate.
2. Remove propellers.
3. Flash firmware.
4. Confirm telemetry and calibration state.
5. Verify arm/disarm and motor order.
6. Run 400 Hz timing check.
7. Hover in angle mode and compare against v6.1.0.
8. Test failsafe thresholds only in a controlled, safe setup.

## Done for local Phase 1

Local Phase 1 is done when host tests pass, no service/core platform leaks are
detected, docs are updated, and hardware-dependent gates are explicitly listed.
