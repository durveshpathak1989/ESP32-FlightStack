#pragma once

#include <Preferences.h>
#include <cstddef>
#include <cstdint>

#include "../../../Application/Configuration/TuningState.h"

// ESP32 infrastructure adapter. Application code sees save/load operations;
// Preferences, namespaces, record layout, and integrity checks remain here.
class PreferencesTuningStore {
public:
    bool save(const TuningState& state) {
        Record record{};
        record.magic = kMagic;
        record.schema = kSchema;
        record.stateSize = sizeof(TuningState);
        record.state = state;
        record.state.dirty = false;
        record.crc32 = crc32(&record.state, sizeof(record.state));

        Preferences preferences;
        if (!preferences.begin(kNamespace, false)) return false;
        const std::size_t written =
            preferences.putBytes(kKey, &record, sizeof(record));
        Record verification{};
        const std::size_t read =
            preferences.getBytes(kKey, &verification, sizeof(verification));
        preferences.end();
        return written == sizeof(record) && read == sizeof(verification) &&
               valid(verification) && verification.crc32 == record.crc32;
    }

    bool load(TuningState& state) {
        Preferences preferences;
        if (!preferences.begin(kNamespace, true)) return false;
        if (preferences.getBytesLength(kKey) != sizeof(Record)) {
            preferences.end();
            return false;
        }
        Record record{};
        const std::size_t read =
            preferences.getBytes(kKey, &record, sizeof(record));
        preferences.end();
        if (read != sizeof(record) || !valid(record)) return false;
        state = record.state;
        state.dirty = true;
        return true;
    }

    bool erase() {
        Preferences preferences;
        if (!preferences.begin(kNamespace, false)) return false;
        const bool removed = preferences.remove(kKey);
        preferences.end();
        return removed;
    }

private:
    static constexpr std::uint32_t kMagic = 0x54554E45UL;
    static constexpr std::uint16_t kSchema = 1;
    static constexpr const char* kNamespace = "flightTune";
    static constexpr const char* kKey = "state";

    struct Record {
        std::uint32_t magic;
        std::uint16_t schema;
        std::uint16_t stateSize;
        TuningState state;
        std::uint32_t crc32;
    };

    static std::uint32_t crc32(const void* source, std::size_t length) {
        const auto* data = static_cast<const std::uint8_t*>(source);
        std::uint32_t crc = 0xFFFFFFFFUL;
        for (std::size_t index = 0; index < length; ++index) {
            crc ^= data[index];
            for (std::uint8_t bit = 0; bit < 8; ++bit)
                crc = (crc >> 1) ^
                      (0xEDB88320UL & (0UL - (crc & 1UL)));
        }
        return ~crc;
    }

    static bool valid(const Record& record) {
        return record.magic == kMagic && record.schema == kSchema &&
               record.stateSize == sizeof(TuningState) &&
               record.crc32 == crc32(&record.state, sizeof(record.state));
    }
};
