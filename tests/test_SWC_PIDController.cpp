#include "test_support.h"

#include "Services/Control/SWC_PIDController.h"

int main()
{
    SWC_PIDController controller;
    SWC_PIDControllerInput input;
    input.timestampUs = 2500;
    input.dtUs = 2500;
    input.isArmed = false;
    input.pilot.throttle = 0.7f;

    auto out = controller.update(input);
    EXPECT_NEAR(out.motors.frontLeft, 0.0f, 0.0001f);
    EXPECT_NEAR(out.motors.frontRight, 0.0f, 0.0001f);

    input.isArmed = true;
    input.angleMode = true;
    input.pilot.valid = true;
    input.pilot.throttle = 0.5f;
    for (int i = 0; i < 300; ++i) {
        input.timestampUs += input.dtUs;
        out = controller.update(input);
    }
    EXPECT_TRUE(out.motors.frontLeft > 0.03f);
    EXPECT_TRUE(out.motors.frontLeft <= 1.0f);
    EXPECT_NEAR(out.motors.frontLeft, out.motors.frontRight, 0.0001f);

    input.pilot.roll = 1.0f;
    input.timestampUs += input.dtUs;
    out = controller.update(input);
    EXPECT_TRUE(out.debug.attErrRoll > 0.0f);
    EXPECT_TRUE(out.debug.rollOutput > 0.0f);
    EXPECT_TRUE(out.motors.frontLeft > out.motors.frontRight);

    return testPassed("test_SWC_PIDController");
}
