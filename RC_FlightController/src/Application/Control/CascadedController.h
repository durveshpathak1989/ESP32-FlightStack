#pragma once

#include <algorithm>
#include <cmath>

#include "LowPassFilter.h"
#include "PidController.h"
#include "../Configuration/TuningState.h"
#include "../../Core/RtePorts.h"

struct CascadedControlInput {
    float rollCommand;
    float pitchCommand;
    float yawCommand;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    float rollRateDps;
    float pitchRateDps;
    float yawRateDps;
    float dtSeconds;
    float commandLpfHz;
    bool angleMode;
    bool attitudeValid;
};

struct CascadedControlOutput {
    float filteredRollCommand;
    float filteredPitchCommand;
    float filteredYawCommand;
    float targetRollDeg;
    float targetPitchDeg;
    float targetYawDeg;
    float targetRollRateDps;
    float targetPitchRateDps;
    float targetYawRateDps;
    float rollAngleErrorDeg;
    float pitchAngleErrorDeg;
    float yawErrorDeg;
    float rollRateErrorDps;
    float pitchRateErrorDps;
    float yawRateErrorDps;
    float rollFeedForward;
    float pitchFeedForward;
    float yawFeedForward;
    float rollCorrection;
    float pitchCorrection;
    float yawCorrection;
    bool rollLimited;
    bool pitchLimited;
    bool yawLimited;
    bool anyRateOutputLimited;
};

// Portable flight-control SWC. It owns command-filter and yaw-hold state while
// the injected PID instances retain their existing gains and diagnostic terms.
class CascadedController : public rte::SoftwareComponent {
public:
    CascadedController(PidController& rateRoll, PidController& ratePitch,
                       PidController& rateYaw, PidController& angleRoll,
                       PidController& anglePitch, PidController& angleYaw)
        : rateRoll_(rateRoll), ratePitch_(ratePitch), rateYaw_(rateYaw),
          angleRoll_(angleRoll), anglePitch_(anglePitch), angleYaw_(angleYaw) {}

    CascadedController(
        PidController& rateRoll, PidController& ratePitch,
        PidController& rateYaw, PidController& angleRoll,
        PidController& anglePitch, PidController& angleYaw,
        rte::SenderReceiverPort<CascadedControlInput>& inputPort,
        rte::SenderReceiverPort<TuningState>& configPort,
        rte::SenderReceiverPort<CascadedControlOutput>& outputPort)
        : CascadedController(rateRoll, ratePitch, rateYaw, angleRoll,
                             anglePitch, angleYaw) {
        inputPort_ = &inputPort;
        configPort_ = &configPort;
        outputPort_ = &outputPort;
    }

    void Init() override {
        reset();
        if (outputPort_ != nullptr) outputPort_->invalidate();
    }

    void Periodic() override {
        if (inputPort_ == nullptr || configPort_ == nullptr || outputPort_ == nullptr)
            return;
        rte::SignalSample<CascadedControlInput> input{};
        rte::SignalSample<TuningState> config{};
        if (!inputPort_->receive(input) || !configPort_->receive(config)) {
            outputPort_->invalidate(input.timestampUs);
            return;
        }
        outputPort_->send(update(input.value, config.value), input.timestampUs);
    }

