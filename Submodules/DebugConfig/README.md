# Debug Print Configuration

## Purpose

Central build-time control for Serial debug output. `VERBOSE_ON=0` compiles debug prints out while preserving Wi-Fi telemetry and calibration log storage.

## Files

- `DebugConfig.h`: Macro definitions for `DBG_PRINT`, `DBG_PRINTLN`, and `DBG_PRINTF`.

## Quick Start

```cpp
// Default: verbose Serial output enabled.
#include "DebugConfig.h"

DBG_PRINTLN(F("Boot message"));

// Quiet build:
// arduino-cli compile ... --build-property compiler.cpp.extra_flags=-DVERBOSE_ON=0
```

## How It Fits Into The Flight Controller

This library lives under `Submodules/DebugConfig` in the main `Test_Quad` firmware
and is built as an Arduino library by adding `Submodules/` to the Arduino
library search path. The main firmware includes it directly from
`RC_FlightController.ino` or from another support module.

The flight controller runs a 400 Hz control loop on ESP32, so this library
should avoid heap allocation, long blocking calls, and unbounded Serial output
inside flight-critical paths. Debug output should use `DebugConfig.h` macros
where available so `VERBOSE_ON=0` builds can compile prints out.

## Data Type Choices

- Preprocessor macros: The compiler removes quiet-mode print calls completely, reducing timing jitter and binary size.
- `((void)0)`: Disabled macros remain valid in expression contexts such as ternary branches.
- `F()` strings: Callers may still store constant text in flash when verbose mode is enabled.

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

