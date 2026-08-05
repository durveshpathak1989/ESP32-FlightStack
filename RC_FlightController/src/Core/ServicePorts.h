#pragma once

#include <cstddef>
#include <cstdint>

namespace rte {

// Client/server ports: application/platform SWCs call these contracts; only
// platform adapters include Arduino, ESP-IDF, FreeRTOS, or concrete drivers.
class ClockServicePort {
public:
    virtual ~ClockServicePort() = default;
    virtual std::uint64_t microseconds() const = 0;
    virtual std::uint32_t milliseconds() const = 0;
};

class CoreServicePort {
public:
    virtual ~CoreServicePort() = default;
    virtual std::uint8_t currentCore() const = 0;
    virtual void delayMilliseconds(std::uint32_t durationMs) = 0;
    virtual void yield() = 0;
};

struct CpuLoadSample {
    float core0Percent;
    float core1Percent;
    bool valid;
};

class CpuLoadServicePort {
public:
    virtual ~CpuLoadServicePort() = default;
    virtual bool InitMonitor(std::uint32_t samplePeriodMs) = 0;
    virtual CpuLoadSample ReadLoad() = 0;
};

class WifiServicePort {
public:
    virtual ~WifiServicePort() = default;
    virtual bool InitAccessPoint(const char* ssid, const char* password) = 0;
    virtual void PeriodicService() = 0;
};

class PersistentStorageServicePort {
public:
    virtual ~PersistentStorageServicePort() = default;
    virtual bool load(const char* key, void* destination,
                      std::size_t destinationSize) = 0;
    virtual bool save(const char* key, const void* source,
                      std::size_t sourceSize) = 0;
    virtual bool erase(const char* key) = 0;
};

}  // namespace rte
