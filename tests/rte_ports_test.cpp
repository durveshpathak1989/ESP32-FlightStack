#include <cassert>

#include "Core/RtePorts.h"

struct Payload { int value; };

int main() {
    rte::SenderReceiverPort<Payload> port;
    rte::SignalSample<Payload> sample{};
    assert(!port.receive(sample));
    assert(sample.validity == rte::DataValidity::NeverReceived);

    port.send({42}, 1000);
    assert(port.receive(sample));
    assert(sample.value.value == 42);
    assert(sample.timestampUs == 1000);
    assert(sample.sequence == 1);

    port.invalidate(1200);
    assert(!port.receive(sample));
    assert(sample.validity == rte::DataValidity::Invalid);
    assert(sample.sequence == 2);
}
