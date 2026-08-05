#pragma once

#include <cstdint>

#include "../../Core/RtePorts.h"

struct LevelTrimInput {
    bool switchHigh;
    bool canCapture;
    std::uint32_t nowMs;
    std::uint32_t sampleDurationMs;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
};

struct LevelTrimUpdate {
    bool captureStarted = false;
    bool captureCompleted = false;
};

struct LevelTrimOutput {
    LevelTrimUpdate event;
    float rollOffsetDeg;
    float pitchOffsetDeg;
    float yawOffsetDeg;
};

// Captures a software level reference while disarmed. This component owns the
// capture state and offsets; callers own switch validation, time, and logging.
class LevelTrimService : public rte::SoftwareComponent {
public:
    LevelTrimService() = default;
    LevelTrimService(rte::SenderReceiverPort<LevelTrimInput>& inputPort,
                     rte::SenderReceiverPort<LevelTrimOutput>& outputPort)
        : inputPort_(&inputPort), outputPort_(&outputPort) {}

    void Init() override {
        switchHandled_ = false;
        clearCapture();
        rollOffsetDeg_ = pitchOffsetDeg_ = yawOffsetDeg_ = 0.0f;
        if (outputPort_ != nullptr) outputPort_->invalidate();
    }

    void Periodic() override {
        if (inputPort_ == nullptr || outputPort_ == nullptr) return;
        rte::SignalSample<LevelTrimInput> input{};
        if (!inputPort_->receive(input)) {
            outputPort_->invalidate(input.timestampUs);
            return;
        }
        const LevelTrimUpdate event = update(
            input.value.switchHigh, input.value.canCapture,
            input.value.nowMs, input.value.sampleDurationMs,
            input.value.rollDeg, input.value.pitchDeg, input.value.yawDeg);
        outputPort_->send(
            {event, rollOffsetDeg_, pitchOffsetDeg_, yawOffsetDeg_},
            input.timestampUs);
    }

    LevelTrimUpdate update(bool switchHigh, bool canCapture,
                           std::uint32_t nowMs, std::uint32_t sampleDurationMs,
                           float rollDeg, float pitchDeg, float yawDeg) {
        LevelTrimUpdate event{};
        if (!switchHigh) {
            switchHandled_ = false;
            clearCapture();
            return event;
        }
        if (!canCapture || switchHandled_) return event;

        if (!capturing_) {
            capturing_ = true;
            startMs_ = nowMs;
            event.captureStarted = true;
        }
        rollSum_ += rollDeg;
        pitchSum_ += pitchDeg;
        yawSum_ += yawDeg;
        ++sampleCount_;

        if ((nowMs - startMs_) >= sampleDurationMs && sampleCount_ > 0) {
            rollOffsetDeg_ = static_cast<float>(rollSum_ / sampleCount_);
            pitchOffsetDeg_ = static_cast<float>(pitchSum_ / sampleCount_);
            yawOffsetDeg_ = static_cast<float>(yawSum_ / sampleCount_);
            switchHandled_ = true;
            clearCapture();
            event.captureCompleted = true;
        }
        return event;
    }

    float controlRollDeg(float rawDeg) const { return rawDeg - rollOffsetDeg_; }
    float controlPitchDeg(float rawDeg) const { return rawDeg - pitchOffsetDeg_; }
    float controlYawDeg(float rawDeg) const {
        return wrapDeg180(rawDeg - yawOffsetDeg_);
    }
    float rollOffsetDeg() const { return rollOffsetDeg_; }
    float pitchOffsetDeg() const { return pitchOffsetDeg_; }
    float yawOffsetDeg() const { return yawOffsetDeg_; }

private:
    static float wrapDeg180(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    void clearCapture() {
        capturing_ = false;
        sampleCount_ = 0;
        startMs_ = 0;
        rollSum_ = pitchSum_ = yawSum_ = 0.0;
    }

    bool switchHandled_ = false;
    bool capturing_ = false;
    std::uint32_t sampleCount_ = 0;
    std::uint32_t startMs_ = 0;
    double rollSum_ = 0.0;
    double pitchSum_ = 0.0;
    double yawSum_ = 0.0;
    float rollOffsetDeg_ = 0.0f;
    float pitchOffsetDeg_ = 0.0f;
    float yawOffsetDeg_ = 0.0f;
    rte::SenderReceiverPort<LevelTrimInput>* inputPort_ = nullptr;
    rte::SenderReceiverPort<LevelTrimOutput>* outputPort_ = nullptr;
};
