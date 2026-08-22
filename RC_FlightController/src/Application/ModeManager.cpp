#include "ModeManager.h"

ModeManagerOutput ModeManager::update(const ModeManagerInput& input)
{
    ModeManagerOutput out;
    out.timestampUs = input.timestampUs;

    if (input.failsafeActive) {
        out.armed = _armed;
        out.mode = ArchV5FlightMode::Failsafe;
        out.activeController = _activeController;
        return out;
    }

    _armed = input.pilot.valid && input.pilot.armSwitch;
    if (!_armed) {
        if (input.requestedController != _activeController) {
            _activeController = input.requestedController;
            out.controllerSwitchAccepted = true;
        }
        out.armed = false;
        out.mode = ArchV5FlightMode::Disarmed;
        out.activeController = _activeController;
        return out;
    }

    out.armed = true;
    out.mode = input.pilot.mode == 1 ? ArchV5FlightMode::Acro : ArchV5FlightMode::Angle;
    out.activeController = _activeController;
    return out;
}

void ModeManager::reset()
{
    _armed = false;
    _activeController = ControllerType::PID;
}
