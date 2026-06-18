/**
 * ============================================================
 * SpectrumAnalyzer.h — lightweight onboard vibration spectrum
 * ESP32 / Arduino, no heap allocation, FreeRTOS safe snapshot
 * ============================================================
 * Captures two scalar vibration signals at the control-loop rate:
 *   gyro vibration  = sqrt(gx^2 + gy^2 + gz^2)
 *   accel vibration = abs(sqrt(ax^2 + ay^2 + az^2) - 1g)
 *
 * It computes a Hann-windowed DFT on request for /spectrum.
 * N=128 at 400 Hz gives 3.125 Hz/bin and 0–200 Hz coverage.
 * This is enough to find the dominant motor/prop vibration peak
 * and tune the notch filter without adding a large FFT library.
 */
#pragma once
#ifndef SPECTRUM_ANALYZER_H
#define SPECTRUM_ANALYZER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

class SpectrumAnalyzer {
public:
    static constexpr uint16_t N = 128;
    static constexpr uint16_t BIN_COUNT = N / 2;

    explicit SpectrumAnalyzer(float sampleHz = 400.0f);

    void setSampleRate(float sampleHz);
    void reset();

    // Call from the 400 Hz control loop after IMU filtering/notch input is available.
    void push(float ax_g, float ay_g, float az_g,
              float gx_dps, float gy_dps, float gz_dps,
              bool motorsRunning);

    // JSON for GET /spectrum. Safe to call from Wi-Fi task.
    String toJson(float notchFreqHz, float notchQ, bool notchEnabled) const;

private:
    float _sampleHz;
    volatile uint16_t _head;
    volatile bool _full;
    volatile bool _motorsRunning;
    volatile uint32_t _sampleSeq;
    float _gyroVibe[N];
    float _accelVibe[N];
    mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;

    static float _hann(uint16_t i);
    static void _computeDft(const float* x, float* mag, float sampleMean);
};

#endif // SPECTRUM_ANALYZER_H
