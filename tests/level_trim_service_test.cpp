#include <cassert>
#include <cmath>

#include "Application/Calibration/LevelTrimService.h"

static bool near(float actual, float expected) {
    return std::fabs(actual - expected) < 0.00001f;
}

int main() {
    LevelTrimService trim;
    auto first = trim.update(true, true, 100, 1000, 2.0f, -4.0f, 179.0f);
    assert(first.captureStarted);
    assert(!first.captureCompleted);
    auto done = trim.update(true, true, 1100, 1000, 4.0f, -2.0f, 179.0f);
    assert(done.captureCompleted);
    assert(near(trim.rollOffsetDeg(), 3.0f));
    assert(near(trim.pitchOffsetDeg(), -3.0f));
    assert(near(trim.controlRollDeg(5.0f), 2.0f));
    assert(near(trim.controlPitchDeg(-1.0f), 2.0f));
    assert(near(trim.controlYawDeg(-179.0f), 2.0f));

    // A held switch is one-shot; lowering it rearms capture.
    auto held = trim.update(true, true, 2200, 1000, 20.0f, 20.0f, 20.0f);
    assert(!held.captureStarted && !held.captureCompleted);
    trim.update(false, false, 2300, 1000, 0.0f, 0.0f, 0.0f);
    auto rearmed = trim.update(true, true, 2400, 1000, 1.0f, 1.0f, 1.0f);
    assert(rearmed.captureStarted);
}
