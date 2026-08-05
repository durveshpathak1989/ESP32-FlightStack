#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Submodules/DebugConfig/DebugConfig.h"
#include "../../../Core/RtePorts.h"
#include "../../../Core/ServicePorts.h"

class ReceiverServiceTask : public rte::SoftwareComponent {
public:
    using LogFunction = void (*)(const char* message);

    ReceiverServiceTask(rte::ReceiverServicePort& receiver,
                        rte::CalibrationServicePort& calibration,
                        rte::ClockServicePort& clock,
                        rte::SenderReceiverPort<flight::ReceiverFrame>& outputPort,
                        LogFunction log, std::uint16_t escThreshold,
                        float throttleCut, std::uint8_t escChannelIndex)
        : receiver_(receiver), calibration_(calibration), clock_(clock), outputPort_(outputPort),
          log_(log), escThreshold_(escThreshold), throttleCut_(throttleCut),
          escChannelIndex_(escChannelIndex) {}

    void Init() override {
        swdPreviouslyHigh_ = false;
        escPreviouslyHigh_ = false;
        lastReportMs_ = 0;
        lastFailureCount_ = 0;
        outputPort_.invalidate();
        receiver_.InitReceiver();
    }

    void Periodic() override {
        latestFrame_ = receiver_.ReadFrame();
        outputPort_.send(latestFrame_,
                         static_cast<std::uint64_t>(clock_.milliseconds()) * 1000ULL);
        serviceImuCalibration(latestFrame_.command);
        serviceEscCalibration(latestFrame_.command);
        reportHealth(latestFrame_);
    }

    [[noreturn]] void run() {
        const TickType_t period = pdMS_TO_TICKS(5);
        TickType_t lastWake = xTaskGetTickCount();
        Init();
        for (;;) {
            Periodic();
            vTaskDelayUntil(&lastWake, period);
        }
    }

private:
    void serviceImuCalibration(const flight::PilotCommand& command) {
        if (command.swdHigh && !swdPreviouslyHigh_) {
            if (command.mode == flight::FlightMode::Disarmed) {
                calibration_.Request(rte::CalibrationRequest::ImuAllGuided);
                log_("[RC] Calibration Manager request — IMU All GUIDED.");
            } else {
                log_("[RC] Cannot calibrate while armed.");
            }
        }
        swdPreviouslyHigh_ = command.swdHigh;
    }

    void serviceEscCalibration(const flight::PilotCommand& command) {
        const bool escHigh = command.valid &&
            command.raw[escChannelIndex_] >= escThreshold_;
        if (escHigh && !escPreviouslyHigh_) {
            if (command.mode == flight::FlightMode::Disarmed &&
                command.throttle <= throttleCut_) {
                calibration_.Request(rte::CalibrationRequest::Esc);
                log_("[RC] ESC calibration requested by VrB. Use SWC to confirm.");
            } else {
                log_("[RC] ESC calibration rejected — disarm and set throttle low first.");
            }
        }
        escPreviouslyHigh_ = escHigh;
        const rte::CalibrationServiceStatus status = calibration_.Status();
        if (status.active && status.request == rte::CalibrationRequest::Esc && !escHigh) {
            calibration_.Cancel();
            log_("[RC] ESC calibration cancelled — VrB lowered.");
        }
    }

    void reportHealth(const flight::ReceiverFrame& frame) {
        const std::uint32_t nowMs = clock_.milliseconds();
        if (nowMs - lastReportMs_ < 1000) return;
        const std::uint32_t failures = frame.checksumFailureCount;
        const std::uint32_t failuresPerSecond = failures - lastFailureCount_;
        lastFailureCount_ = failures;
        lastReportMs_ = nowMs;
        DBG_PRINTF("[iBUS] %.0f Hz good | %lu bad/s | %lu bad total\n",
                   frame.frameRateHz,
                   static_cast<unsigned long>(failuresPerSecond),
                   static_cast<unsigned long>(failures));
    }

    rte::ReceiverServicePort& receiver_;
    rte::CalibrationServicePort& calibration_;
    rte::ClockServicePort& clock_;
    rte::SenderReceiverPort<flight::ReceiverFrame>& outputPort_;
    LogFunction log_;
    std::uint16_t escThreshold_;
    float throttleCut_;
    std::uint8_t escChannelIndex_;
    flight::ReceiverFrame latestFrame_{};
    bool swdPreviouslyHigh_ = false;
    bool escPreviouslyHigh_ = false;
    std::uint32_t lastReportMs_ = 0;
    std::uint32_t lastFailureCount_ = 0;
};
