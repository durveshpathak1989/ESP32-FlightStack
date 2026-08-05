# Software Component Runnable and Port Matrix

## Rules

Every software component (SWC) provides exactly these lifecycle runnables:

| Runnable | Trigger | Contract |
| --- | --- | --- |
| `Init` | once, after all ports are connected and before scheduling | initialize owned state; invalidate outputs until safe data exists |
| `Periodic` | component-specific periodic/event activation | read required ports, validate inputs, perform bounded work, publish outputs |

Application-to-application data uses typed sender/receiver (S/R) ports only. ESP32,
FreeRTOS, persistence, network, and hardware operations use client/server (C/S) ports.
`DataValidity::NeverReceived`, `Valid`, and `Invalid` accompany every S/R sample together
with a timestamp and monotonically increasing sequence counter.

## Application SWC sender/receiver interfaces

| Port | Datatype | Direction | Sender SWC | Receiver SWC | Unit | Range | Validity condition | Type |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `EstimatorInput` | `AttitudeEstimatorInput` | required | IMU conditioning SWC | Attitude estimator router | g, deg/s, µT, s | sensor-specific; mode `0..2`; `dt>0` | fresh IMU sample and bounded estimator mode | S/R implicit |
| `AttitudeEstimate` | `AttitudeEstimate` | provided | Attitude estimator router | Cascaded controller, telemetry publisher | deg, quaternion | roll/pitch `[-180,180]`; yaw implementation range; quaternion normalized | estimator input valid and selected algorithm completed | S/R implicit |
| `ControlInput` | `CascadedControlInput` | required | Flight-mode/command SWC | Cascaded controller | normalized, deg, deg/s, s, Hz | commands `[-1,1]`; `dt>0`; cutoff `>=0` | command and conditioned-rate samples belong to current control cycle | S/R implicit |
| `ControlConfig` | `TuningState` | required | Tuning SWC | Cascaded controller | mixed; see fields | validated tune bounds | complete atomic tuning snapshot | S/R implicit |
| `ControlOutput` | `CascadedControlOutput` | provided | Cascaded controller | Motor mixer, diagnostics | deg, deg/s, normalized | corrections bounded by axis limits | both required input ports valid | S/R implicit |
| `MixerInput` | `MotorMixerInput` | required | Cascaded controller/command SWC | Motor mixer | normalized, s | throttle `[0,1]`; corrections configured limits; `dt>0` | current-cycle controller result | S/R implicit |
| `MixerConfig` | `MotorMixerConfig` | required | Tuning SWC | Motor mixer | normalized, 1/s | expo `[0,1]`; nonnegative rates; `idleRampEnd>cut` | complete atomic tuning snapshot | S/R implicit |
| `MotorCommand` | `MotorMixerOutput` | provided | Motor mixer | Safety gate, motor-output adapter, diagnostics | normalized | final motors `[0,motorMaximum]` | mixer input and config valid | S/R implicit |
| `LevelTrimInput` | `LevelTrimInput` | required | Flight-mode/attitude SWCs | Level-trim SWC | bool, ms, deg | duration `>0`; attitude finite | disarmed capture eligibility and valid attitude | S/R implicit |
| `LevelTrimOutput` | `LevelTrimOutput` | provided | Level-trim SWC | Flight attitude transform, diagnostics | deg | offsets estimator-dependent | after first valid Periodic; event flags are one-cycle | S/R implicit |
| `FlightState` | `FlightState` | provided | control and sensor publisher SWCs | telemetry, log, OTA-safety SWCs | mixed; field-specific | field-specific | atomic snapshot; each sensor field includes explicit validity/age | synchronized S/R |

## Client/server service interfaces

