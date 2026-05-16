/**
 * ============================================================
 *  CPUUtilization.h — ESP32 dual-core CPU load monitor
 *  FreeRTOS idle-hook based estimate for Core 0 and Core 1
 * ============================================================
 *
 *  How it works:
 *    • ESP32 calls an idle hook whenever a core has nothing else to run.
 *    • This module counts idle-hook hits per core.
 *    • The highest observed idle count becomes the local 0% load baseline.
 *    • Current load ≈ 100 * (1 - currentIdle / bestIdleBaseline).
 *
 *  Notes:
 *    • This is a lightweight estimate, not a lab-grade profiler.
 *    • Accuracy improves after the first few seconds of runtime.
 *    • It is useful for ground-station health monitoring.
 * ============================================================
 */

#pragma once
#ifndef CPU_UTILIZATION_H
#define CPU_UTILIZATION_H

#include <Arduino.h>

struct CPUUtilizationData {
    float core0_pct;
    float core1_pct;
    bool  valid;
    uint32_t sample_ms;
};

class CPUUtilization {
public:
    CPUUtilization();

    /** Register ESP32 FreeRTOS idle hooks. Call once in setup(). */
    bool begin(uint32_t sampleMs = 1000);

    /** Update the load estimate. Call periodically from any task. */
    void update();

    /** Copy the latest CPU load values. */
    CPUUtilizationData get() const;

    float core0() const;
    float core1() const;
    bool  isValid() const;

private:
    uint32_t _sampleMs;
    uint32_t _lastMs;
    uint32_t _lastIdle0;
    uint32_t _lastIdle1;
    uint32_t _bestIdle0;
    uint32_t _bestIdle1;
    float _core0Pct;
    float _core1Pct;
    bool  _valid;
    bool  _hooksRegistered;
};

extern CPUUtilization cpuUtilization;

#endif // CPU_UTILIZATION_H
