/*
  ToFSensor.h
  Optional Adafruit VL53L4CX wrapper for the shared I2C bus used by BMP280.

  Wiring: VIN->3.3V, GND->GND, SDA->GPIO21, SCL->GPIO22.
  The BMP280 can stay on the same I2C bus because it uses address 0x76/0x77;
  VL53L4CX usually uses 0x29.

  To enable actual hardware reads:
    1. Install the Adafruit VL53L4CX Arduino library.
    2. Change TOF_ENABLE_VL53L4CX from 0 to 1 below.
*/

#pragma once
#ifndef TOF_SENSOR_H
#define TOF_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

#define TOF_ENABLE_VL53L4CX 0

#if TOF_ENABLE_VL53L4CX
#include <Adafruit_VL53L4CX.h>
#endif

struct ToFData {
    float distance_m = 0.0f;
    bool valid = false;
    uint32_t ts_ms = 0;
};

class ToFSensor {
public:
    explicit ToFSensor(TwoWire& wire = Wire);
    bool begin(int sdaPin = 21, int sclPin = 22, uint32_t i2cHz = 400000);
    bool read(ToFData& out);
    bool isConnected() const { return _begun; }

private:
    TwoWire& _wire;
    bool _begun;
#if TOF_ENABLE_VL53L4CX
    Adafruit_VL53L4CX _vl53;
#endif
};

extern ToFSensor tofSensor;

#endif // TOF_SENSOR_H
