# Biquad Notch Filter

## Purpose

Applies configurable second-order notch filtering to IMU channels to reduce motor-vibration energy before estimation and control.

## Files

- `NotchFilter.h/.cpp`: Filter coefficient calculation, sample update, reset, and enable control.

## Quick Start

```cpp
#include "NotchFilter.h"

NotchFilter notch;

void setup() {
    notch.configure(400.0f, 90.0f, 8.0f, true);
}

void loop() {
    float filteredGyro = notch.apply(rawGyro);
}
```

## How It Fits Into The Flight Controller

This library lives under `Submodules/NotchFilter` in the main `Test_Quad` firmware
and is built as an Arduino library by adding `Submodules/` to the Arduino
library search path. The main firmware includes it directly from
`RC_FlightController.ino` or from another support module.

The flight controller runs a 400 Hz control loop on ESP32, so this library
should avoid heap allocation, long blocking calls, and unbounded Serial output
inside flight-critical paths. Debug output should use `DebugConfig.h` macros
where available so `VERBOSE_ON=0` builds can compile prints out.

## Data Type Choices

- `float` coefficients: Biquad coefficients and IMU samples are continuous values; float is fast enough on ESP32.
- `bool enabled`: Allows runtime tuning to bypass filtering without changing caller code.
- Internal state floats: Previous input/output samples must preserve fractional precision for stable filtering.

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

