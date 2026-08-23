# Test Quad DebugConfig Library

## Explain It Simply

This module is the print switch. When `VERBOSE_ON` is 1, the drone talks on Serial for debugging. When it is 0, most prints are turned off so the flight loop stays quiet and steady.

`DebugConfig` centralizes build-time logging control. Build with `-DVERBOSE_ON=0` to remove normal debug prints while preserving WiFi telemetry needed for calibration.

## Pin Map

No GPIO pins are used by this library.

## Main INO Integration Example

```cpp
#include "DebugConfig.h"

void setup() {
    DBG_PRINTLN(F("[BOOT] Debug print enabled when VERBOSE_ON is 1"));
}

void loop() {
    DBG_PRINTF("[LOOP] millis=%lu\n", (unsigned long)millis());
}
```


## Build Examples

```bash
arduino-cli compile --build-property compiler.cpp.extra_flags=-DVERBOSE_ON=1 .
arduino-cli compile --build-property compiler.cpp.extra_flags=-DVERBOSE_ON=0 .
```


## Why These Data Types

Preprocessor macros are used instead of runtime flags so disabled prints compile out of the firmware. That reduces timing jitter, flash use, and accidental `Serial` blocking in flight-critical loops.
