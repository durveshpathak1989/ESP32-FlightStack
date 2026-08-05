# Adding or Replacing Hardware

This guide describes the intended extension workflow while the HAL migration is
in progress.

## Replace a pin-compatible part

1. Open `RC_FlightController/src/Application/FlightConfig.h`.
2. Change only the relevant pin or electrical constant.
3. Compile before connecting motors.
4. Test with propellers removed.

## Add a new device implementation

1. Choose the matching interface in `RC_FlightController/src/Core/Ports.h`.
2. Create an adapter under `RC_FlightController/src/Platforms/<Platform>/`.
3. Translate the device library's units into the units in `FlightTypes.h`.
4. Keep vendor types inside the adapter.
5. Add a host test using a fake implementation of the same interface.
6. Select the adapter only in the board's composition root.

For example, every IMU adapter must return acceleration in g, angular rate in
degrees per second, magnetic field in microtesla, and an explicit validity flag.
Controllers never need to know which IMU produced the sample.

## Port to another microcontroller

Do not copy and edit flight-control equations. Implement the platform services
listed in `docs/ARCHITECTURE.md`, then construct the same application with those
adapters. Keep all vendor SDK includes in the platform folder.

## Definition of done

A hardware or MCU port is complete when:

- the portable core compiles without vendor headers;
- the board build passes;
- failsafe and motor-stop tests pass;
- configuration can be loaded, validated, and reset;
- the control-loop period and resource budgets are documented;
- a child following this guide changes an adapter or registry entry rather than
  editing the main control loop.
