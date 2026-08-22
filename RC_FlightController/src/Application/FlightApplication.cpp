#include "FlightApplication.h"

void FlightApplication::init(const FlightApplicationInput& input)
{
    reset();
    (void)update(input);
}

FlightApplicationOutput FlightApplication::update(const FlightApplicationInput& input)
{
    FlightApplicationOutput out;
    out.timestampUs = input.timestampUs;

    auto imuInput = input.imu;
    auto receiverInput = input.receiver;
    auto batteryInput = input.battery;
    auto gpsInput = input.gps;
    if (imuInput.timestampUs == 0) imuInput.timestampUs = input.timestampUs;
    if (receiverInput.timestampUs == 0) receiverInput.timestampUs = input.timestampUs;
    if (batteryInput.timestampUs == 0) batteryInput.timestampUs = input.timestampUs;
    if (gpsInput.timestampUs == 0) gpsInput.timestampUs = input.timestampUs;

    auto imuOut = _imu.update(imuInput);
    SWC_EKFAHRSInput ahrsIn;
    ahrsIn.imu = imuOut.imu;
    ahrsIn.dtUs = input.dtUs;
    ahrsIn.timestampUs = input.timestampUs;
    auto ahrsOut = _ahrs.update(ahrsIn);

    auto rcOut = _receiver.update(receiverInput);
    auto batteryOut = _battery.update(batteryInput);
    auto gpsOut = _gps.update(gpsInput);

    SWC_FailsafeLogicInput failsafeIn;
    failsafeIn.battery = batteryOut.battery;
    failsafeIn.receiver = rcOut.receiver;
    failsafeIn.gps.hasGpsFix = gpsOut.gps.fix != flight::GpsState::NO_FIX;
    failsafeIn.gps.gpsRequired = input.gpsRequired;
    failsafeIn.gps.lastFixUpdateUs = gpsOut.gps.timestampUs;
    failsafeIn.config = input.config;
    failsafeIn.timestampUs = input.timestampUs;
    auto failsafeOut = _failsafe.update(failsafeIn);

    ModeManagerInput modeIn;
    modeIn.pilot = rcOut.command;
    modeIn.failsafeActive = FailsafePolicy::isActive(failsafeOut);
    modeIn.requestedController = input.requestedController;
    modeIn.timestampUs = input.timestampUs;
    auto modeOut = _mode.update(modeIn);
    if (modeOut.controllerSwitchAccepted) {
        _controllers.setActiveController(modeOut.activeController, modeOut.armed);
    }

    SWC_PIDControllerInput controllerIn;
    controllerIn.attitude = ahrsOut.attitude;
    controllerIn.pilot = rcOut.command;
    controllerIn.config = input.config;
    controllerIn.isArmed = modeOut.armed;
    controllerIn.angleMode = modeOut.mode == ArchV5FlightMode::Angle;
    controllerIn.imuValid = imuOut.valid;
    controllerIn.dtUs = input.dtUs;
    controllerIn.timestampUs = input.timestampUs;
    auto controllerOut = _controllers.update(controllerIn);

    SWC_FailsafeOrchestratorInput orchestratorIn;
    orchestratorIn.motors = controllerOut.motors;
    orchestratorIn.failsafe = failsafeOut;
    orchestratorIn.dtUs = input.dtUs;
    orchestratorIn.timestampUs = input.timestampUs;
    auto motorCommandOut = _failsafeOrchestrator.update(orchestratorIn);

    SWC_MotorOutputInput motorIn;
    motorIn.motors = motorCommandOut.motors;
    motorIn.isArmed = modeOut.armed && !motorCommandOut.disarm;
    motorIn.dtUs = input.dtUs;
    motorIn.timestampUs = input.timestampUs;
    auto pwmOut = _motorOutput.update(motorIn);

    SWC_HealthMonitorInput healthIn;
    healthIn.attitude = ahrsOut.attitude;
    healthIn.battery = batteryOut.battery;
    healthIn.receiver = rcOut.receiver;
    healthIn.imuFresh = imuOut.valid;
    healthIn.timestampUs = input.timestampUs;
    auto healthOut = _health.update(healthIn);

    out.attitude = ahrsOut.attitude;
    out.pilot = rcOut.command;
    out.battery = batteryOut.battery;
    out.gps = gpsOut.gps;
    out.motors = motorCommandOut.motors;
    out.motorPwm = pwmOut;
    out.failsafe = failsafeOut;
    out.mode = modeOut;
    out.health = healthOut;
    return out;
}

void FlightApplication::reset()
{
    _ahrs.reset();
    _receiver.reset();
    _battery.reset();
    _failsafe.reset();
    _controllers.reset();
    _motorOutput.reset();
    _telemetry.reset();
    _mode.reset();
}
