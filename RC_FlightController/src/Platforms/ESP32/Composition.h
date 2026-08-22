#pragma once

#include "Application/FlightApplication.h"
#include "Platforms/ESP32/ESP32Clock.h"
#include "Platforms/ESP32/ESP32Preferences.h"
#include "Platforms/ESP32/ESP32WiFiAdapter.h"

struct ESP32Composition {
    ESP32Clock clock;
    ESP32PreferencesStore preferences;
    ESP32WiFiAdapter wifi;
    FlightApplication application;

    void begin()
    {
        wifi.begin();
    }
};
