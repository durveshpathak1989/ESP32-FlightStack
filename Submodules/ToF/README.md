# Time-of-Flight Sensor Wrapper

## Purpose

Provides a small wrapper around optional VL53L4CX time-of-flight altitude sensing so the main firmware can compile even when the vendor library is absent.

## Files

- `ToFSensor.h/.cpp`: Optional sensor initialization and read interface.

## Quick Start

```cpp
#include "ToFSensor.h"

ToFSensor tof;

void setup() {
    tof.begin(21, 22, 400000);
}

void loop() {
    ToFData d{};
    if (tof.read(d) && d.valid) {
        uint16_t rangeMm = d.distance_mm;
    }
}
```

## How It Fits Into The Flight Controller

This library lives under `Submodules/ToF` in the main `Test_Quad` firmware
and is built as an Arduino library by adding `Submodules/` to the Arduino
library search path. The main firmware includes it directly from
`RC_FlightController.ino` or from another support module.

The flight controller runs a 400 Hz control loop on ESP32, so this library
should avoid heap allocation, long blocking calls, and unbounded Serial output
inside flight-critical paths. Debug output should use `DebugConfig.h` macros
where available so `VERBOSE_ON=0` builds can compile prints out.

## Data Type Choices

- `uint16_t distance_mm`: Millimeter distance readings are non-negative and fit comfortably in 16 bits for typical ToF range.
- `bool valid`: Optional hardware may be absent; validity prevents callers from trusting dummy values.
- Compile-time feature flag: Keeps the project buildable without forcing every developer to install the vendor ToF library.

## Usage Guidance

1. Initialize hardware-facing classes once during `setup()`.
2. Keep update/read calls deterministic when used from a FreeRTOS task.
3. Prefer explicit validity flags over sentinel numeric values.
4. Keep units visible in field names, such as `_dps`, `_g`, `_uT`, `_m`, or `_us`.
5. When adding telemetry fields, update both the packet struct and JSON serializer.

## Example Build Integration

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32:UploadSpeed=921600,CPUFreq=240,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default,ZigbeeMode=default \
  --libraries ./Submodules \
  .
```

For quiet flight builds:

```bash
arduino-cli compile ... --build-property compiler.cpp.extra_flags=-DVERBOSE_ON=0
```


## Integration Notes

In the main flight-controller sketch, this library is included through Arduino's
library search path. When this folder is converted to a git submodule, keep the
folder name stable under `Submodules/` so includes such as `#include "..."`
continue to resolve.

Most examples below are intentionally small. On the real flight controller,
objects are usually constructed globally, initialized once from `setup()`, and
then called from FreeRTOS tasks at deterministic rates.

