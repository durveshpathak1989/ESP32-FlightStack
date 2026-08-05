#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Typed ESP32 Runtime Environment channel. It owns synchronization for one
// shared snapshot and provides bounded, atomic copies to readers.
template <typename Snapshot>
class SnapshotRte {
public:
    explicit SnapshotRte(Snapshot& storage) : storage_(storage) {}

    bool begin() {
        mutex_ = xSemaphoreCreateMutex();
        return mutex_ != nullptr;
    }
    bool lock(TickType_t maximumWait) {
        return mutex_ != nullptr &&
               xSemaphoreTake(mutex_, maximumWait) == pdTRUE;
    }
    void unlock() { xSemaphoreGive(mutex_); }
    bool read(Snapshot& destination, TickType_t maximumWait) {
        if (!lock(maximumWait)) return false;
        destination = storage_;
        unlock();
        return true;
    }
    Snapshot& writerData() { return storage_; }

private:
    Snapshot& storage_;
    SemaphoreHandle_t mutex_ = nullptr;
};
