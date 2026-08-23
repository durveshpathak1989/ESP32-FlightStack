# Test Quad MadgwickAHRS Library

## Explain It Simply

This module is another way to estimate drone attitude from motion sensor data. It is kept as a separate reference version so it can be studied by itself.

This standalone repository provides the `MadgwickAHRS` estimator. It shares `AHRSInput` and `AttitudeEstimate` with the other attitude filters.

## Pin Map

Madgwick does not use pins directly. It consumes IMU data:

| Signal | ESP32 pin | Notes |
| --- | ---: | --- |
| SPI SCK | GPIO 5 | MPU-9250/MPU-6500 clock |
| SPI MISO | GPIO 19 | MPU data to ESP32 |
| SPI MOSI | GPIO 18 | ESP32 data to MPU |
| MPU CS | GPIO 33 | Chip select passed to `MPU9250 imu(PIN_MPU_CS)` |
| MPU INT | GPIO 27 | Optional data-ready interrupt; current firmware does not require it |
| Motor FL | GPIO 25 | Front-left ESC signal |
| Motor FR | GPIO 15 | Front-right ESC signal |
| Motor RL | GPIO 14 | Rear-left ESC signal |
| Motor RR | GPIO 32 | Rear-right ESC signal |
| iBUS RX | GPIO 16 | FS-iA6B iBUS TX into ESP32 UART2 RX |
| iBUS TX | GPIO 4 | Spare UART TX; avoids GPIO17 GPS conflict |
| I2C SDA | GPIO 21 | BMP280 and VL53L4CX ToF bus |
| I2C SCL | GPIO 22 | BMP280 and VL53L4CX ToF bus |
| GPS RX | GPIO 13 | GPS TXD into ESP32 UART1 RX |
| GPS TX | GPIO 17 | Optional GPS RXD from ESP32 UART1 TX |


## Main INO Integration Example

```cpp
#include "MadgwickAHRS.h"

MadgwickAHRS madgwickAHRS;

void setup() {
    madgwickAHRS.setBeta(0.08f);
}

void loop() {
    AHRSInput in;
    AttitudeEstimate att;
    // Fill in from MPU_SensorData.
    madgwickAHRS.update(in, 0.0025f, att);
}
```


## Why These Data Types

The beta gain and quaternion state are `float` because the update is math-heavy and runs in the high-rate control loop. `AHRSInput` makes unit expectations explicit at the API boundary.
