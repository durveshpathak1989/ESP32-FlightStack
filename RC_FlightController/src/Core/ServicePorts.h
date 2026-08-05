#pragma once

#include <cstddef>
#include <cstdint>

#include "FlightTypes.h"

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

struct GpsServiceSample {
    double latitude;
    double longitude;
    float altitudeM;
    float geoidSeparationM;
    float speedKmh;
    float speedKnots;
    float courseDeg;
    float hdop;
    std::uint32_t sentenceCount;
    std::uint8_t satellites;
    std::uint8_t fixQuality;
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;
    std::uint8_t day;
    std::uint8_t month;
    std::uint16_t year;
    std::uint32_t lastFixMs;
    bool hasFix;
    bool valid;
};

class GpsServicePort {
public:
    virtual ~GpsServicePort() = default;
    virtual bool InitReceiver() = 0;
    virtual GpsServiceSample ReadPosition() = 0;
};

struct BarometerServiceSample {
    float temperatureC;
    float pressureHpa;
    float altitudeM;
    bool valid;
};

class BarometerServicePort {
public:
    virtual ~BarometerServicePort() = default;
    virtual bool InitSensor() = 0;
    virtual bool ReadSample(BarometerServiceSample& sample) = 0;
};

struct RangeServiceSample {
    std::uint16_t distanceMm;
    std::uint8_t rangeStatus;
    std::uint8_t objectCount;
    std::uint8_t streamCount;
    float signalMcps;
    float ambientMcps;
    std::uint32_t ageMs;
    std::uint32_t lastUpdateMs;
    bool ready;
    bool valid;
};

class RangeServicePort {
public:
    virtual ~RangeServicePort() = default;
    virtual bool InitSensor() = 0;
    virtual RangeServiceSample ReadRange() = 0;
};

class ReceiverServicePort {
public:
    virtual ~ReceiverServicePort() = default;
    virtual bool InitReceiver() = 0;
    virtual flight::ReceiverFrame ReadFrame() = 0;
};

enum class CalibrationRequest : std::uint8_t {
    ImuAllGuided,
    Esc
};

struct CalibrationServiceStatus {
    bool active;
    CalibrationRequest request;
};

class CalibrationServicePort {
public:
    virtual ~CalibrationServicePort() = default;
    virtual bool Request(CalibrationRequest request) = 0;
    virtual CalibrationServiceStatus Status() const = 0;
    virtual void Cancel() = 0;
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
