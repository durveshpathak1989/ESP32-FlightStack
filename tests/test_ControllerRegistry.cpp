#include "test_support.h"

#include "Services/Control/ControllerRegistry.h"

int main()
{
    ControllerRegistry registry;
    SWC_PIDControllerInput input;
    input.isArmed = false;
    EXPECT_TRUE(registry.setActiveController(ControllerType::LQR, false));
    EXPECT_TRUE(registry.activeController() == ControllerType::LQR);
    EXPECT_TRUE(!registry.setActiveController(ControllerType::PID, true));
    EXPECT_TRUE(registry.activeController() == ControllerType::LQR);

    input.timestampUs = 2500;
    auto out = registry.update(input);
    EXPECT_TRUE(out.fallbackToPid);
    EXPECT_TRUE(out.activeController == ControllerType::PID);

    return testPassed("test_ControllerRegistry");
}
