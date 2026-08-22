#pragma once

#include <cstdint>

#include "Services/Control/SWC_LQRController.h"
#include "Services/Control/SWC_PIDController.h"

enum class ControllerType : std::uint8_t {
    PID = 0,
    LQR = 1
};

struct ControllerRegistryOutput {
    flight::MotorCommand motors;
    SWC_PIDControllerOutput::Debug pidDebug;
    ControllerType activeController = ControllerType::PID;
    bool controllerSwitchAccepted = false;
    bool fallbackToPid = false;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class ControllerRegistry {
public:
    void init(const SWC_PIDControllerInput& input)
    {
        _pid.init(input);
        _lqr.init(input);
    }

    bool setActiveController(ControllerType type, bool isArmed)
    {
        if (isArmed) return false;
        _active = type;
        _pid.reset();
        _lqr.reset();
        return true;
    }

    ControllerType activeController() const { return _active; }

    ControllerRegistryOutput update(const SWC_PIDControllerInput& input)
    {
        ControllerRegistryOutput out;
        out.activeController = _active;
        out.timestampUs = input.timestampUs;

        if (_active == ControllerType::LQR) {
            const auto lqrOut = _lqr.update(input);
            out.motors = lqrOut.motors;
            out.fallbackToPid = !lqrOut.debug.implemented;
            if (!out.fallbackToPid) return out;
        }

        const auto pidOut = _pid.update(input);
        out.motors = pidOut.motors;
        out.pidDebug = pidOut.debug;
        out.activeController = ControllerType::PID;
        return out;
    }

    void reset()
    {
        _pid.reset();
        _lqr.reset();
    }

private:
    SWC_PIDController _pid;
    SWC_LQRController _lqr;
    ControllerType _active = ControllerType::PID;
};
