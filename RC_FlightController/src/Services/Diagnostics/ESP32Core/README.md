# Test Quad ESP32Core Library

## Explain It Simply

This module watches how busy the ESP32 is. It helps you see if one processor core is working too hard while the drone is flying.

This module contains ESP32 support utilities such as CPU utilization tracking.

## Pin Map

No external GPIO pins are used. CPU utilization is measured from ESP32 runtime hooks/tasks.

## Main INO Integration Example

```cpp
#include "CPUUtilization.h"

void setup() {
    cpuUtilization.begin(1000);
}

void loop() {
    CPUUtilizationData cpu = cpuUtilization.get();
    // Use cpu.core0_pct and cpu.core1_pct in telemetry.
}
```


## Why These Data Types

Percentages use `float` because utilization is averaged over a measurement window. Millisecond windows use unsigned integer timing because they are monotonic counters from the ESP32 runtime.
