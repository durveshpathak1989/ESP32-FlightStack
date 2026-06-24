# Spectrum Analyzer

## Purpose

Collects IMU vibration samples and estimates dominant gyro vibration peaks for dynamic notch-filter tuning.

## Files

- `SpectrumAnalyzer.h/.cpp`: Sample buffering, spectral analysis, JSON telemetry, and peak selection.

## Quick Start

```cpp
#include "SpectrumAnalyzer.h"

SpectrumAnalyzer spectrum;

void loop() {
    spectrum.push(ax, ay, az, gx, gy, gz, motorsActive);
    float peakHz = 0.0f, score = 0.0f;
    uint32_t seq = 0;
    if (spectrum.findGyroPeak(45.0f, 170.0f, 3.5f, peakHz, score, seq)) {
        // Use peakHz to retune a notch filter slowly.
    }
}
```

## How It Fits Into The Flight Controller

This library lives under `Submodules/FFT` in the main `Test_Quad` firmware
and is built as an Arduino library by adding `Submodules/` to the Arduino
library search path. The main firmware includes it directly from
`RC_FlightController.ino` or from another support module.

The flight controller runs a 400 Hz control loop on ESP32, so this library
should avoid heap allocation, long blocking calls, and unbounded Serial output
inside flight-critical paths. Debug output should use `DebugConfig.h` macros
where available so `VERBOSE_ON=0` builds can compile prints out.

## Data Type Choices

- `float` samples: Vibration data comes from IMU values already scaled into g and degrees/second.
- `uint32_t seq`: Monotonic sequence IDs let callers tell whether a peak is new.
- Fixed buffers: Avoid heap allocation and unpredictable timing inside flight firmware.
- `bool motorsActive`: Analysis can ignore bench/static samples that do not represent motor vibration.

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

