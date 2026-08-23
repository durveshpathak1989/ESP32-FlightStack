# Test Quad BMP280 Library

## Explain It Simply

This module reads air pressure, like a tiny weather station. Because air pressure changes with height, the drone can use it to guess if it is going up or down.

This library reads pressure, temperature, and estimated altitude from a BMP280 over I2C.

## Pin Map

| BMP280 signal | ESP32 pin | Notes |
| --- | ---: | --- |
| SDA | GPIO 21 | I2C data |
| SCL | GPIO 22 | I2C clock |
| VCC | 3.3V | Tie CSB to 3.3V for I2C mode |
| GND | GND | Common ground |
| CSB | 3.3V | Selects I2C mode on common breakout boards |

## Main INO Integration Example

```cpp
#include "BMP280Sensor.h"

#define PIN_BMP_SDA 21
#define PIN_BMP_SCL 22

void setup() {
    bmp280.scanI2C(PIN_BMP_SDA, PIN_BMP_SCL, 100000);
    bmp280.beginAuto(PIN_BMP_SDA, PIN_BMP_SCL, 100000);
}

void loop() {
    BMP280Data b;
    if (bmp280.read(b)) {
        // b.temperature_c, b.pressure_hpa, b.altitude_m
    }
}
```


## Why These Data Types

Altitude, pressure, and temperature use `float` because barometric calculations are fractional. The `BMP280Data` struct groups one coherent sensor sample so telemetry and estimators do not mix fields from different read cycles.
