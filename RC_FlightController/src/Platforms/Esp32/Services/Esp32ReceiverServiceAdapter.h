#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/iFly/FlySkyiBUS.h"

class Esp32ReceiverServiceAdapter : public rte::ReceiverServicePort {
public:
    Esp32ReceiverServiceAdapter(FlySkyiBUS& receiver, std::uint8_t rxPin,
                                std::uint8_t txPin, std::uint8_t uart)
        : receiver_(receiver), rxPin_(rxPin), txPin_(txPin), uart_(uart) {}

    bool InitReceiver() override {
        receiver_.begin(rxPin_, txPin_, uart_);
        return true;
    }
    flight::ReceiverFrame ReadFrame() override {
        receiver_.update();
        const RCCommand input = receiver_.getCommand();
        flight::ReceiverFrame output{};
        output.command.throttle = input.throttle;
        output.command.roll = input.roll;
        output.command.pitch = input.pitch;
        output.command.yaw = input.yaw;
        for (std::size_t i = 0; i < 14; ++i)
            output.command.raw[i] = input.raw[i];
        output.command.mode = convertMode(input.mode);
        output.command.swdHigh = input.swdHigh;
        output.command.valid = input.valid;
        output.frameRateHz = receiver_.getFrameRate();
        output.checksumFailureCount = receiver_.getChecksumFailCount();
        output.failsafeCount = receiver_.getFailsafeCount();
        return output;
    }

private:
    static flight::FlightMode convertMode(FlightMode mode) {
        switch (mode) {
            case FlightMode::ANGLE: return flight::FlightMode::Angle;
            case FlightMode::ACRO: return flight::FlightMode::Acro;
            case FlightMode::FAILSAFE: return flight::FlightMode::Failsafe;
            default: return flight::FlightMode::Disarmed;
        }
    }

    FlySkyiBUS& receiver_;
    std::uint8_t rxPin_;
    std::uint8_t txPin_;
    std::uint8_t uart_;
};