| Port | Operation/datatype | Direction | Client SWC | Server SWC | Unit/range | Validity/failure | Type |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `WifiService` | `InitAccessPoint(ssid,password)` | required | Wi-Fi service task | ESP32 telemetry Wi-Fi adapter | strings | `false` prevents ready state | C/S synchronous |
| `WifiService` | `PeriodicService()` | required | Wi-Fi service task | ESP32 telemetry Wi-Fi adapter | one bounded service pass | transport errors contained in adapter | C/S synchronous |
| `CpuLoadService` | `InitMonitor(periodMs)` | required | CPU service task | ESP32 CPU-load adapter | ms, `>0` | `false` marks service unavailable | C/S synchronous |
| `CpuLoadService` | `ReadLoad()` → `CpuLoadSample` | required | CPU service task | ESP32 CPU-load adapter | percent `[0,100]` | sample contains explicit `valid` | C/S synchronous |
| `PersistentStorage` | `load/save/erase` | required | Tuning/calibration SWCs | ESP32 NVS adapters | byte records | schema, size, CRC, and read-back verification | C/S synchronous; disarmed only |
| `CoreService` | `currentCore/delay/yield` | required | platform task SWCs | ESP32 FreeRTOS core adapter | core ID, ms | platform error policy; never called by control equations | C/S synchronous |
| `ClockService` | `microseconds/milliseconds` | required | scheduled SWCs | ESP32 clock adapter | monotonic µs/ms | wrap handled by unsigned elapsed arithmetic | C/S synchronous |
| ImuService | InitSensor/ReadSample/HasMagnetometer/LoadCalibration | required | flight-control SWC, firmware lifecycle | selected IMU adapter | g, deg/s, µT, °C | operation result plus magnetic-field validity | C/S synchronous |
| BatteryService | InitMonitor/ReadStatus/CalibrationScale | required | flight-control SWC, firmware lifecycle | ESP32 ADC battery adapter | V, %, scale | sample contains valid/low/critical flags | C/S synchronous |
| ReceiverService | InitReceiver/ReadFrame | required | receiver service task | ESP32 iBUS adapter | normalized and raw channels | frame contains explicit command validity | C/S synchronous |
| GpsService | InitReceiver/ReadPosition | required | GPS service task | ESP32 GPS adapter | degrees, m, km/h, time | sample contains fix and validity flags | C/S synchronous |
| BarometerService | InitSensor/ReadSample | required | barometer service task | BMP280 adapter | °C, hPa, m | boolean result and sample validity | C/S synchronous |
| RangeService | InitSensor/ReadRange | required | ToF service task | VL53L4CX adapter | mm, MCPS, ms | ready, valid, range-status, and age fields | C/S synchronous |
| MotorService | begin/prepareEscs/write/stop | required | flight-control SWC, firmware lifecycle | ESP32 PWM motor adapter | normalized [0,1] | safety SWC gates every nonzero command | C/S synchronous |
| CalibrationService | Request/Status/SetSafety/ConfirmStep/PeriodicService/Cancel | required | receiver, flight-control, diagnostics SWCs | calibration-manager adapter | progress [0,1]; state enum | status exposes safe, active, blocks-flight, owns-motors, error | C/S synchronous |
| DiagnosticService | CommandAvailable/ReadCommandByte/Write | required | serial diagnostics SWC | ESP32 serial adapter | byte/text | reads are nonblocking; diagnostic output is never control input | C/S synchronous |

## Runnable ownership

| SWC | `Init` result/state | `Periodic` activation | Worst-case policy |
| --- | --- | --- | --- |
| Attitude estimator router | reset EKF; invalidate attitude output | 400 Hz | no allocation, I/O, or blocking |
| Cascaded controller | reset six PIDs, command LPFs, yaw hold; invalidate output | 400 Hz | no allocation, I/O, or blocking |
| Motor mixer | reset throttle slew; invalidate output | 400 Hz | no allocation, I/O, or blocking |
| Level-trim service | clear capture/offset state; invalidate output | 400 Hz while control runs | no allocation, I/O, or blocking |
| Flight-control orchestrator | reset control/estimator state | timer-released 400 Hz | hardware only through C/S; SWC data only through S/R; no intentional wait |
| Serial diagnostics | clear tick state and publish banner | 20 Hz | noncritical; never scheduled on the control core |
| Receiver service task | reset edge/health counters | 5 ms | bounded receiver service and calibration request |
| GPS service task | reset diagnostic timer | 20 ms | one UART parser service and bounded RTE write |
| Barometer service task | reset vertical-speed history | 50 ms | I2C mutex wait limited to 10 ms; RTE wait 2 ms |
| ToF service task | no retained reset required | configured 25 ms | I2C mutex wait limited to 10 ms; RTE wait 2 ms |
| CPU service task | initialize monitor through C/S port | 500 ms | one C/S call and bounded RTE write |
| Wi-Fi service task | initialize AP through C/S port | event service with 10 ms yield | never runs on control core; no flight-state write |

The FreeRTOS run functions and task entry functions are platform scheduling
adapters, not SWC runnables. They invoke Init once and Periodic at each
activation. No task loop contains application decisions.

Any new SWC must add its two runnables and every provided/required port to these tables before
its implementation is accepted.
