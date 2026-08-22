#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"
#include "Services/Estimation/SWC_EKFAHRS.h"

class Mahony_Extract {
public:
    void setGains(float kp, float ki);
    void reset();
    flight::AttitudeState update(const SWC_EKFAHRSInput::RawImu& imu,
                                 std::uint64_t dtUs,
                                 std::uint64_t timestampUs);

private:
    float _q0 = 1.0f;
    float _q1 = 0.0f;
    float _q2 = 0.0f;
    float _q3 = 0.0f;
    float _ix = 0.0f;
    float _iy = 0.0f;
    float _iz = 0.0f;
    float _kp = 1.0f;
    float _ki = 0.005f;
};
