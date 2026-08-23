# Mcal — Microcontroller Abstraction Layer

AUTOSAR layer 4 (lowest). This layer owns direct hardware access.

## Current status

The ESP32 Arduino Core APIs act as the de-facto MCAL today:

| Peripheral | API used by drivers above |
|---|---|
| SPI | `SPI.h` (IMU) |
| I2C | `Wire.h` (BMP280, ToF) |
| UART | `HardwareSerial.h` (GPS, iBUS receiver) |
| PWM | `ledc*` / analogWrite (ESC outputs) |
| ADC | `analogRead` via battery divider |
| Timer | `esp_timer.h` (400 Hz control tick) |
| NVM | `Preferences.h` (calibration storage) |

Driver code in `EcuAbstraction/` is the only place these may appear.
Application, Services and Core layers must never include them.

## Target state

When the project is ported to another MCU (STM32, RP2040), implement
thin adapters here exposing the same primitives (SpiBus, I2cBus, Uart,
PwmOut, AdcIn, TickTimer, NonVolatileStore). Only this folder changes;
everything above compiles unchanged, satisfying the dependency rule in
`docs/ARCHITECTURE.md`.
