//  setup()
// ═════════════════════════════════════════════════════════════
void setup()
{
    Serial.begin(115200);
    delay(600);

    configASSERT(flightStateRte.begin());
    configASSERT(tuningRte.begin());
    configASSERT(timingRte.begin());
    g_i2cMutex    = xSemaphoreCreateMutex();
    configASSERT(g_i2cMutex);
    if (!flightLogger.begin()) {
        DBG_PRINTLN(F("[BOOT][WARN] FlightLogger allocation failed; /flightlog/csv disabled."));
    }

    memset(&g_state,    0, sizeof(g_state));
    memset(&g_timing,   0, sizeof(g_timing));
    attitudeEstimator.Init();
    flightController.Init();
    motorMixer.Init();
    levelTrim.Init();

    batteryServiceAdapter.InitMonitor();
    const rte::BatteryServiceSample bootBattery =
        batteryServiceAdapter.ReadStatus(clockServiceAdapter.milliseconds(), true);
    g_state.batteryVoltage_v = bootBattery.batteryVoltage_v;
    g_state.batteryAdcVoltage_v = bootBattery.adcVoltage_v;
    g_state.batteryCellVoltage_v = bootBattery.cellVoltage_v;
    g_state.batteryPercent = bootBattery.percent;
    g_state.batteryValid = bootBattery.valid;
    g_state.batteryLow = bootBattery.low;
    g_state.batteryCritical = bootBattery.critical;
    DBG_PRINTF("[BOOT] Battery ADC pin=%u scale=%.4f adc=%.3fV batt=%.3fV cell=%.3fV valid=%d\n",
               (unsigned)PIN_BATTERY_ADC,
               batteryServiceAdapter.CalibrationScale(),
               bootBattery.adcVoltage_v,
               bootBattery.batteryVoltage_v,
               bootBattery.cellVoltage_v,
               bootBattery.valid ? 1 : 0);
    DBG_PRINTLN(F("[BOOT] Battery sanity: 12.2V pack should read about 1.58V at ADC."));

    motorServiceAdapter.begin();
    motorServiceAdapter.prepareEscs();
    g_motorOutputsActive = false;

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_MPU_CS);
    delay(10);

    DBG_PRINTF("[BOOT] IMU backend: %s... ", SelectedImu::backendName());
    if (!imuServiceAdapter.InitSensor()) {
        DBG_PRINTLN(F("FAILED. Check SPI wiring. Halting."));
        while (true) delay(1000);
    }
    DBG_PRINTLN(F("OK"));
#if FLIGHT_IMU_BACKEND == FLIGHT_IMU_BACKEND_MPU9250
    calManager.begin(imu);
#endif
    calManager.attachMotorOutputs(writeMotors, motorsOff);
    DBG_PRINTLN(F("[BOOT] CalibrationManager ready."));

    if (imuServiceAdapter.HasMagnetometer()) {
        DBG_PRINTLN(F("[BOOT] AK8963 magnetometer: DETECTED — 9-DOF AHRS"));
    } else {
        DBG_PRINTLN(F("[BOOT] AK8963 magnetometer: NOT FOUND — 6-DOF AHRS"));
        DBG_PRINTLN(F("[BOOT] Roll and pitch accurate. Yaw will drift."));
    }

    Wire.setClock(TOF_I2C_HZ);
    DBG_PRINTLN(F("[BOOT] BMP280 and VL53L4CX initialization delegated to task Init runnables."));

    DBG_PRINTLN(F("[BOOT] CPU monitor delegated to CpuServiceTask::Init."));

    DBG_PRINTLN(F("[BOOT] GPS initialization delegated to GpsServiceTask::Init."));

    DBG_PRINT(F("[BOOT] NVS calibration... "));
    if (imuServiceAdapter.LoadCalibration()) {
        DBG_PRINTLN(F("loaded."));
        imuServiceAdapter.PrintCalibration();
    } else {
        DBG_PRINTLN(F("none — flip SWD UP (disarmed) to calibrate."));
    }

    DBG_PRINTLN(F("[BOOT] iBUS initialization delegated to ReceiverServiceTask::Init."));

    syncTuningFromObjects();
    DBG_PRINT(F("[BOOT] NVS tuning... "));
    if (g_tuningStore.load(g_tuning)) {
        applyTuningToObjects();
        DBG_PRINTLN(F("loaded and applied."));
    } else {
        DBG_PRINTLN(F("none/invalid — using firmware defaults."));
    }

    const Esp32TaskDefinition serviceTasks[] = {
        {taskRC,     "RC",     6144,  3, 0},
        {taskSerial, "Serial", 4096,  1, 0},
        {taskWiFi,   "WiFi",   12288, 1, 0},
        {taskBMP,    "BMP280", 3072,  1, 0},
        {taskToF,    "ToF",    4096,  1, 0},
        {taskCPU,    "CPU",    3072,  1, 0},
        {taskGPS,    "GPS",    4096,  1, 0}
    };
    const Esp32TaskDefinition controlTask{taskControl, "Ctrl", 10240, 5, 1};
    if (!flightScheduler.start(
            serviceTasks, sizeof(serviceTasks) / sizeof(serviceTasks[0]),
            controlTask, TIMING_TARGET_US)) {
        DBG_PRINTLN(F("[BOOT][ERROR] Failed to start flight scheduler."));
        while (true) delay(1000);
    }

    DBG_PRINTLN(F("[BOOT] All tasks running."));
    DBG_PRINTLN(F("[BOOT] Timing target: 2500 us (400 Hz control loop)."));
    DBG_PRINTLN(F("[BOOT] Ground station: http://192.168.4.1"));
    DBG_PRINTLN(F("[BOOT] GPS fix takes 30-90 s outdoors."));
    DBG_PRINTF("[BOOT] Free heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(portMAX_DELAY); }