    CascadedControlOutput update(const CascadedControlInput& input,
                                 const TuningState& tuning) {
        CascadedControlOutput out{};
        out.filteredRollCommand = rollCommandFilter_.update(
            input.rollCommand, input.dtSeconds, input.commandLpfHz);
        out.filteredPitchCommand = pitchCommandFilter_.update(
            input.pitchCommand, input.dtSeconds, input.commandLpfHz);
        out.filteredYawCommand = yawCommandFilter_.update(
            input.yawCommand, input.dtSeconds, input.commandLpfHz);

        if (input.angleMode) {
            out.targetRollDeg = out.filteredRollCommand * tuning.max_angle_deg;
            out.targetPitchDeg = out.filteredPitchCommand * tuning.max_angle_deg;
            out.rollAngleErrorDeg = out.targetRollDeg - input.rollDeg;
            out.pitchAngleErrorDeg = out.targetPitchDeg - input.pitchDeg;
            out.targetRollRateDps = angleRoll_.update(
                out.rollAngleErrorDeg, input.dtSeconds);
            out.targetPitchRateDps = anglePitch_.update(
                out.pitchAngleErrorDeg, input.dtSeconds);
        } else {
            out.targetRollRateDps =
                out.filteredRollCommand * tuning.max_rate_dps;
            out.targetPitchRateDps =
                out.filteredPitchCommand * tuning.max_pitch_rate_dps;
        }

        out.rollRateErrorDps = out.targetRollRateDps - input.rollRateDps;
        out.pitchRateErrorDps = out.targetPitchRateDps - input.pitchRateDps;
        out.rollFeedForward = tuning.pid_roll_ff * out.targetRollRateDps;
        out.pitchFeedForward = tuning.pid_pitch_ff * out.targetPitchRateDps;
        out.rollCorrection = out.rollFeedForward + rateRoll_.updateDOnMeasurement(
            out.rollRateErrorDps, input.rollRateDps, input.dtSeconds,
            tuning.pid_roll_d_lpf_hz);
        out.pitchCorrection = out.pitchFeedForward + ratePitch_.updateDOnMeasurement(
            out.pitchRateErrorDps, input.pitchRateDps, input.dtSeconds,
            tuning.pid_pitch_d_lpf_hz);

        if (input.attitudeValid &&
            std::fabs(out.filteredYawCommand) < tuning.yaw_deadband) {
            if (!yawHoldActive_) {
                yawSetpointDeg_ = input.yawDeg;
                yawHoldActive_ = true;
                angleYaw_.reset();
            }
            out.targetYawDeg = yawSetpointDeg_;
            out.yawErrorDeg = wrapDeg180(yawSetpointDeg_ - input.yawDeg);
            out.targetYawRateDps = clamp(
                angleYaw_.update(out.yawErrorDeg, input.dtSeconds),
                -tuning.yaw_max_rate_dps, tuning.yaw_max_rate_dps);
        } else {
            yawHoldActive_ = false;
            out.targetYawDeg = input.yawDeg;
            out.targetYawRateDps =
                -out.filteredYawCommand * tuning.yaw_max_rate_dps;
        }
        out.yawRateErrorDps = out.targetYawRateDps - input.yawRateDps;
        out.yawFeedForward = tuning.pid_yaw_ff * out.targetYawRateDps;
        out.yawCorrection = out.yawFeedForward + rateYaw_.updateDOnMeasurement(
            out.yawRateErrorDps, input.yawRateDps, input.dtSeconds,
            tuning.pid_yaw_d_lpf_hz);

        out.rollLimited = outside(out.rollCorrection, tuning.roll_output_limit);
        out.pitchLimited = outside(out.pitchCorrection, tuning.pitch_output_limit);
        out.yawLimited = outside(out.yawCorrection, tuning.yaw_output_limit);
        out.anyRateOutputLimited =
            out.rollLimited || out.pitchLimited || out.yawLimited;
        out.rollCorrection = clamp(out.rollCorrection,
                                   -tuning.roll_output_limit,
                                   tuning.roll_output_limit);
        out.pitchCorrection = clamp(out.pitchCorrection,
                                    -tuning.pitch_output_limit,
                                    tuning.pitch_output_limit);
        out.yawCorrection = clamp(out.yawCorrection,
                                  -tuning.yaw_output_limit,
                                  tuning.yaw_output_limit);
        return out;
    }

    void reset() {
        rateRoll_.reset();
        ratePitch_.reset();
        rateYaw_.reset();
        angleRoll_.reset();
        anglePitch_.reset();
        angleYaw_.reset();
        rollCommandFilter_.reset();
        pitchCommandFilter_.reset();
        yawCommandFilter_.reset();
        yawHoldActive_ = false;
    }

    bool yawHoldActive() const { return yawHoldActive_; }
    float yawSetpointDeg() const { return yawSetpointDeg_; }

private:
    static float clamp(float value, float low, float high) {
        return std::max(low, std::min(value, high));
    }

    static bool outside(float value, float magnitudeLimit) {
        return value > magnitudeLimit || value < -magnitudeLimit;
    }

    static float wrapDeg180(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    PidController& rateRoll_;
    PidController& ratePitch_;
    PidController& rateYaw_;
    PidController& angleRoll_;
    PidController& anglePitch_;
    PidController& angleYaw_;
    LowPassFilter rollCommandFilter_;
    LowPassFilter pitchCommandFilter_;
    LowPassFilter yawCommandFilter_;
    float yawSetpointDeg_ = 0.0f;
    bool yawHoldActive_ = false;
    rte::SenderReceiverPort<CascadedControlInput>* inputPort_ = nullptr;
    rte::SenderReceiverPort<TuningState>* configPort_ = nullptr;
    rte::SenderReceiverPort<CascadedControlOutput>* outputPort_ = nullptr;
};
