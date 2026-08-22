#pragma once

#include <cstdint>

#include "Core/FlightTypes.h"
#include "Services/Control/ControllerRegistry.h"

enum class ArchV5FlightMode : std::uint8_t {
    Disarmed = 0,
    Angle = 1,
    Acro = 2,
    AltHold = 3,
    Loiter = 4,
    Auto = 5,
    Land = 6,
    Failsafe = 7
};

struct ModeManagerInput {
    flight::PilotCommand pilot;
    bool failsafeActive = false;
    ControllerType requestedController = ControllerType::PID;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct ModeManagerOutput {
    bool armed = false;
    ArchV5FlightMode mode = ArchV5FlightMode::Disarmed;
    ControllerType activeController = ControllerType::PID;
    bool controllerSwitchAccepted = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class ModeManager {
public:
    void init(const ModeManagerInput&) { reset(); }
    ModeManagerOutput update(const ModeManagerInput& input);
    void reset();

private:
    bool _armed = false;
    ControllerType _activeController = ControllerType::PID;
};
