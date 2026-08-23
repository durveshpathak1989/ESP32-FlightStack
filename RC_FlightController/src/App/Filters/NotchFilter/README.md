# Test Quad NotchFilter Library

## Explain It Simply

This module removes one bad shake frequency from the sensor data. It is like lowering one annoying buzzing sound so the drone can hear the real motion better.

`NotchFilter` removes narrow motor-vibration bands from accel and gyro streams before AHRS and PID processing.

## Pin Map

No pins are used directly. The filter processes IMU data in software.

## Main INO Integration Example

```cpp
#include "NotchFilter.h"

NotchFilter notchGx;

void setup() {
    notchGx.configure(400.0f, 120.0f, 5.0f); // sample Hz, center Hz, Q
}

void loop() {
    float filteredGx = notchGx.apply(s.gx_dps);
}
```


## Why These Data Types

Filter coefficients and samples use `float` because biquad math needs fractional precision. The configured sample rate, center frequency, and Q are kept explicit so telemetry tuning can move the notch without changing code.
