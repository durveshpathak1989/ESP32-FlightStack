#pragma once

#include <algorithm>
#include <cmath>

struct MotorMixerConfig {
    float throttleExpo;
    float throttleUpRatePerSecond;
    float throttleDownRatePerSecond;
    float motorIdle;
    float motorMaximum;
    float throttleCut;
    float idleRampEnd;
};

struct MotorMixerInput {
    float throttle;
    float rollCorrection;
    float pitchCorrection;
    float yawCorrection;
    float dtSeconds;
};

struct MotorMixerOutput {
    float shapedThrottle;
    float frontLeftPreSaturation;
    float frontRightPreSaturation;
    float rearLeftPreSaturation;
    float rearRightPreSaturation;
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;
    float maximumBeforeDesaturation;
    float controlAuthorityRemaining;
    bool saturated;
};

// Portable, deterministic Quad-X mixer. It owns only throttle slew state and
// performs no I/O, allocation, logging, or synchronization.
class MotorMixer {
public:
    MotorMixerOutput update(const MotorMixerInput& input,
                            const MotorMixerConfig& config) {
        const float rawThrottle = clamp(input.throttle, 0.0f, 1.0f);
        const float target = rawThrottle > config.throttleCut
                                 ? applyExpo(rawThrottle, config.throttleExpo)
                                 : 0.0f;
        const float maxStepUp = config.throttleUpRatePerSecond * input.dtSeconds;
        const float maxStepDown =
            config.throttleDownRatePerSecond * input.dtSeconds;
        if (target > smoothedThrottle_)
            smoothedThrottle_ += std::min(target - smoothedThrottle_, maxStepUp);
        else
            smoothedThrottle_ -= std::min(smoothedThrottle_ - target, maxStepDown);

        MotorMixerOutput output{};
        output.shapedThrottle = clamp(smoothedThrottle_, 0.0f, 1.0f);
        output.frontLeftPreSaturation = output.shapedThrottle + input.rollCorrection -
                                        input.pitchCorrection - input.yawCorrection;
        output.frontRightPreSaturation = output.shapedThrottle - input.rollCorrection -
                                         input.pitchCorrection + input.yawCorrection;
        output.rearLeftPreSaturation = output.shapedThrottle + input.rollCorrection +
                                       input.pitchCorrection + input.yawCorrection;
        output.rearRightPreSaturation = output.shapedThrottle - input.rollCorrection +
                                        input.pitchCorrection - input.yawCorrection;
        output.frontLeft = output.frontLeftPreSaturation;
        output.frontRight = output.frontRightPreSaturation;
        output.rearLeft = output.rearLeftPreSaturation;
        output.rearRight = output.rearRightPreSaturation;
        output.maximumBeforeDesaturation = std::max(
            std::max(output.frontLeft, output.frontRight),
            std::max(output.rearLeft, output.rearRight));
        output.controlAuthorityRemaining =
            std::max(0.0f, config.motorMaximum - output.maximumBeforeDesaturation);

        if (output.maximumBeforeDesaturation > config.motorMaximum) {
            output.saturated = true;
            const float excess =
                output.maximumBeforeDesaturation - config.motorMaximum;
            output.frontLeft -= excess;
            output.frontRight -= excess;
            output.rearLeft -= excess;
            output.rearRight -= excess;
        }

        const float idleBlend = smoothStep(
            (output.shapedThrottle - config.throttleCut) /
            (config.idleRampEnd - config.throttleCut));
        const float motorMinimum = config.motorIdle * idleBlend;
        if (output.shapedThrottle > config.throttleCut) {
            output.frontLeft = clamp(output.frontLeft, motorMinimum, config.motorMaximum);
            output.frontRight = clamp(output.frontRight, motorMinimum, config.motorMaximum);
            output.rearLeft = clamp(output.rearLeft, motorMinimum, config.motorMaximum);
            output.rearRight = clamp(output.rearRight, motorMinimum, config.motorMaximum);
        } else {
            output.frontLeft = output.frontRight = 0.0f;
            output.rearLeft = output.rearRight = 0.0f;
        }
        return output;
    }

    void reset() { smoothedThrottle_ = 0.0f; }
    float smoothedThrottle() const { return smoothedThrottle_; }

private:
    static float clamp(float value, float minimum, float maximum) {
        return std::max(minimum, std::min(value, maximum));
    }

    static float applyExpo(float value, float expo) {
        const float x = clamp(value, 0.0f, 1.0f);
        const float e = clamp(expo, 0.0f, 1.0f);
        return (1.0f - e) * x + e * x * x * x;
    }

    static float smoothStep(float value) {
        const float x = clamp(value, 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    float smoothedThrottle_ = 0.0f;
};
