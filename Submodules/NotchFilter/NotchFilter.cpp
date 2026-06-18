#include "NotchFilter.h"

NotchFilter::NotchFilter()
    : _sampleRateHz(400.0f),
      _centerHz(90.0f),
      _q(8.0f),
      _enabled(true),
      _b0(1.0f), _b1(0.0f), _b2(0.0f),
      _a1(0.0f), _a2(0.0f),
      _z1(0.0f), _z2(0.0f)
{
    configure(_sampleRateHz, _centerHz, _q, _enabled);
}

void NotchFilter::configure(float sampleRateHz, float centerHz, float q, bool enabled)
{
    if (sampleRateHz < 10.0f) sampleRateHz = 10.0f;
    const float nyquist = 0.5f * sampleRateHz;

    // Keep the notch away from DC and Nyquist for numeric stability.
    centerHz = constrain(centerHz, 1.0f, nyquist * 0.90f);
    q        = constrain(q, 0.3f, 80.0f);

    _sampleRateHz = sampleRateHz;
    _centerHz     = centerHz;
    _q            = q;
    _enabled      = enabled;

    if (!_enabled) {
        _b0 = 1.0f; _b1 = 0.0f; _b2 = 0.0f;
        _a1 = 0.0f; _a2 = 0.0f;
        return;
    }

    const float w0 = 2.0f * PI * centerHz / sampleRateHz;
    const float c  = cosf(w0);
    const float alpha = sinf(w0) / (2.0f * q);

    const float b0 = 1.0f;
    const float b1 = -2.0f * c;
    const float b2 = 1.0f;
    const float a0 = 1.0f + alpha;
    const float a1 = -2.0f * c;
    const float a2 = 1.0f - alpha;

    _b0 = b0 / a0;
    _b1 = b1 / a0;
    _b2 = b2 / a0;
    _a1 = a1 / a0;
    _a2 = a2 / a0;
}

float NotchFilter::apply(float x)
{
    if (!_enabled) return x;

    // Direct Form II Transposed:
    // y  = b0*x + z1
    // z1 = b1*x - a1*y + z2
    // z2 = b2*x - a2*y
    const float y = _b0 * x + _z1;
    _z1 = _b1 * x - _a1 * y + _z2;
    _z2 = _b2 * x - _a2 * y;

    if (!isfinite(y) || !isfinite(_z1) || !isfinite(_z2)) {
        reset(x);
        return x;
    }
    return y;
}

void NotchFilter::reset(float value)
{
    // Initialize state so output starts near the input and does not kick the PID.
    _z1 = value * (1.0f - _b0);
    _z2 = value * (_a2 - _b2);
}
