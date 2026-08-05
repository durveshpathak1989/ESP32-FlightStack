#pragma once

#include <cstddef>
#include <cstdint>

#include "FlightTypes.h"

namespace flight {

// Ports are stable contracts owned by the portable flight core. A new board,
// sensor, receiver, or motor protocol is added by implementing these contracts.
// Contract details: docs/SWC_INTERFACES.md

class ClockPort {
public:
    virtual ~ClockPort() = default;
    virtual std::uint64_t microseconds() const = 0;
    virtual std::uint32_t milliseconds() const = 0;
};

class ImuPort {
public:
    virtual ~ImuPort() = default;
    virtual bool begin() = 0;
    virtual bool read(ImuSample& sample) = 0;
};

class ReceiverPort {
public:
    virtual ~ReceiverPort() = default;
    virtual bool begin() = 0;
    virtual PilotCommand read() = 0;
};

class MotorPort {
public:
    virtual ~MotorPort() = default;
    virtual bool begin() = 0;
    virtual void write(const MotorCommand& command) = 0;
    virtual void stop() = 0;
};

class ConfigurationStorePort {
public:
    virtual ~ConfigurationStorePort() = default;
    virtual bool load(const char* key, void* destination,
                      std::size_t destinationSize) = 0;
    virtual bool save(const char* key, const void* source,
                      std::size_t sourceSize) = 0;
    virtual bool erase(const char* key) = 0;
};

class DiagnosticPort {
public:
    virtual ~DiagnosticPort() = default;
    virtual void info(const char* message) = 0;
    virtual void warning(const char* message) = 0;
    virtual void error(const char* message) = 0;
};

}  // namespace flight
