# Software Component Interface Contracts

This is the interface catalog for Architecture V4. It is normative: code and
tests must follow these contracts. Units are part of the interface.

## Core data SWC: `FlightTypes`

**Layer:** Core  
**Responsibility:** Portable data exchanged through hardware and application ports.

| Type/field | Direction | Unit/range | Validity |
| --- | --- | --- | --- |
| `ImuSample.accelerationG` | IMU → estimator | g, body axes | `accelerationValid` |
| `ImuSample.angularRateDps` | IMU → controller/estimator | degrees/s, body axes | `angularRateValid` |
| `ImuSample.magneticFieldUt` | IMU → estimator | µT, body axes | `magneticFieldValid` |
| `ImuSample.timestampUs` | clock/IMU → consumers | monotonic µs | nonzero after first sample |
| `PilotCommand` | receiver → application | normalized `[-1,1]`; throttle `[0,1]` | `valid` and failsafe state |
| `Attitude` | estimator → controller | degrees and unit quaternion | `valid` |
| `MotorCommand` | controller → motors | normalized `[0,1]` | application clamps before output |

**State:** none. **Side effects:** none. **Dependencies:** C++ standard integer types only.

## Core port SWCs

### `ClockPort`

- **Input:** none.
- **Output:** monotonic microseconds and milliseconds.
- **Caller:** scheduler, diagnostics, application services.
- **Failure policy:** must not move backward during a boot session.

### `ImuPort`

- **Input:** `begin()` configuration is supplied by the concrete adapter constructor.
- **Output:** `ImuSample`; expected control acquisition rate is 400 Hz.
- **Failure policy:** `false` means no fresh usable sample; consumers retain failsafe behavior.
- **Prohibited:** motor writes, network operations, controller access.

### `ReceiverPort`

- **Input:** physical/protocol receiver stream.
- **Output:** `PilotCommand` with explicit validity.
- **Update expectation:** receiver-specific, sampled by the RC task.
- **Failure policy:** invalid command must lead to application failsafe, never last-command flight indefinitely.

### `MotorPort`

- **Input:** normalized `MotorCommand`.
- **Output:** physical ESC command; `stop()` is an unconditional safe-state request.
- **Execution:** time-bounded; no allocation or network access.
- **Failure policy:** platform adapter must prefer stopped output.

### `ConfigurationStorePort`

- **Input:** key plus bounded byte record.
- **Output:** success/failure and loaded bytes.
- **Execution:** disarmed/non-real-time context only.
- **Side effect:** nonvolatile storage mutation.

### `DiagnosticPort`

- **Input:** immutable message text.
- **Output:** platform-selected diagnostic transport.
- **Rule:** armed control code cannot depend on diagnostic completion.

## Application SWC: `FlightConfig`

**Provided interface:** compile-time constants for one board/application composition.  
**Required interface:** none.  
**Inputs:** developer-selected pins, backend macro, rates, gains, limits, electrical constants.  
**Outputs:** strongly typed constants consumed during construction/default initialization.

**Safety:** changing values changes the board composition or safe fallback behavior. Runtime
GCS tuning is separate and may override `TUNE_*` values after validated NVS load.

## Application SWC: `TuningState`

**Provided interface:** plain runtime configuration snapshot.  
**Required interface:** none; platform independent.  
**Inputs:** firmware defaults, validated tune packets, or verified persistent record.  
**Outputs:** controller, estimator, filter, throttle, and authority configuration.

**Ownership:** `g_tuning` currently owns the active instance; access is serialized by
`g_tuneMutex`. A future `TuningService` will own it and remove the global.

**Validity:** external input is constrained before assignment. `dirty` is runtime-only and
must not cause a controller to observe a partially updated snapshot.

## Application SWC: `PidController`

| Interface | Inputs | Output | Side effects |
| --- | --- | --- | --- |
| `update` | error, seconds `dt` | controller output | updates integral/history/diagnostics |
| `updateDOnMeasurement` | error, measurement, seconds `dt`, D-LPF Hz | controller output | derivative-on-measurement state |
| `reset` | none | none | clears all accumulated state |

**Execution:** 400 Hz control task. **Allocation:** none. **Blocking:** none.  
**State owner:** one instance per control axis/loop; never shared across axes.  
**Failure policy:** near-zero `dt` produces zero derivative rather than division by zero.

## Infrastructure SWC: `PreferencesTuningStore`

**Layer:** ESP32 platform services.  
**Required interface:** ESP32 `Preferences`.  
**Provided interfaces:** `save(TuningState)`, `load(TuningState&)`, `erase()`.

| Operation | Input | Output | Context |
| --- | --- | --- | --- |
| `save` | complete validated tuning snapshot | verified success/failure | disarmed HTTP service |
| `load` | destination snapshot | valid record or failure | boot before tasks |
| `erase` | none | removal status | disarmed maintenance |

**Integrity:** magic, schema, size, and CRC32. **Namespace/key:** `flightTune/state`.  
**Fallback:** missing, incompatible, or corrupt data leaves firmware defaults active.  
**Prohibited:** use from the armed 400 Hz path.

## Driver-selection SWC: `SelectedImu`

**Layer:** ESP32 adapter/composition boundary.  
**Input configuration:** backend macro, bus pins, chip select/address, report interval.  
**Output:** common `ImuSensorData` in g, degrees/s, µT, °C, and milliseconds.

### MPU9250 implementation

- SPI acquisition and optional AK8963 magnetometer.
- Supports existing manual gyro/accelerometer/magnetometer calibration.
- Calibration persists through the IMU submodule's store.

### BNO085 implementation

- I2C calibrated accelerometer, calibrated gyro, and calibrated magnetic-field reports.
- A read succeeds only with a fresh gyro report and previously valid acceleration.
- Existing EKF/Mahony/Madgwick remains the attitude authority.
- Existing MPU manual calibration is rejected; persistent DCD support is a future capability.

**Failure policy:** `begin()` or fresh-read failure propagates to existing IMU-invalid/failsafe behavior.

## Interface change checklist

- [ ] Inputs, outputs, units, and valid ranges documented.
- [ ] Execution rate and task/core context documented.
- [ ] State owner and synchronization documented.
- [ ] Failure/fallback behavior documented.
- [ ] Side effects and persistence documented.
- [ ] Composition and runtime-flow diagrams updated.
- [ ] Portable host test or target build added.
- [ ] No prohibited dependency crosses inward.
