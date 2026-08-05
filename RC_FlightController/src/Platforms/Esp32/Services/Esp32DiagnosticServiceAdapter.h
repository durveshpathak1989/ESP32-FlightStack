#pragma once

#include <Arduino.h>

#include "../../../Core/ServicePorts.h"

class Esp32DiagnosticServiceAdapter final : public rte::DiagnosticServicePort {
public:
    bool CommandAvailable() const override { return Serial.available() > 0; }
    int ReadCommandByte() override { return Serial.read(); }
    void Write(const char* message) override { Serial.print(message); }
};
