#include "test_support.h"

#include "Services/Estimation/SWC_EKFAHRS.h"

int main()
{
    SWC_EKFAHRS estimator;
    SWC_EKFAHRSInput input;
    input.imu.az = 9.80665f;
    input.dtUs = 2500;

    auto out = estimator.update(input);
    EXPECT_NEAR(out.attitude.roll, 0.0f, 0.001f);
    EXPECT_NEAR(out.attitude.pitch, 0.0f, 0.001f);
    EXPECT_TRUE(out.debug.accelAccepted);

    input.imu.gz = 1.0f;
    for (int i = 0; i < 400; ++i) {
        input.timestampUs += input.dtUs;
        out = estimator.update(input);
    }
    EXPECT_TRUE(out.attitude.yaw > 0.5f);
    EXPECT_TRUE(out.debug.healthy);

    return testPassed("test_SWC_EKFAHRS");
}
