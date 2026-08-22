#include "ESP32Preferences.h"

#include <Preferences.h>

ESP32PreferencesStore::ESP32PreferencesStore(const char* nsName)
    : _namespace(nsName)
{
}

bool ESP32PreferencesStore::load(const char* key, void* destination, std::size_t destinationSize)
{
    Preferences prefs;
    if (!prefs.begin(_namespace, true)) return false;
    const std::size_t read = prefs.getBytes(key, destination, destinationSize);
    prefs.end();
    return read == destinationSize;
}

bool ESP32PreferencesStore::save(const char* key, const void* source, std::size_t sourceSize)
{
    Preferences prefs;
    if (!prefs.begin(_namespace, false)) return false;
    const std::size_t written = prefs.putBytes(key, source, sourceSize);
    prefs.end();
    return written == sourceSize;
}

bool ESP32PreferencesStore::erase(const char* key)
{
    Preferences prefs;
    if (!prefs.begin(_namespace, false)) return false;
    const bool removed = prefs.remove(key);
    prefs.end();
    return removed;
}
