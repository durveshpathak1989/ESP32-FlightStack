#include <cassert>
#include <cmath>

#include "Application/Control/CascadedController.h"

static bool near(float actual, float expected) {
    return std::fabs(actual - expected) < 0.00001f;
}

static TuningState tuningForTest() {
    TuningState tuning{};
    tuning.max_angle_deg = 20.0f;
    tuning.max_rate_dps = 300.0f;
    tuning.max_pitch_rate_dps = 200.0f;
    tuning.roll_output_limit = 1.0f;
    tuning.pitch_output_limit = 1.0f;
    tuning.yaw_output_limit = 1.0f;
    tuning.yaw_deadband = 0.02f;
    tuning.yaw_max_rate_dps = 20.0f;
    return tuning;
}

int main() {
    PidController rateRoll(0.01f, 0.0f, 0.0f);
    PidController ratePitch(0.01f, 0.0f, 0.0f);
    PidController rateYaw(0.01f, 0.0f, 0.0f);
    PidController angleRoll(2.0f, 0.0f, 0.0f);
    PidController anglePitch(2.0f, 0.0f, 0.0f);
    PidController angleYaw(1.0f, 0.0f, 0.0f);
    CascadedController controller(rateRoll, ratePitch, rateYaw,
                                  angleRoll, anglePitch, angleYaw);
    const TuningState tuning = tuningForTest();

    // ANGLE: stick becomes angle, then angle error becomes a rate request.
    const auto angle = controller.update(
        {0.5f, -0.25f, 0.0f, 2.0f, -1.0f, 30.0f,
         4.0f, -2.0f, 1.0f, 0.01f, 0.0f, true, true}, tuning);
    assert(near(angle.targetRollDeg, 10.0f));
    assert(near(angle.targetPitchDeg, -5.0f));
    assert(near(angle.targetRollRateDps, 16.0f));
    assert(near(angle.targetPitchRateDps, -8.0f));
    assert(near(angle.rollCorrection, 0.12f));
    assert(near(angle.pitchCorrection, -0.06f));
    assert(controller.yawHoldActive());
    assert(near(controller.yawSetpointDeg(), 30.0f));

    controller.reset();
    // ACRO: attitude is bypassed; filtered gyro rates close the inner loop.
    const auto acro = controller.update(
        {0.5f, -0.25f, 0.5f, 70.0f, -40.0f, 10.0f,
         100.0f, -20.0f, 2.0f, 0.01f, 0.0f, false, true}, tuning);
    assert(near(acro.targetRollRateDps, 150.0f));
    assert(near(acro.targetPitchRateDps, -50.0f));
    assert(near(acro.rollCorrection, 0.5f));
    assert(near(acro.pitchCorrection, -0.3f));
    assert(near(acro.targetYawRateDps, -10.0f));
    assert(near(acro.yawCorrection, -0.12f));
    assert(!controller.yawHoldActive());
}
