#pragma once

#include "../../../Core/Ports.h"
#include "../../../Submodules/MotorControl/MotorControl.h"

class Esp32MotorServiceAdapter : public flight::MotorPort {
public:
    bool begin() override {
        motorBegin();
        return true;
    }
    void prepareEscs() override { motorEscArm(); }
    void write(const flight::MotorCommand& command) override {
        motorSet(command.frontLeft, command.frontRight,
                 command.rearLeft, command.rearRight);
    }
    void stop() override { motorOff(); }
};
