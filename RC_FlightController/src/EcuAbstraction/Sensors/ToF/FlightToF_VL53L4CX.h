#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "vl53l4cx_class.h"

/*
 * FlightToF_VL53L4CX
 * Lightweight non-blocking wrapper around the STM32duino/ST VL53L4CX driver.
 *
 * Designed for ESP32 flight-controller use:
 * - call begin() once during setup/init
 * - call update() from a slow sensor task, for example 20 Hz to 50 Hz
 * - read cached distance from the 400 Hz control loop
 *
 * Required dependency:
 * - STM32duino VL53L4CX library, or your local VL53L4CX_stm repo src folder
 */

struct FlightToFReading {
    bool valid = false;
    bool newData = false;

    uint16_t distanceMm = 0;
    uint8_t rangeStatus = 255;
    uint8_t objectCount = 0;
    uint8_t streamCount = 0;

    float signalMcps = 0.0f;
    float ambientMcps = 0.0f;

    uint32_t lastUpdateMs = 0;
};

class FlightVL53L4CXDriver : public VL53L4CX {
public:
    FlightVL53L4CXDriver(TwoWire* wire, int xshutPin)
        : VL53L4CX(wire, xshutPin)
    {
    }

    int setInterMeasurementPeriodMs(uint32_t periodMs)
    {
        return VL53L4CX_set_inter_measurement_period_ms(Dev, periodMs);
    }
};

class FlightToF_VL53L4CX {
public:
    enum DistanceMode : uint8_t {
        DISTANCE_SHORT = 1,
        DISTANCE_MEDIUM = 2,
        DISTANCE_LONG = 3
    };

    FlightToF_VL53L4CX(TwoWire& wire = Wire, int xshutPin = -1);

    /*
     * stAddress uses the ST-driver style address.
     * The STM32duino VL53L4CX example uses 0x12.
     */
    bool begin(uint8_t stAddress = 0x12,
               DistanceMode distanceMode = DISTANCE_MEDIUM,
               uint32_t timingBudgetUs = 20000,
               uint32_t interMeasurementMs = 25);

    /*
     * Non-blocking.
     * Returns true only when a fresh measurement was copied into _reading.
     */
    bool update();

    void stop();
    void powerOff();
    void powerOn();

    bool isReady() const { return _initialized; }
    bool isValid() const { return _reading.valid; }
    bool hasNewData() const { return _reading.newData; }

    uint16_t distanceMm() const { return _reading.distanceMm; }
    float distanceM() const { return _reading.distanceMm * 0.001f; }

    uint32_t ageMs() const {
        if (_reading.lastUpdateMs == 0) {
            return UINT32_MAX;
        }
        return millis() - _reading.lastUpdateMs;
    }

    const FlightToFReading& reading() const { return _reading; }

private:
    TwoWire* _wire = nullptr;
    int _xshutPin = -1;

    FlightVL53L4CXDriver _sensor;
    FlightToFReading _reading;

    bool _initialized = false;

    static bool isGoodRangeStatus(uint8_t status);
};
