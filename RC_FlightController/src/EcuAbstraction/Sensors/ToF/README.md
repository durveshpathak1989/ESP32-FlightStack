# Test Quad ToF Library

## Explain It Simply

This module reads a tiny distance sensor. It can help the drone know how far it is from the ground when it is close to the ground.

`ToFSensor` reads a VL53L4CX time-of-flight range sensor on the shared I2C bus. It is intended for low-altitude sensing and future altitude-hold experiments.

## Pin Map

| ToF signal | ESP32 pin | Notes |
| --- | ---: | --- |
| SDA | GPIO 21 | Shared I2C data |
| SCL | GPIO 22 | Shared I2C clock |
| VCC | 3.3V | Use the voltage required by your breakout |
| GND | GND | Common ground |

## Main INO Integration Example

```cpp
#include "ToFSensor.h"

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

void setup() {
    tofSensor.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
}

void loop() {
    ToFData d;
    if (tofSensor.read(d) && d.valid) {
        // d.distance_m can be fused with barometer altitude later.
    }
}
```


## Why These Data Types

Range is exposed as `float` meters for direct fusion with barometer altitude. Validity is explicit because optical range sensors can fail from reflectivity, sunlight, or out-of-range targets.
