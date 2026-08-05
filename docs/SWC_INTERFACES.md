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

**Ownership:** the composition owns one active instance; access is serialized by
the dedicated tuning RTE channel. HTTP/configurator translation is isolated in
ConfigurationTelemetryBindings and cannot expose a partially updated snapshot.

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

## Application SWC: `MotorMixer`

**Provided interface:** MotorCommand S/R port. **Required interfaces:** MixerInput
and MixerConfig S/R ports. Init resets throttle-slew history and invalidates the
provided port; Periodic consumes one coherent input/config pair and publishes one
complete MotorMixerOutput.

| Input | Unit/range | Meaning |
| --- | --- | --- |
| throttle | normalized `[0, 1]` | requested collective throttle |
| roll/pitch/yaw correction | normalized motor command | bounded controller outputs |
| dt | seconds, positive | elapsed control-cycle time |
| configuration | normalized values and rates/second | expo, slew, idle, cut, ramp, maximum |

The output contains shaped throttle, all four pre-saturation values, final bounded Quad-X
motor values, saturation status, pre-desaturation maximum, and remaining high-side authority.

**Execution:** called synchronously by the 400 Hz control task. **Allocation/blocking/I/O:**
none. **State owner:** the component exclusively owns the prior shaped throttle used by its
slew limiter. The composition root owns one mixer instance. The instance is intentionally not
reset by disarm in this migration because the former function-local state also survived disarm;
changing that policy requires a separately reviewed safety change.

**Failure policy:** external tuning validation must keep rates nonnegative and
`idleRampEnd > throttleCut`. The component clamps normalized throttle and final motor commands.

## Application SWC: `CascadedController`

**Inputs:** normalized pilot roll/pitch/yaw commands, control attitude in degrees,
filtered angular rates in degrees/second, elapsed seconds, ANGLE/ACRO selection,
attitude-valid flag, and one immutable `TuningState` snapshot.

**Outputs:** filtered commands; angle/rate targets and errors; feed-forward terms;
bounded roll/pitch/yaw corrections; and per-axis saturation diagnostics.

**Execution:** synchronous at 400 Hz. **Owned state:** three command LPFs and yaw-heading
hold/setpoint. The six injected PID instances own their integral and derivative histories.
`reset()` clears all control histories at the existing disarm/calibration boundaries.
**I/O/allocation/blocking:** none. ACRO bypasses attitude feedback for roll/pitch and closes
the inner loop directly on conditioned gyro rates.

## Application SWC: `AttitudeEstimatorRouter`

**Required interface:** EstimatorInput S/R port containing common AHRSInput,
elapsed seconds, and estimator mode 0..2. **Provided interface:** AttitudeEstimate
S/R port containing common quaternion/Euler output plus read-only EKF diagnostics.
**Algorithms:** EKF, Mahony, or Madgwick. **Execution:** 400 Hz after IMU conditioning.
**Transition policy:** changing mode resets EKF and Madgwick exactly as the original runtime
did; Mahony state remains intact. **I/O/allocation/blocking:** none.

## Application SWC: `LevelTrimService`

**Inputs:** validated switch/capture eligibility, monotonic milliseconds, capture duration,
and raw estimator attitude in degrees. **Outputs:** capture-start/completion events and
roll/pitch/yaw offsets. **Owned state:** edge latch, accumulator, timing, offsets.
**Side effects:** none; the runtime adapter performs logging. A held switch is one-shot and
must be lowered before another capture.

## Application SWC: `FlightSafetyPolicy`

Pure decisions for control shutdown and OTA authorization. OTA requires disarmed state,
throttle at/below cut, and every motor output at/below the off threshold. It performs no
motor I/O; the motor adapter executes the resulting stop command.

## Platform SWC: `Esp32FlightScheduler`

**Required interface:** FreeRTOS task creation/notification and ESP high-resolution timer.
**Input:** declarative service-task table, control-task definition, control period in µs.
**Output:** started/not-started result. **Owned state:** control task handle and timer handle.
The timer only releases the control task; it never runs flight logic in timer context.
Task creation or timer failure prevents boot from declaring the flight runtime ready.

## Runtime SWC: `SnapshotRte<T>`

Owns one FreeRTOS mutex for one typed shared snapshot. `read()` produces an atomic copy;
`lock()`/`unlock()` provide a bounded writer transaction for the single owning producer.
Flight, tuning, and timing state use separate channels so a slow telemetry request cannot
hold the 400 Hz control path behind an unrelated state operation. No channel performs I/O.

## Platform task adapters

| Component | Activation | Required interfaces | Published output |
| --- | ---: | --- | --- |
| `ReceiverServiceTask` | 5 ms | iBUS, calibration request/log ports | receiver/calibration events |
| `GpsServiceTask` | 20 ms | GPS driver, flight-state RTE | GPS snapshot |
| `BarometerServiceTask` | 50 ms | BMP280, I2C mutex, flight-state RTE | pressure/altitude/vertical speed |
| `TofServiceTask` | 25 ms configured | VL53L4CX, I2C mutex, flight-state RTE | range/status/age |
| `CpuServiceTask` | 500 ms | CPU utilization monitor, flight-state RTE | per-core load |
| `WifiServiceTask` | event loop + 10 ms yield | telemetry adapter and bound callbacks | HTTP/OTA/log services |

Each task adapter owns only its pacing and device-specific publication state. It cannot
change flight-control equations or directly arm the aircraft.

## Platform adapter: `TelemetryTuneAdapter`

Translates optional HTML/configurator `TunePacket` fields into one validated `TuningState`.
All numeric bounds and cross-field throttle constraints are applied before the snapshot is
made visible or persisted. It performs no NVS write; `PreferencesTuningStore` remains the
single persistence adapter and verifies the complete record after saving.

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
