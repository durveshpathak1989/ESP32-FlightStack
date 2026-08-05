#pragma once

#include <cstdint>

#include "../../Submodules/GPS/GPSSensor.h"
#include "../../Core/FlightTypes.h"

// Complete immutable-at-reader-boundary telemetry snapshot. The 400 Hz control
// task is the primary writer; slower services copy it under the RTE mutex.
struct FlightState {
    float roll_deg, pitch_deg, yaw_deg;
    float q0, q1, q2, q3;
    bool imuValid;
    bool magValid;
    std::uint8_t ahrsFilterMode;
    float roll_ctrl_deg, pitch_ctrl_deg, yaw_ctrl_deg;
    float roll_offset_deg, pitch_offset_deg, yaw_offset_deg;
    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps, gz_dps;
    float mx_uT, my_uT, mz_uT;
    float imuTemp_c;
    float bmpTemp_c, bmpPressure_hpa, bmpAltitude_m;
    bool bmpValid;
    float tofDistance_m;
    std::uint16_t tofDistance_mm;
    std::uint8_t tofRangeStatus;
    std::uint8_t tofObjectCount;
    std::uint8_t tofStreamCount;
    float tofSignalMcps;
    float tofAmbientMcps;
    std::uint32_t tofAge_ms;
    std::uint32_t tofLastUpdate_ms;
    bool tofReady;
    bool tofValid;
    float cpuCore0_pct, cpuCore1_pct;
    bool cpuValid;
    float motorFL, motorFR, motorRL, motorRR;
    float batteryVoltage_v;
    float batteryAdcVoltage_v;
    float batteryCellVoltage_v;
    float batteryPercent;
    bool batteryValid;
    bool batteryLow;
    bool batteryCritical;
    float pidRollOut, pidPitchOut, pidYawOut;
    float motorFLPreSat, motorFRPreSat, motorRLPreSat, motorRRPreSat;
    std::uint32_t loopPeriod_us, imuRead_us, rcRead_us, ahrsUpdate_us;
    std::uint32_t controlUpdate_us, motorWrite_us, wifiService_us, onboardLogWrite_us;
    std::int16_t loopJitter_us;
    std::uint32_t missedLoopCount;
    float rawAx_g, rawAy_g, rawAz_g;
    float rawGx_dps, rawGy_dps, rawGz_dps;
    float filtAx_g, filtAy_g, filtAz_g;
    float filtGx_dps, filtGy_dps, filtGz_dps;
    float magNorm_uT;
    bool ekfMagUsed;
    float ekfBgx_dps, ekfBgy_dps, ekfBgz_dps;
    float targetRollDeg, targetPitchDeg, targetYawDeg;
    float targetRollRateDps, targetPitchRateDps, targetYawRateDps;
    float rollRateError_dps, pitchRateError_dps, yawRateError_dps;
    float yawError_deg;
    bool motorSaturated;
    bool angleModeActive, acroModeActive;
    std::uint32_t rcFailsafeCount;
    std::uint16_t modeSwitchRaw_us, armSwitchRaw_us;
    std::uint16_t auxTune1Raw_us, auxTune2Raw_us;
    bool angleLoopEnabled, rateLoopEnabled;
    float actualRollRate_dps, actualPitchRate_dps, actualYawRate_dps;
    float angleRollP, angleRollI, angleRollD;
    float anglePitchP, anglePitchI, anglePitchD;
    float rateRollP, rateRollI, rateRollD;
    float ratePitchP, ratePitchI, ratePitchD;
    float rateYawP, rateYawI, rateYawD;
    float angleRollIterm, anglePitchIterm;
    std::uint32_t pidResetCount;
    std::uint32_t modeTransitionCount, lastModeChange_ms;
    std::uint32_t armingTransitionCount, lastArmChange_ms;
    bool throttleLow;
    float controlAuthorityRemaining;
    bool rollOutputLimited, pitchOutputLimited, yawOutputLimited, rateOutputLimited;
    float cmdRpmFL, cmdRpmFR, cmdRpmRL, cmdRpmRR;
    float actualRpmFL, actualRpmFR, actualRpmRL, actualRpmRR;
    bool rpmActualValid;
    float bmpVerticalSpeed_mps;
    float accelRoll_deg, accelPitch_deg;
    float gyroRoll_deg, gyroPitch_deg, gyroYaw_deg;
    float rollAngleError_deg, pitchAngleError_deg;
    flight::PilotCommand rc;
    float rcFrameRateHz;
    bool armed;
    std::uint32_t loopCount;
    GPSData gps;
};
