#include "SpectrumAnalyzer.h"
#include <math.h>

SpectrumAnalyzer::SpectrumAnalyzer(float sampleHz)
    : _sampleHz(sampleHz), _head(0), _full(false), _motorsRunning(false), _sampleSeq(0)
{
    for (uint16_t i = 0; i < N; i++) { _gyroVibe[i] = 0.0f; _accelVibe[i] = 0.0f; }
}

void SpectrumAnalyzer::setSampleRate(float sampleHz)
{
    if (sampleHz > 10.0f) _sampleHz = sampleHz;
}

void SpectrumAnalyzer::reset()
{
    portENTER_CRITICAL(&_mux);
    _head = 0; _full = false; _sampleSeq = 0; _motorsRunning = false;
    for (uint16_t i = 0; i < N; i++) { _gyroVibe[i] = 0.0f; _accelVibe[i] = 0.0f; }
    portEXIT_CRITICAL(&_mux);
}

void SpectrumAnalyzer::push(float ax_g, float ay_g, float az_g,
                            float gx_dps, float gy_dps, float gz_dps,
                            bool motorsRunning)
{
    const float gyroMag = sqrtf(gx_dps*gx_dps + gy_dps*gy_dps + gz_dps*gz_dps);
    const float accelMag = sqrtf(ax_g*ax_g + ay_g*ay_g + az_g*az_g);
    const float accelVibe = fabsf(accelMag - 1.0f);

    portENTER_CRITICAL(&_mux);
    _gyroVibe[_head] = gyroMag;
    _accelVibe[_head] = accelVibe;
    _motorsRunning = motorsRunning;
    _sampleSeq++;
    if (++_head >= N) { _head = 0; _full = true; }
    portEXIT_CRITICAL(&_mux);
}

float SpectrumAnalyzer::_hann(uint16_t i)
{
    return 0.5f * (1.0f - cosf(2.0f * PI * (float)i / (float)(N - 1)));
}

void SpectrumAnalyzer::_computeDft(const float* x, float* mag, float sampleMean)
{
    // Hann-windowed DFT. Bin 0 is forced to 0 because DC is removed.
    mag[0] = 0.0f;
    for (uint16_t k = 1; k < BIN_COUNT; k++) {
        float re = 0.0f;
        float im = 0.0f;
        for (uint16_t n = 0; n < N; n++) {
            const float w = _hann(n);
            const float v = (x[n] - sampleMean) * w;
            const float a = 2.0f * PI * (float)k * (float)n / (float)N;
            re += v * cosf(a);
            im -= v * sinf(a);
        }
        // 2/N roughly normalizes one-sided magnitude.
        mag[k] = (2.0f / (float)N) * sqrtf(re*re + im*im);
    }
}

String SpectrumAnalyzer::toJson(float notchFreqHz, float notchQ, bool notchEnabled) const
{
    float gyro[N];
    float accel[N];
    float gyroMean = 0.0f;
    float accelMean = 0.0f;
    uint16_t head;
    bool full;
    bool motors;
    uint32_t seq;
    float fs;

    portENTER_CRITICAL(&_mux);
    head = _head; full = _full; motors = _motorsRunning; seq = _sampleSeq; fs = _sampleHz;
    const uint16_t count = full ? N : head;
    for (uint16_t i = 0; i < N; i++) {
        const uint16_t src = full ? ((head + i) % N) : i;
        gyro[i] = (i < count) ? _gyroVibe[src] : 0.0f;
        accel[i] = (i < count) ? _accelVibe[src] : 0.0f;
    }
    portEXIT_CRITICAL(&_mux);

    const uint16_t validCount = full ? N : head;
    if (validCount < N) {
        String j;
        j.reserve(160);
        j += "{\"ok\":false,\"ready\":false,\"samples\":";
        j += String(validCount);
        j += ",\"needed\":";
        j += String(N);
        j += ",\"sampleHz\":";
        j += String(fs, 1);
        j += "}";
        return j;
    }

    for (uint16_t i = 0; i < N; i++) { gyroMean += gyro[i]; accelMean += accel[i]; }
    gyroMean /= (float)N;
    accelMean /= (float)N;

    float gyroMag[BIN_COUNT];
    float accelMag[BIN_COUNT];
    _computeDft(gyro, gyroMag, gyroMean);
    _computeDft(accel, accelMag, accelMean);

    uint16_t peakBin = 1;
    float peak = gyroMag[1];
    for (uint16_t k = 2; k < BIN_COUNT; k++) {
        if (gyroMag[k] > peak) { peak = gyroMag[k]; peakBin = k; }
    }

    String j;
    j.reserve(3600);
    j += "{\"ok\":true,\"ready\":true";
    j += ",\"samples\":" + String(N);
    j += ",\"seq\":" + String(seq);
    j += ",\"sampleHz\":" + String(fs, 1);
    j += ",\"binHz\":" + String(fs / (float)N, 4);
    j += ",\"motorsRunning\":" + String(motors ? "true" : "false");
    j += ",\"notchEnable\":" + String(notchEnabled ? "true" : "false");
    j += ",\"notchFreqHz\":" + String(notchFreqHz, 3);
    j += ",\"notchQ\":" + String(notchQ, 3);
    j += ",\"peakGyroHz\":" + String((float)peakBin * fs / (float)N, 3);
    j += ",\"peakGyroMag\":" + String(peak, 6);
    j += ",\"bins\":[";
    for (uint16_t k = 1; k < BIN_COUNT; k++) {
        if (k > 1) j += ',';
        j += '{';
        j += "\"f\":" + String((float)k * fs / (float)N, 3);
        j += ",\"gyro\":" + String(gyroMag[k], 6);
        j += ",\"accel\":" + String(accelMag[k], 6);
        j += '}';
    }
    j += "]}";
    return j;
}
