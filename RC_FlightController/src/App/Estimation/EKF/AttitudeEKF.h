/*
 * Name: AttitudeEKF.h
 * Use: 18-state inertial EKF for attitude, angular rate, gyro bias,
 *      world acceleration, velocity, and position estimation.
 * Version: 5.0.0-experimental
 * Created by: Durvesh Pathak dp676@cornell.edu
 */

#pragma once
#ifndef ATTITUDE_EKF_H
#define ATTITUDE_EKF_H

#include "../AHRS/AHRSCommon.h"
#include <stdint.h>

struct PositionVelocityEstimate {
    float posX_m = 0.0f;
    float posY_m = 0.0f;
    float posZ_m = 0.0f;

    float velX_mps = 0.0f;
    float velY_mps = 0.0f;
    float velZ_mps = 0.0f;

    float accelX_mps2 = 0.0f;
    float accelY_mps2 = 0.0f;
    float accelZ_mps2 = 0.0f;

    bool valid = false;
};

struct BodyRateEstimate {
    float rollRate_dps = 0.0f;
    float pitchRate_dps = 0.0f;
    float yawRate_dps = 0.0f;
    bool valid = false;
};

class AttitudeEKF {
public:
    static constexpr int STATE_DIM = 18;

    AttitudeEKF();

    void reset();
    void resetPositionVelocity();

    // Backward-compatible tuning API.
    void setProcessNoise(float angleQ, float biasQ);
    void setAccelMeasurementNoise(float accelAngleR);
    void setMagMeasurementNoise(float magYawR);
    void setMagDeclinationDeg(float declinationDeg);
    void setMagYawOffsetDeg(float offsetDeg);
    void setMagYawSign(float sign);

    // New full-state tuning hooks.
    void setKinematicProcessNoise(float positionQ, float velocityQ, float worldAccelQ);
    void setRateProcessNoise(float bodyRateQ);
    void setGyroMeasurementNoise(float gyroRateR);
    void setWorldAccelMeasurementNoise(float worldAccelR);
    void setTofMeasurementNoise(float altitudeR, float verticalVelocityR);

    bool update(const AHRSInput& in, float dt, AttitudeEstimate& out);

    // Optional absolute/aiding measurements. These are intentionally separate
    // from update() so GPS, optical flow, barometer, and ToF can be fused at
    // their own sample rates.
    void updatePositionMeasurement(float x_m, float y_m, float z_m,
                                   float varianceX, float varianceY, float varianceZ);
    void updateVelocityMeasurement(float vx_mps, float vy_mps, float vz_mps,
                                   float varianceX, float varianceY, float varianceZ);
    void updateAltitudeMeasurement(float z_m, float variance);

    // Fuse one fresh downward-facing ToF sample. The range is tilt-compensated
    // into vertical altitude. Consecutive timestamps are differentiated to
    // produce a guarded vertical-velocity measurement for VZ correction.
    void updateTofMeasurement(float rangeM, uint32_t sampleTsMs);

    // Backward-compatible altitude hook. BMP corrects PZ; ToF corrects PZ and,
    // when a second fresh sample is available, VZ.
    void updateAltitudeSensors(float bmpAltM, bool bmpValid,
                               float tofAltM, bool tofValid, uint32_t tsMs);

    float rollBiasDps() const;
    float pitchBiasDps() const;
    float yawBiasDps() const;

    float rollRateDps() const;
    float pitchRateDps() const;
    float yawRateDps() const;

    bool lastMagAccepted() const { return _lastMagAccepted; }
    bool lastTofVelocityAccepted() const { return _lastTofVelocityAccepted; }
    const PositionVelocityEstimate& positionVelocity() const { return _posVel; }
    const BodyRateEstimate& bodyRates() const { return _bodyRates; }

    float state(int index) const;
    float covariance(int row, int col) const;

private:
    enum StateIndex : int {
        PX = 0, PY, PZ,
        VX, VY, VZ,
        AX, AY, AZ,
        ROLL, PITCH, YAW,
        WX, WY, WZ,
        BGX, BGY, BGZ
    };

    float _x[STATE_DIM];
    float _cov[STATE_DIM][STATE_DIM];

    float _positionQ;
    float _velocityQ;
    float _worldAccelQ;
    float _angleQ;
    float _bodyRateQ;
    float _biasQ;

    float _gyroRateR;
    float _accelAngleR;
    float _worldAccelR;
    float _magYawR;
    float _tofAltitudeR;
    float _tofVelocityR;

    float _declinationRad;
    float _magYawOffsetRad;
    float _magYawSign;

    bool _lastMagAccepted;
    bool _lastTofVelocityAccepted;
    PositionVelocityEstimate _posVel;
    BodyRateEstimate _bodyRates;

    float _lastBmpAltM;
    float _lastTofAltM;
    bool _lastBmpValid;
    bool _lastTofValid;
    uint32_t _lastAltTsMs;

    bool _tofHistoryValid;
    float _previousTofVerticalM;
    float _filteredTofVelocityMps;
    uint32_t _previousTofTsMs;

    void _predict(float dt);
    void _predictCovariance(float dt);
    void _addProcessNoise();

    void _updateScalar(const float h[STATE_DIM], float measurement,
                       float variance, bool wrapAngleInnovation = false);
    void _updateDirectState(int stateIndex, float measurement,
                            float variance, bool wrapAngleInnovation = false);
    void _updateGyro(const AHRSInput& in);
    void _updateAccelTilt(const AHRSInput& in);
    void _updateWorldAcceleration(const AHRSInput& in);

    bool _magYawRad(const AHRSInput& in, float& yawRad) const;
    void _syncOutputs();
    void _stabilizeCovariance();

    static void _quatFromEuler(float roll, float pitch, float yaw, AttitudeEstimate& out);
};

#endif // ATTITUDE_EKF_H
