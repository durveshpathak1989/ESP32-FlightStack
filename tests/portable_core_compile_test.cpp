#include "Application/Control/PidController.h"
#include "Application/FlightApplication.h"
#include "Core/FlightTypes.h"
#include "Core/Ports.h"
#include "Services/Control/SWC_PIDController.h"
#include "Services/Navigation/SWC_FailsafeLogic.h"

class FakeClock final : public flight::ClockPort {
public:
    std::uint64_t microseconds() const override { return 1000; }
    std::uint32_t milliseconds() const override { return 1; }
};

int main() {
    FakeClock clock;
    PidController controller(1.0f, 0.1f, 0.01f);
    const float output = controller.update(0.5f, 0.01f);

    SWC_PIDController pidSwc;
    SWC_PIDControllerInput pidInput;
    pidInput.isArmed = false;
    const auto pidOutput = pidSwc.update(pidInput);

    SWC_FailsafeLogic failsafe;
    SWC_FailsafeLogicInput failsafeInput;
    failsafeInput.battery.voltage = 11.8f;
    failsafeInput.receiver.isConnected = true;
    failsafeInput.timestampUs = 1000;
    const auto failsafeOutput = failsafe.update(failsafeInput);

    FlightApplication app;
    FlightApplicationInput appInput;
    appInput.timestampUs = 2500;
    const auto appOutput = app.update(appInput);

    return clock.microseconds() == 1000 &&
           output > 0.0f &&
           pidOutput.motors.frontLeft == 0.0f &&
           failsafeOutput.action == SWC_FailsafeLogicOutput::NONE &&
           appOutput.timestampUs == appInput.timestampUs ? 0 : 1;
}
