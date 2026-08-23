# Test Quad FFT Library

## Explain It Simply

This module listens for shaking. It looks at the IMU data and finds vibration frequencies, so the notch filter can quiet motor or propeller vibrations.

`SpectrumAnalyzer` collects IMU vibration samples and estimates dominant vibration frequencies for notch-filter tuning.

## Pin Map

No pins are owned by this module. It consumes IMU samples from the SPI IMU path and publishes spectrum data to telemetry.

## Main INO Integration Example

```cpp
#include "SpectrumAnalyzer.h"

SpectrumAnalyzer spectrumAnalyzer;

void loop() {
    spectrumAnalyzer.push(s.ax_g, s.ay_g, s.az_g,
                          s.gx_dps, s.gy_dps, s.gz_dps,
                          motorOutputsActive);

    float peakHz = 0.0f;
    float peakMag = 0.0f;
    uint32_t seq = 0;
    if (spectrumAnalyzer.findGyroPeak(40.0f, 180.0f, 0.01f,
                                      peakHz, peakMag, seq)) {
        // peakHz can guide notch filter frequency.
    }
}
```


## Why These Data Types

Samples use `float` because vibration energy is computed from scaled IMU units. Buffer indexes and sample counts use unsigned integer types to make wraparound/ring-buffer behavior explicit.
