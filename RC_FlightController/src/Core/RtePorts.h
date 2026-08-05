#pragma once

#include <cstdint>

namespace rte {

enum class DataValidity : std::uint8_t {
    NeverReceived = 0,
    Valid = 1,
    Invalid = 2
};

template <typename T>
struct SignalSample {
    T value{};
    std::uint64_t timestampUs = 0;
    std::uint32_t sequence = 0;
    DataValidity validity = DataValidity::NeverReceived;
};

// One sender / one-or-more receiver implicit sender-receiver port. Platform
// compositions may wrap access in a synchronized channel when tasks cross.
template <typename T>
class SenderReceiverPort {
public:
    void send(const T& value, std::uint64_t timestampUs = 0) {
        sample_.value = value;
        sample_.timestampUs = timestampUs;
        ++sample_.sequence;
        sample_.validity = DataValidity::Valid;
    }

    void invalidate(std::uint64_t timestampUs = 0) {
        sample_.timestampUs = timestampUs;
        ++sample_.sequence;
        sample_.validity = DataValidity::Invalid;
    }

    bool receive(SignalSample<T>& destination) const {
        destination = sample_;
        return destination.validity == DataValidity::Valid;
    }

private:
    SignalSample<T> sample_{};
};

// All software components expose these two runnables. Init executes once after
// ports are connected; Periodic executes only in the component's declared task.
class SoftwareComponent {
public:
    virtual ~SoftwareComponent() = default;
    virtual void Init() = 0;
    virtual void Periodic() = 0;
};

}  // namespace rte
