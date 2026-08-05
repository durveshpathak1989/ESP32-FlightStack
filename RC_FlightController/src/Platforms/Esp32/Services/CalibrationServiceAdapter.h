#pragma once

#include "../../../Core/ServicePorts.h"
#include "../../../Submodules/CalManager/CalibrationManager.h"

class CalibrationServiceAdapter : public rte::CalibrationServicePort {
public:
    explicit CalibrationServiceAdapter(CalibrationManager& manager)
        : manager_(manager) {}

    bool Request(rte::CalibrationRequest request) override {
        const CalibrationMode mode =
            request == rte::CalibrationRequest::Esc
                ? CalibrationMode::ESC
                : CalibrationMode::IMU_ALL_GUIDED;
        return manager_.request(mode, CalibrationSource::RC);
    }
    rte::CalibrationServiceStatus Status() const override {
        const CalibrationStatus status = manager_.status();
        return {status.active,
                status.mode == CalibrationMode::ESC
                    ? rte::CalibrationRequest::Esc
                    : rte::CalibrationRequest::ImuAllGuided,
                status.safeToRun,
                status.requiresUserConfirm,
                manager_.shouldBlockFlight(),
                manager_.ownsMotors(),
                status.progress,
                status.runId,
                static_cast<std::uint8_t>(status.state),
                ModeName(status.mode),
                SourceName(status.source),
                StateName(status.state),
                status.message,
                status.error};
    }
    void SetSafety(bool safeToRun) override { manager_.setSafety(safeToRun); }
    void ConfirmStep() override { manager_.confirmStep(); }
    void PeriodicService() override { manager_.update(); }
    void Cancel() override { manager_.cancel(); }

private:
    static const char* ModeName(CalibrationMode mode) {
        switch (mode) {
            case CalibrationMode::ESC: return "ESC";
            case CalibrationMode::GYRO_BIAS: return "GYRO_BIAS";
            case CalibrationMode::ACCEL_6_FACE: return "ACCEL_6_FACE";
            case CalibrationMode::MAG_MINMAX: return "MAG_MINMAX";
            case CalibrationMode::IMU_ALL_GUIDED: return "IMU_ALL_GUIDED";
            default: return "NONE";
        }
    }
    static const char* StateName(CalibrationState state) {
        switch (state) {
            case CalibrationState::REQUESTED: return "REQUESTED";
            case CalibrationState::WAITING_FOR_SAFE: return "WAITING_FOR_SAFE";
            case CalibrationState::WAITING_FOR_STILLNESS: return "WAITING_FOR_STILLNESS";
            case CalibrationState::WAITING_FOR_USER_STEP: return "WAITING_FOR_USER_STEP";
            case CalibrationState::COLLECTING: return "COLLECTING";
            case CalibrationState::COMPUTING: return "COMPUTING";
            case CalibrationState::SAVING: return "SAVING";
            case CalibrationState::DONE: return "DONE";
            case CalibrationState::FAILED: return "FAILED";
            case CalibrationState::CANCELLED: return "CANCELLED";
            default: return "IDLE";
        }
    }
    static const char* SourceName(CalibrationSource source) {
        switch (source) {
            case CalibrationSource::WEB: return "WEB";
            case CalibrationSource::RC: return "RC";
            default: return "NONE";
        }
    }
    CalibrationManager& manager_;
};
