#include <cassert>

#include "Application/Safety/FlightSafetyPolicy.h"

int main() {
    assert(FlightSafetyPolicy::controlMustStop(true, false, false));
    assert(FlightSafetyPolicy::controlMustStop(false, true, false));
    assert(FlightSafetyPolicy::controlMustStop(false, false, true));
    assert(!FlightSafetyPolicy::controlMustStop(false, false, false));

    assert(FlightSafetyPolicy::otaAllowed({false, 0.02f, 0, 0, 0, 0}));
    assert(!FlightSafetyPolicy::otaAllowed({true, 0.02f, 0, 0, 0, 0}));
    assert(!FlightSafetyPolicy::otaAllowed({false, 0.04f, 0, 0, 0, 0}));
    assert(!FlightSafetyPolicy::otaAllowed({false, 0.02f, 0, 0.01f, 0, 0}));
}
