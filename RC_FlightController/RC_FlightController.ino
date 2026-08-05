/*
 * Name: RC_FlightController.ino
 * Use: Main Arduino sketch that starts sensors, tasks, telemetry, calibration, PID control, and motor output.
 * Version: 6.1.0
 * Created by: Durvesh Pathak dp676@cornell.edu
 */

/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  RC_FlightController.ino  v6.1.0                                ║
 * ║  FlySky FS-iA6B iBUS  +  MPU-9250/6500  +  BMP280  +  GPS       ║
 * ║  Fully autonomous — no keyboard required                         ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  v2.3.2 changes (over the pasted v2.3.1 working file):           ║
 * ║   • PID telemetry FIXED: real rO/pO/yO now published in          ║
 * ║     FlightState and printed (was feeding 4 motor values into     ║
 * ║     3 out-slots, garbling yawSP/hold). [PID] line is now a       ║
 * ║     sibling of the 1 Hz block at ~4 Hz, toggled by Serial 'p'.   ║
 * ║   • No flight-behavior changes: gains, yaw-stick negation,       ║
 * ║     heading hold, LPF reset, output limits all preserved.        ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                           ║
 * ║   FS-iA6B iBUS port → GPIO 16 (UART2 RX)                        ║
 * ║   MPU-9250 SCLK→5  MOSI→18  MISO→19  NCS→33  VCC→3.3V          ║
 * ║   BMP280 SDA→21  SCL→22  VCC→3.3V  GND→GND  CSB→3.3V           ║
 * ║   GPS TXD→13  VCC→3.3V  GND→GND (UART1)                         ║
 * ║   Motors: FL→25  FR→15  RL→14  RR→32                            ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  RC SWITCH ASSIGNMENTS                                            ║
 * ║   CH7  SWA  → ARM / DISARM                                       ║
 * ║   CH8  SWB  → ANGLE / ACRO                                       ║
 * ║   CH9  SWC  → Accel confirm during calibration                   ║
 * ║   CH10 SWD  → CALIBRATION trigger (flip UP while disarmed)       ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  SERIAL COMMANDS                                                  ║
 * ║   p  → toggle ~4 Hz [PID] tuning trace on/off (off at boot)     ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  HTTP ENDPOINTS                                                   ║
 * ║   GET  /telemetry     — full state JSON                          ║
 * ║   POST /tune          — apply PID / Mahony gains (disarmed)      ║
 * ║   GET/POST /update    — Web OTA update (disarmed/motors off)     ║
 * ║   GET  /log?since=N   — calibration log lines                    ║
 * ║   GET  /timing        — IMU jitter stats JSON (Test 7.1)         ║
 * ║   POST /timing/reset  — reset jitter stats (between conditions)  ║
 * ║   GET  /timing/csv    — raw period_us ring buffer CSV            ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <SPI.h>
#include <stdarg.h>
#include "src/Submodules/DebugConfig/DebugConfig.h"
#include "src/Submodules/MotorControl/MotorControl.h"
#include "src/Platforms/Esp32/Imu/SelectedImu.h"
#include "src/Platforms/Esp32/Imu/SelectedImuServiceAdapter.h"
#include "src/Submodules/iFly/FlySkyiBUS.h"
#include "src/Submodules/WiFiTelemetry/TelemetryWiFi.h"
#include "src/Submodules/BatteryMonitor/BatteryMonitor.h"
#include "src/Submodules/BMP280/BMP280Sensor.h"
#include "src/Submodules/EKF/AttitudeEKF.h"
#include "src/Submodules/ToF/FlightToF_VL53L4CX.h"
#include "src/Submodules/NotchFilter/NotchFilter.h"
#include "src/Submodules/FFT/SpectrumAnalyzer.h"
#include "src/Submodules/Madgwick/MadgwickAHRS.h"
#include "src/Submodules/MahonyAHRS/MahonyAHRS.h"
#include "src/Submodules/ESP32Core/CPUUtilization.h"
#include "src/Submodules/GPS/GPSSensor.h"
#include "src/Submodules/Logger/Logger.h"
#include "FirmwareBuildInfo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "src/Submodules/CalManager/CalibrationManager.h"
#include "src/Application/FlightConfig.h"
#include "src/Application/Calibration/LevelTrimService.h"
#include "src/Application/Control/CascadedController.h"
#include "src/Application/Control/LowPassFilter.h"
#include "src/Application/Control/MotorMixer.h"
#include "src/Application/Control/PidController.h"
#include "src/Application/Configuration/TuningState.h"
#include "src/Application/Estimation/AttitudeEstimatorRouter.h"
#include "src/Application/Safety/FlightSafetyPolicy.h"
#include "src/Application/State/FlightState.h"
#include "src/Application/Runtime/FlightControlSwc.h"
#include "src/Application/Diagnostics/SerialDiagnosticsSwc.h"
#include "src/Platforms/Esp32/Storage/PreferencesTuningStore.h"
#include "src/Platforms/Esp32/Configuration/TelemetryTuneAdapter.h"
#include "src/Platforms/Esp32/Runtime/Esp32FlightScheduler.h"
#include "src/Platforms/Esp32/Runtime/SnapshotRte.h"
#include "src/Platforms/Esp32/Tasks/CpuServiceTask.h"
#include "src/Platforms/Esp32/Tasks/GpsServiceTask.h"
#include "src/Platforms/Esp32/Tasks/ReceiverServiceTask.h"
#include "src/Platforms/Esp32/Tasks/BarometerServiceTask.h"
#include "src/Platforms/Esp32/Tasks/TofServiceTask.h"
#include "src/Platforms/Esp32/Tasks/WifiServiceTask.h"
#include "src/Platforms/Esp32/Services/TelemetryWifiServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32CpuLoadServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32GpsServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32BarometerServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32RangeServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32ReceiverServiceAdapter.h"
#include "src/Platforms/Esp32/Services/CalibrationServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32BatteryServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32SystemServiceAdapter.h"
#include "src/Platforms/Esp32/Services/Esp32DiagnosticServiceAdapter.h"
#include "src/Platforms/Esp32/Motor/Esp32MotorServiceAdapter.h"

#include "src/Composition/RuntimeObjects.inc"
#include "src/Composition/ConfigurationTelemetryBindings.inc"
#include "src/Composition/Esp32TaskComposition.inc"
#include "src/Composition/FirmwareLifecycle.inc"
