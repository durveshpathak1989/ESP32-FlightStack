#pragma once

#include <cstdint>

#include "Application/FailsafePolicy.h"
#include "Application/ModeManager.h"
#include "Components/SWC_BatteryMonitor/SWC_BatteryMonitor.h"
#include "Components/SWC_FailsafeOrchestrator/SWC_FailsafeOrchestrator.h"
#include "Components/SWC_GPSReceiver/SWC_GPSReceiver.h"
#include "Components/SWC_IMUReader/SWC_IMUReader.h"
#include "Components/SWC_MotorOutput/SWC_MotorOutput.h"
#include "Components/SWC_RCReceiver/SWC_RCReceiver.h"
#include "Components/SWC_Telemetry/SWC_Telemetry.h"
#include "Services/Control/ControllerRegistry.h"
#include "Services/Diagnostics/SWC_HealthMonitor.h"
#include "Services/Estimation/SWC_EKFAHRS.h"
#include "Services/Navigation/SWC_FailsafeLogic.h"

struct FlightApplicationInput {
    SWC_IMUReaderInput imu;
    SWC_RCReceiverInput receiver;
    SWC_BatteryMonitorInput battery;
    SWC_GPSReceiverInput gps;
    flight::FlightConfig config;
    ControllerType requestedController = ControllerType::PID;
    bool gpsRequired = false;
    std::uint64_t dtUs = 2500;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct FlightApplicationOutput {
    flight::AttitudeState attitude;
    flight::PilotCommand pilot;
    flight::BatteryState battery;
    flight::GpsState gps;
    flight::MotorCommand motors;
    SWC_MotorOutputOutput motorPwm;
    SWC_FailsafeLogicOutput failsafe;
    ModeManagerOutput mode;
    SWC_HealthMonitorOutput health;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class FlightApplication {
public:
    void init(const FlightApplicationInput& input);
    FlightApplicationOutput update(const FlightApplicationInput& input);
    void reset();

private:
    SWC_IMUReader _imu;
    SWC_EKFAHRS _ahrs;
    SWC_RCReceiver _receiver;
    SWC_BatteryMonitor _battery;
    SWC_GPSReceiver _gps;
    SWC_FailsafeLogic _failsafe;
    SWC_FailsafeOrchestrator _failsafeOrchestrator;
    ControllerRegistry _controllers;
    SWC_MotorOutput _motorOutput;
    SWC_Telemetry _telemetry;
    SWC_HealthMonitor _health;
    ModeManager _mode;
};
