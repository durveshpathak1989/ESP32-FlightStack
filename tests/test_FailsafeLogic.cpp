#include "test_support.h"

#include "Services/Navigation/SWC_FailsafeLogic.h"

static SWC_FailsafeLogicInput safeInput()
{
    SWC_FailsafeLogicInput input;
    input.battery.voltage = 11.8f;
    input.battery.status = flight::BatteryState::HEALTHY;
    input.receiver.isConnected = true;
    input.receiver.lastUpdateUs = 0;
    input.timestampUs = 0;
    return input;
}

int main()
{
    SWC_FailsafeLogic logic;
    auto input = safeInput();

    input.battery.voltage = 8.8f;
    auto out = logic.update(input);
    EXPECT_TRUE(out.action == SWC_FailsafeLogicOutput::LAND_IMMEDIATELY);
    EXPECT_TRUE(out.reason == SWC_FailsafeLogicOutput::BATTERY_CRITICAL);

    logic.reset();
    input = safeInput();
    input.battery.voltage = 9.8f;
    out = logic.update(input);
    EXPECT_TRUE(out.action == SWC_FailsafeLogicOutput::NONE);
    input.timestampUs = 1000000;
    out = logic.update(input);
    EXPECT_TRUE(out.action == SWC_FailsafeLogicOutput::DESCEND);
    EXPECT_TRUE(out.reason == SWC_FailsafeLogicOutput::BATTERY_LOW);

    logic.reset();
    input = safeInput();
    input.receiver.lastUpdateUs = 0;
    input.timestampUs = 2500001;
    out = logic.update(input);
    EXPECT_TRUE(out.action == SWC_FailsafeLogicOutput::DESCEND);
    EXPECT_TRUE(out.reason == SWC_FailsafeLogicOutput::RC_LOSS_TIMEOUT);

    logic.reset();
    input = safeInput();
    input.gps.gpsRequired = true;
    input.gps.hasGpsFix = true;
    input.gps.lastFixUpdateUs = 0;
    input.timestampUs = 6000001;
    input.receiver.lastUpdateUs = input.timestampUs;
    out = logic.update(input);
    EXPECT_TRUE(out.reason == SWC_FailsafeLogicOutput::GPS_LOSS_TIMEOUT);

    return testPassed("test_FailsafeLogic");
}
