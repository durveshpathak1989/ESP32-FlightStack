#include "test_support.h"

#include "Application/FlightApplication.h"

int main()
{
    FlightApplication app;
    FlightApplicationInput input;
    input.dtUs = 2500;
    input.timestampUs = 2500;
    input.imu.azG = 1.0f;
    input.imu.accelValid = true;
    input.imu.gyroValid = true;
    input.receiver.frameValid = true;
    input.receiver.channels[2] = 1500;
    input.receiver.channels[6] = 1800;
    input.receiver.channels[7] = 1000;
    input.battery.rawAdcCount = 2000;
    input.battery.adcMillivolts = 1556;

    auto out = app.update(input);
    EXPECT_TRUE(out.pilot.valid);
    EXPECT_TRUE(out.mode.armed);
    EXPECT_TRUE(out.mode.mode == ArchV5FlightMode::Angle);
    EXPECT_TRUE(out.failsafe.action == SWC_FailsafeLogicOutput::NONE);
    EXPECT_TRUE(out.motorPwm.debug.pwmFL == 1000);

    return testPassed("test_FlightApplication");
}
