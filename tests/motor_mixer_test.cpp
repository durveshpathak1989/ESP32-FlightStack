#include <cassert>
#include <cmath>

#include "Application/Control/MotorMixer.h"

static bool near(float actual, float expected) {
    return std::fabs(actual - expected) < 0.00001f;
}

int main() {
    const MotorMixerConfig config{0.0f, 100.0f, 100.0f, 0.08f, 1.0f, 0.03f, 0.15f};
    MotorMixer mixer;
    auto nominal = mixer.update({0.5f, 0.1f, 0.05f, 0.02f, 0.01f}, config);
    assert(near(nominal.shapedThrottle, 0.5f));
    assert(near(nominal.frontLeft, 0.53f));
    assert(near(nominal.frontRight, 0.37f));
    assert(near(nominal.rearLeft, 0.67f));
    assert(near(nominal.rearRight, 0.43f));
    assert(!nominal.saturated);

    mixer.reset();
    auto saturated = mixer.update({1.0f, 0.2f, 0.2f, 0.2f, 0.01f}, config);
    assert(saturated.saturated);
    assert(near(saturated.maximumBeforeDesaturation, 1.6f));
    assert(near(saturated.frontLeft, 0.2f));
    assert(near(saturated.frontRight, 0.2f));
    assert(near(saturated.rearLeft, 1.0f));
    assert(near(saturated.rearRight, 0.2f));

    mixer.reset();
    auto cut = mixer.update({0.02f, 0.1f, 0.1f, 0.1f, 0.01f}, config);
    assert(near(cut.frontLeft, 0.0f));
    assert(near(cut.frontRight, 0.0f));
    assert(near(cut.rearLeft, 0.0f));
    assert(near(cut.rearRight, 0.0f));
}
