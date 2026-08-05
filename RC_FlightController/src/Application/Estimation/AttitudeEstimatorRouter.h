#pragma once

#include <algorithm>
#include <cstdint>

#include "../../Submodules/EKF/AttitudeEKF.h"
#include "../../Submodules/Madgwick/MadgwickAHRS.h"
#include "../../Submodules/MahonyAHRS/MahonyAHRS.h"
#include "../../Core/RtePorts.h"

enum class AttitudeEstimatorMode : std::uint8_t {
    Ekf = 0,
    Mahony = 1,
    Madgwick = 2
};

struct AttitudeEstimatorInput {
    AHRSInput sample;
    float dtSeconds;
    std::uint8_t requestedMode;
};

// Application-layer selector for interchangeable attitude algorithms. Sensor
// acquisition remains below this component and flight control consumes only
// the common AttitudeEstimate output.
class AttitudeEstimatorRouter : public rte::SoftwareComponent {
public:
    AttitudeEstimatorRouter(AttitudeEKF& ekf, MahonyAHRS& mahony,
                            MadgwickAHRS& madgwick)
        : ekf_(ekf), mahony_(mahony), madgwick_(madgwick) {}

    AttitudeEstimatorRouter(
        AttitudeEKF& ekf, MahonyAHRS& mahony, MadgwickAHRS& madgwick,
        rte::SenderReceiverPort<AttitudeEstimatorInput>& inputPort,
        rte::SenderReceiverPort<AttitudeEstimate>& outputPort)
        : AttitudeEstimatorRouter(ekf, mahony, madgwick) {
        inputPort_ = &inputPort;
        outputPort_ = &outputPort;
    }

    void Init() override {
        ekf_.reset();
        activeMode_ = 255;
        if (outputPort_ != nullptr) outputPort_->invalidate();
    }

    void Periodic() override {
        if (inputPort_ == nullptr || outputPort_ == nullptr) return;
        rte::SignalSample<AttitudeEstimatorInput> input{};
        if (!inputPort_->receive(input)) {
            outputPort_->invalidate(input.timestampUs);
            return;
        }
        outputPort_->send(
            update(input.value.sample, input.value.dtSeconds,
                   input.value.requestedMode),
            input.timestampUs);
    }

    AttitudeEstimate update(const AHRSInput& input, float dtSeconds,
                            std::uint8_t requestedMode) {
        const auto boundedMode = static_cast<std::uint8_t>(
            std::min(2, std::max(0, static_cast<int>(requestedMode))));
        if (boundedMode != activeMode_) {
            // Preserve the original transition policy: EKF and Madgwick reset;
            // Mahony retains its state.
            ekf_.reset();
            madgwick_.reset();
            activeMode_ = boundedMode;
        }

        AttitudeEstimate output{};
        if (activeMode_ == static_cast<std::uint8_t>(AttitudeEstimatorMode::Mahony)) {
            mahony_.update(input, dtSeconds, output);
        } else if (activeMode_ ==
                   static_cast<std::uint8_t>(AttitudeEstimatorMode::Madgwick)) {
            madgwick_.update(input, dtSeconds, output);
        } else {
            ekf_.update(input, dtSeconds, output);
        }
        return output;
    }

    void resetEkf() { ekf_.reset(); }
    std::uint8_t activeMode() const { return activeMode_; }
    bool ekfMagAccepted() const { return ekf_.lastMagAccepted(); }
    float ekfRollBiasDps() const { return ekf_.rollBiasDps(); }
    float ekfPitchBiasDps() const { return ekf_.pitchBiasDps(); }
    float ekfYawBiasDps() const { return ekf_.yawBiasDps(); }

private:
    AttitudeEKF& ekf_;
    MahonyAHRS& mahony_;
    MadgwickAHRS& madgwick_;
    std::uint8_t activeMode_ = 255;
    rte::SenderReceiverPort<AttitudeEstimatorInput>* inputPort_ = nullptr;
    rte::SenderReceiverPort<AttitudeEstimate>* outputPort_ = nullptr;
};
