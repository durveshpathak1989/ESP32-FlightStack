#pragma once

#include <cmath>

// Deterministic first-order low-pass filter. One instance owns one signal's
// history; callers supply seconds and cutoff frequency in hertz.
class LowPassFilter {
public:
    float update(float input, float dtSeconds, float cutoffHz) {
        if (dtSeconds <= 0.0f || cutoffHz <= 0.0f) return input;
        if (!initialized_) {
            output_ = input;
            initialized_ = true;
            return output_;
        }
        constexpr float kTwoPi = 6.28318530717958647692f;
        const float timeConstant = 1.0f / (kTwoPi * cutoffHz);
        const float alpha = dtSeconds / (dtSeconds + timeConstant);
        output_ += alpha * (input - output_);
        return output_;
    }

    void reset() {
        output_ = 0.0f;
        initialized_ = false;
    }

private:
    float output_ = 0.0f;
    bool initialized_ = false;
};
