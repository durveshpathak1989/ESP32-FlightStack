#pragma once

struct OtaSafetyInput {
    bool armed;
    float throttle;
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;
};

// Side-effect-free safety decisions shared by runtime adapters. Hardware
// shutdown remains the motor adapter's responsibility.
class FlightSafetyPolicy {
public:
    static bool controlMustStop(bool disarmed, bool failsafe,
                                bool calibrationBlocksFlight) {
        return disarmed || failsafe || calibrationBlocksFlight;
    }

    static bool otaAllowed(const OtaSafetyInput& input,
                           float throttleCut = 0.03f,
                           float motorOffThreshold = 0.001f) {
        const float motorMaximum = maximum(
            maximum(input.frontLeft, input.frontRight),
            maximum(input.rearLeft, input.rearRight));
        return !input.armed && input.throttle <= throttleCut &&
               motorMaximum <= motorOffThreshold;
    }

private:
    static float maximum(float left, float right) {
        return left > right ? left : right;
    }
};
