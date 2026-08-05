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
                    : rte::CalibrationRequest::ImuAllGuided};
    }
    void Cancel() override { manager_.cancel(); }

private:
    CalibrationManager& manager_;
};
