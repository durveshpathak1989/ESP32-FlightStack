# Controller Swapping

ArchV5 selects the active controller through `ControllerRegistry`.

```cpp
ControllerRegistry registry;
SWC_PIDControllerInput input;

registry.setActiveController(ControllerType::PID, false);
auto output = registry.update(input);
```

Rules:

- Controller switches are accepted only while disarmed.
- PID is the default and fallback.
- LQR is a Phase 2 stub in this branch; it returns no motor command and the
  registry falls back to PID.
- New controllers should accept `SWC_PIDControllerInput` or a compatible
  contract and return normalized `flight::MotorCommand` values.

To add a controller:

1. Add a service class in `Services/Control`.
2. Add a registry enum value.
3. Dispatch it in `ControllerRegistry::update`.
4. Add host tests for disarmed switching, armed switch rejection, and fallback.
