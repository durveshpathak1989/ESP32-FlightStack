#include "test_support.h"

#include "Application/ModeManager.h"

int main()
{
    ModeManager manager;
    ModeManagerInput input;
    input.pilot.valid = true;
    input.pilot.armSwitch = false;
    input.requestedController = ControllerType::LQR;
    auto out = manager.update(input);
    EXPECT_TRUE(!out.armed);
    EXPECT_TRUE(out.mode == ArchV5FlightMode::Disarmed);
    EXPECT_TRUE(out.controllerSwitchAccepted);

    input.pilot.armSwitch = true;
    input.pilot.mode = 1;
    input.requestedController = ControllerType::PID;
    out = manager.update(input);
    EXPECT_TRUE(out.armed);
    EXPECT_TRUE(out.mode == ArchV5FlightMode::Acro);
    EXPECT_TRUE(!out.controllerSwitchAccepted);
    EXPECT_TRUE(out.activeController == ControllerType::LQR);

    input.failsafeActive = true;
    out = manager.update(input);
    EXPECT_TRUE(out.mode == ArchV5FlightMode::Failsafe);

    return testPassed("test_ModeManager");
}
