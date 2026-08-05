#include "Application/Control/PidController.h"
#include "Application/Configuration/TuningState.h"
#include "Core/FlightTypes.h"
#include "Core/Ports.h"

class FakeClock final : public flight::ClockPort {
public:
    std::uint64_t microseconds() const override { return 1000; }
    std::uint32_t milliseconds() const override { return 1; }
};

int main() {
    FakeClock clock;
    PidController controller(1.0f, 0.1f, 0.01f);
    TuningState tuning{};
    tuning.max_angle_deg = 5.0f;
    const float output = controller.update(0.5f, 0.01f);
    return clock.microseconds() == 1000 && output > 0.0f &&
                   tuning.max_angle_deg == 5.0f
               ? 0
               : 1;
}
