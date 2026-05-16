/**
 * ============================================================
 *  CPUUtilization.cpp — ESP32 dual-core CPU load monitor
 * ============================================================
 */

#include "CPUUtilization.h"
#include "esp_freertos_hooks.h"

CPUUtilization cpuUtilization;

static volatile uint32_t s_idleCount0 = 0;
static volatile uint32_t s_idleCount1 = 0;

static bool idleHookCore0(void) {
    s_idleCount0++;
    return true;
}

static bool idleHookCore1(void) {
    s_idleCount1++;
    return true;
}

CPUUtilization::CPUUtilization()
    : _sampleMs(1000),
      _lastMs(0),
      _lastIdle0(0),
      _lastIdle1(0),
      _bestIdle0(0),
      _bestIdle1(0),
      _core0Pct(0.0f),
      _core1Pct(0.0f),
      _valid(false),
      _hooksRegistered(false)
{
}

bool CPUUtilization::begin(uint32_t sampleMs)
{
    _sampleMs = sampleMs < 250 ? 250 : sampleMs;
    _lastMs = millis();
    _lastIdle0 = s_idleCount0;
    _lastIdle1 = s_idleCount1;

    if (!_hooksRegistered) {
        bool ok0 = (esp_register_freertos_idle_hook_for_cpu(idleHookCore0, 0) == ESP_OK);
        bool ok1 = (esp_register_freertos_idle_hook_for_cpu(idleHookCore1, 1) == ESP_OK);
        _hooksRegistered = ok0 && ok1;
    }

    return _hooksRegistered;
}

void CPUUtilization::update()
{
    uint32_t now = millis();
    uint32_t elapsed = now - _lastMs;
    if (elapsed < _sampleMs) return;

    uint32_t idle0 = s_idleCount0;
    uint32_t idle1 = s_idleCount1;
    uint32_t d0 = idle0 - _lastIdle0;
    uint32_t d1 = idle1 - _lastIdle1;

    _lastIdle0 = idle0;
    _lastIdle1 = idle1;
    _lastMs = now;

    // Track the best observed idle rate. This becomes our local 0% CPU-load baseline.
    if (d0 > _bestIdle0) _bestIdle0 = d0;
    if (d1 > _bestIdle1) _bestIdle1 = d1;

    if (_bestIdle0 > 0 && _bestIdle1 > 0) {
        float load0 = 100.0f * (1.0f - ((float)d0 / (float)_bestIdle0));
        float load1 = 100.0f * (1.0f - ((float)d1 / (float)_bestIdle1));
        _core0Pct = constrain(load0, 0.0f, 100.0f);
        _core1Pct = constrain(load1, 0.0f, 100.0f);
        _valid = true;
    }
}

CPUUtilizationData CPUUtilization::get() const
{
    CPUUtilizationData d;
    d.core0_pct = _core0Pct;
    d.core1_pct = _core1Pct;
    d.valid = _valid;
    d.sample_ms = _sampleMs;
    return d;
}

float CPUUtilization::core0() const { return _core0Pct; }
float CPUUtilization::core1() const { return _core1Pct; }
bool CPUUtilization::isValid() const { return _valid; }
