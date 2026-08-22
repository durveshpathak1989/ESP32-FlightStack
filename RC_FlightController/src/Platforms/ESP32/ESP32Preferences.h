#pragma once

#include <cstddef>

#include "Core/Ports.h"

class ESP32PreferencesStore final : public flight::ConfigurationStorePort {
public:
    explicit ESP32PreferencesStore(const char* nsName = "archv5");
    bool load(const char* key, void* destination, std::size_t destinationSize) override;
    bool save(const char* key, const void* source, std::size_t sourceSize) override;
    bool erase(const char* key) override;

private:
    const char* _namespace;
};
