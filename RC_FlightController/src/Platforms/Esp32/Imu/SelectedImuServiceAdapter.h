#pragma once

#include "../../../Core/ServicePorts.h"
#include "SelectedImu.h"

class SelectedImuServiceAdapter : public rte::ImuServicePort {
public:
    explicit SelectedImuServiceAdapter(SelectedImu& sensor) : sensor_(sensor) {}

    bool InitSensor() override { return sensor_.begin(); }
    bool ReadSample(rte::ImuServiceSample& sample) override {
        ImuSensorData value{};
        if (!sensor_.readScaled(value)) return false;
        sample = {value.ax_g, value.ay_g, value.az_g,
                  value.gx_dps, value.gy_dps, value.gz_dps,
                  value.mx_uT, value.my_uT, value.mz_uT, value.temp_c,
                  sensor_.isMagConnected()};
        return true;
    }
    bool HasMagnetometer() const override { return sensor_.hasMag(); }
    bool LoadCalibration() override { return sensor_.loadCalibration(); }
    void PrintCalibration() const override { sensor_.printCalibration(); }

private:
    SelectedImu& sensor_;
};
