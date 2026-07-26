#include "src/Submodules/FlightApplication/FlightApplication.h"

// The Arduino entry point is intentionally composition-only.
// All services and real-time tasks are owned by FlightApplication.
void setup()
{
    flightApplicationSetup();
}

void loop()
{
    flightApplicationRun();
}
