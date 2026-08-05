#pragma once

#include <cstdint>

#include "Arduino.h"
#include "../../../Submodules/CalManager/CalibrationManager.h"
#include "../../../Submodules/DebugConfig/DebugConfig.h"
#include "../../../Submodules/iFly/FlySkyiBUS.h"
#include "../../../Core/RtePorts.h"

class ReceiverServiceTask : public rte::SoftwareComponent {
public:
    using LogFunction = void (*)(const char* message);

    ReceiverServiceTask(FlySkyiBUS& receiver, CalibrationManager& calibration,
                        LogFunction log, std::uint16_t escThreshold,
                        float throttleCut)
        : receiver_(receiver), calibration_(calibration), log_(log),
          escThreshold_(escThreshold), throttleCut_(throttleCut) {}

    void Init() override {
        swdPreviouslyHigh_ = false;
        escPreviouslyHigh_ = false;
        lastReportMs_ = 0;
        lastFailureCount_ = 0;
    }

    void Periodic() override {
        receiver_.update();
        const RCCommand command = receiver_.getCommand();
        serviceImuCalibration(command);
        serviceEscCalibration(command);
        reportHealth();
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
    void serviceImuCalibration(const RCCommand& command) {
        if (command.swdHigh && !swdPreviouslyHigh_) {
            if (command.mode == FlightMode::DISARMED) {
                calibration_.request(CalibrationMode::IMU_ALL_GUIDED,
                                     CalibrationSource::RC);
                log_("[RC] Calibration Manager request — IMU All GUIDED.");
            } else {
                log_("[RC] Cannot calibrate while armed.");
            }
        }
        swdPreviouslyHigh_ = command.swdHigh;
    }

    void serviceEscCalibration(const RCCommand& command) {
        const bool escHigh = command.valid &&
            command.raw[RC_CH_AUX5] >= escThreshold_;
        if (escHigh && !escPreviouslyHigh_) {
            if (command.mode == FlightMode::DISARMED &&
                command.throttle <= throttleCut_) {
                calibration_.request(CalibrationMode::ESC, CalibrationSource::RC);
                log_("[RC] ESC calibration requested by VrB. Use SWC to confirm.");
            } else {
                log_("[RC] ESC calibration rejected — disarm and set throttle low first.");
            }
        }
        escPreviouslyHigh_ = escHigh;
        const CalibrationStatus status = calibration_.status();
        if (status.active && status.mode == CalibrationMode::ESC && !escHigh) {
            calibration_.cancel();
            log_("[RC] ESC calibration cancelled — VrB lowered.");
        }
    }

    void reportHealth() {
        const std::uint32_t nowMs = millis();
        if (nowMs - lastReportMs_ < 1000) return;
        const std::uint32_t failures = receiver_.getChecksumFailCount();
        const std::uint32_t failuresPerSecond = failures - lastFailureCount_;
        lastFailureCount_ = failures;
        lastReportMs_ = nowMs;
        DBG_PRINTF("[iBUS] %.0f Hz good | %lu bad/s | %lu bad total\n",
                   receiver_.getFrameRate(),
                   static_cast<unsigned long>(failuresPerSecond),
                   static_cast<unsigned long>(failures));
    }

    FlySkyiBUS& receiver_;
    CalibrationManager& calibration_;
    LogFunction log_;
    std::uint16_t escThreshold_;
    float throttleCut_;
    bool swdPreviouslyHigh_ = false;
    bool escPreviouslyHigh_ = false;
    std::uint32_t lastReportMs_ = 0;
    std::uint32_t lastFailureCount_ = 0;
};
