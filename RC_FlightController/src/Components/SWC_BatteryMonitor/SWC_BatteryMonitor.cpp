#include "SWC_BatteryMonitor.h"

#include <algorithm>

namespace {

float clampf(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

bool validReading(std::uint16_t raw, std::uint32_t mv, float cellVoltage)
{
    if (raw < 8 || mv < 50 || mv > 3100) return false;
    return cellVoltage >= 2.50f && cellVoltage <= 4.35f;
}

float estimatePercent(float cellVoltage)
{
    if (cellVoltage >= 4.20f) return 100.0f;
    if (cellVoltage <= 3.30f) return 0.0f;
    return clampf((cellVoltage - 3.30f) / (4.20f - 3.30f) * 100.0f, 0.0f, 100.0f);
}

}  // namespace

SWC_BatteryMonitorOutput SWC_BatteryMonitor::update(const SWC_BatteryMonitorInput& input)
{
    SWC_BatteryMonitorOutput out;
    out.timestampUs = input.timestampUs;
    out.adcVoltage = static_cast<float>(input.adcMillivolts) * 0.001f;
    const float measuredBattery = out.adcVoltage * input.dividerScale;
    const std::uint8_t cells = input.cellCount == 0 ? 1 : input.cellCount;
    const float measuredCell = measuredBattery / static_cast<float>(cells);
    out.valid = validReading(input.rawAdcCount, input.adcMillivolts, measuredCell);

    if (out.valid) {
        if (!_haveFiltered) {
            _filteredVoltage = measuredBattery;
            _haveFiltered = true;
        } else {
            _filteredVoltage += 0.08f * (measuredBattery - _filteredVoltage);
        }
    } else {
        _filteredVoltage = input.fallbackVoltage;
        _haveFiltered = false;
    }

    out.cellVoltage = _filteredVoltage / static_cast<float>(cells);
    out.percent = estimatePercent(out.cellVoltage);
    out.battery.voltage = _filteredVoltage;
    out.battery.current = input.currentA;
    out.battery.capacity = input.capacityMah;
    out.battery.cellCount = cells;
    out.battery.timestampUs = input.timestampUs;

    if (!out.valid) {
        out.battery.status = flight::BatteryState::DISCONNECTED;
    } else if (out.cellVoltage <= 3.40f) {
        out.battery.status = flight::BatteryState::CRITICAL;
    } else if (out.cellVoltage <= 3.55f) {
        out.battery.status = flight::BatteryState::LOW_VOLTAGE;
    } else {
        out.battery.status = flight::BatteryState::HEALTHY;
    }
    return out;
}

void SWC_BatteryMonitor::reset()
{
    _filteredVoltage = 11.1f;
    _haveFiltered = false;
}
