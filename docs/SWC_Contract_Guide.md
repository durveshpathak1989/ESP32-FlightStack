# SWC Contract Guide

## Contract shape

Every SWC has:

- an input struct with explicit units and `version = 1`;
- an output struct with explicit units and `version = 1`;
- `init`, `update`, and `reset` methods;
- no blocking I/O in service-layer `update` methods;
- deterministic behavior for the same input and internal state.

## Service-layer rules

- Do not include Arduino, FreeRTOS, or ESP32 headers.
- Do not call sensor, motor, network, storage, or delay APIs.
- Do not allocate heap memory in `update`.
- Pass time through `dtUs` and `timestampUs`.
- Keep state private to the SWC instance.

## Component-layer rules

Components convert hardware-facing data into service contracts, enforce rate
contracts, and route outputs. They should not contain flight-control policy
that belongs in Services or Application.

## Example

```cpp
struct SWC_ExampleInput {
    float measurement = 0.0f;
    std::uint64_t dtUs = 2500;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

struct SWC_ExampleOutput {
    float command = 0.0f;
    std::uint64_t timestampUs = 0;
    std::uint8_t version = 1;
};

class SWC_Example {
public:
    void init(const SWC_ExampleInput& initial);
    SWC_ExampleOutput update(const SWC_ExampleInput& input);
    void reset();
};
```

## Validation checklist

- Host test covers nominal and edge cases.
- Platform leak grep returns no matches.
- Timing budget is documented.
- Output saturation behavior is tested.
- Reset clears integral, filters, and latches.
