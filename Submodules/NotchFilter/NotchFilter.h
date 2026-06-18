#pragma once
#ifndef NOTCH_FILTER_H
#define NOTCH_FILTER_H

#include <Arduino.h>
#include <math.h>

/**
 * Runtime-tunable second-order IIR notch filter.
 *
 * Use this for narrow motor/prop vibration bands before the EKF and PID.
 * The filter is a normalized biquad:
 *   H(z) = (b0 + b1 z^-1 + b2 z^-2) / (1 + a1 z^-1 + a2 z^-2)
 *
 * Tuning:
 *   centerHz: vibration frequency to reject. Must be below Nyquist.
 *   q:        notch sharpness. Higher Q = narrower notch. Start 5–10.
 *   enabled:  false bypasses the filter without losing configuration.
 */
class NotchFilter {
public:
    NotchFilter();

    void configure(float sampleRateHz, float centerHz, float q, bool enabled = true);
    float apply(float x);
    void reset(float value = 0.0f);

    float sampleRateHz() const { return _sampleRateHz; }
    float centerHz() const { return _centerHz; }
    float q() const { return _q; }
    bool enabled() const { return _enabled; }

private:
    float _sampleRateHz;
    float _centerHz;
    float _q;
    bool  _enabled;

    // Normalized coefficients. a0 is always 1 after normalization.
    float _b0, _b1, _b2;
    float _a1, _a2;

    // Direct Form II Transposed state.
    float _z1, _z2;
};

#endif // NOTCH_FILTER_H
