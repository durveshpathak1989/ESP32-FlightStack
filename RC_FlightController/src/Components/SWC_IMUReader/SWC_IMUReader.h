#pragma once

#include <cstdint>

#include "Services/Estimation/SWC_EKFAHRS.h"

struct SWC_IMUReaderInput {
    float axG = 0.0f;
    float ayG = 0.0f;
    float azG = 1.0f;
    float gxDps = 0.0f;
    float gyDps = 0.0f;
    float gzDps = 0.0f;
    float mx = 0.0f;
    float my = 0.0f;
    float mz = 0.0f;
    bool accelValid = true;
    bool gyroValid = true;
    bool magValid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_IMUReaderOutput {
    SWC_EKFAHRSInput::RawImu imu;
    bool valid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_IMUReader {
public:
    void init(const SWC_IMUReaderInput&) {}
    SWC_IMUReaderOutput update(const SWC_IMUReaderInput& input);
    void reset() {}
};
