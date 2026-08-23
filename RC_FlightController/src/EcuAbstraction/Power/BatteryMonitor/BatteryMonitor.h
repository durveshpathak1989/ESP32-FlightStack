#pragma once

#include <Arduino.h>

struct BatteryStatus {
    uint16_t rawAdcCount = 0;
    uint32_t adcMillivolts = 0;
    float adcVoltage_v = 0.0f;
    float batteryVoltage_v = 0.0f;
    float cellVoltage_v = 0.0f;
    float percent = 0.0f;
    bool valid = false;
    bool low = false;
    bool critical = false;
};

class BatteryMonitor {
public:
    bool begin(uint8_t pin,
               uint8_t cellCount,
               float rTopOhms,
               float rBottomOhms,
               float fallbackVoltage_v,
               float calibrationScale = 1.0f)
    {
        _pin = pin;
        _cellCount = (cellCount == 0) ? 1 : cellCount;
        _fallbackVoltage_v = fallbackVoltage_v;
        _dividerScale = ((rTopOhms + rBottomOhms) / rBottomOhms) * calibrationScale;
        _lastSampleMs = 0;
        _haveFiltered = false;

        analogSetPinAttenuation(_pin, ADC_11db);
        forceRead();
        return _status.valid;
    }

    bool update(uint32_t nowMs)
    {
        if ((uint32_t)(nowMs - _lastSampleMs) < SAMPLE_PERIOD_MS) {
            return false;
        }
        forceRead(nowMs);
        return true;
    }

    BatteryStatus forceRead()
    {
        return forceRead(millis());
    }

    BatteryStatus forceRead(uint32_t nowMs)
    {
        _lastSampleMs = nowMs;

        const uint16_t raw = (uint16_t)analogRead(_pin);
        const uint32_t mv = analogReadMilliVolts(_pin);
        const float adcVoltage = (float)mv * 0.001f;
        const float measuredBattery = adcVoltage * _dividerScale;
        const float measuredCell = measuredBattery / (float)_cellCount;
        const bool valid = isValidReading(raw, mv, measuredCell);

        _status.rawAdcCount = raw;
        _status.adcMillivolts = mv;
        _status.adcVoltage_v = adcVoltage;
        _status.valid = valid;

        if (valid) {
            if (!_haveFiltered) {
                _filteredBattery_v = measuredBattery;
                _haveFiltered = true;
            } else {
                _filteredBattery_v += FILTER_ALPHA * (measuredBattery - _filteredBattery_v);
            }
            _status.batteryVoltage_v = _filteredBattery_v;
        } else {
            _status.batteryVoltage_v = _fallbackVoltage_v;
        }

        _status.cellVoltage_v = _status.batteryVoltage_v / (float)_cellCount;
        _status.percent = estimatePercent(_status.cellVoltage_v);
        _status.low = _status.valid && (_status.cellVoltage_v <= LOW_CELL_V);
        _status.critical = _status.valid && (_status.cellVoltage_v <= CRITICAL_CELL_V);

        return _status;
    }

    const BatteryStatus& status() const
    {
        return _status;
    }

    float voltage() const
    {
        return _status.batteryVoltage_v;
    }

    float scale() const
    {
        return _dividerScale;
    }

private:
    struct PercentPoint {
        float cellVoltage_v;
        float percent;
    };

    static constexpr uint32_t SAMPLE_PERIOD_MS = 500;
    static constexpr float FILTER_ALPHA = 0.08f;
    static constexpr float LOW_CELL_V = 3.55f;
    static constexpr float CRITICAL_CELL_V = 3.40f;

    bool isValidReading(uint16_t raw, uint32_t mv, float cellVoltage_v) const
    {
        if (raw < 8 || mv < 50 || mv > 3100) {
            return false;
        }
        return (cellVoltage_v >= 2.50f && cellVoltage_v <= 4.35f);
    }

    static float estimatePercent(float cellVoltage_v)
    {
        static constexpr PercentPoint table[] = {
            {4.20f, 100.0f},
            {4.15f,  95.0f},
            {4.11f,  90.0f},
            {4.08f,  85.0f},
            {4.02f,  80.0f},
            {3.98f,  75.0f},
            {3.95f,  70.0f},
            {3.91f,  65.0f},
            {3.87f,  60.0f},
            {3.85f,  55.0f},
            {3.83f,  50.0f},
            {3.80f,  45.0f},
            {3.79f,  40.0f},
            {3.77f,  35.0f},
            {3.75f,  30.0f},
            {3.73f,  25.0f},
            {3.71f,  20.0f},
            {3.69f,  15.0f},
            {3.61f,  10.0f},
            {3.50f,   5.0f},
            {3.30f,   0.0f},
        };

        if (cellVoltage_v >= table[0].cellVoltage_v) {
            return table[0].percent;
        }

        const size_t count = sizeof(table) / sizeof(table[0]);
        for (size_t i = 1; i < count; ++i) {
            const PercentPoint& high = table[i - 1];
            const PercentPoint& low = table[i];
            if (cellVoltage_v >= low.cellVoltage_v) {
                const float span = high.cellVoltage_v - low.cellVoltage_v;
                const float t = (span > 0.0f) ? ((cellVoltage_v - low.cellVoltage_v) / span) : 0.0f;
                return low.percent + t * (high.percent - low.percent);
            }
        }

        return 0.0f;
    }

    uint8_t _pin = 0;
    uint8_t _cellCount = 3;
    float _dividerScale = 1.0f;
    float _fallbackVoltage_v = 11.1f;
    float _filteredBattery_v = 11.1f;
    uint32_t _lastSampleMs = 0;
    bool _haveFiltered = false;
    BatteryStatus _status;
};
