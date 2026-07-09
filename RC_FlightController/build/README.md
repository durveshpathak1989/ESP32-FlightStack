# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-07-09

## Build Variants

| Folder | Verbose flag | Purpose |
| --- | --- | --- |
| `bin_release/` | `VERBOSE_ON=0` | Normal OTA/release firmware. |
| `bin_debug/` | `VERBOSE_ON=1` | Debug firmware with verbose diagnostics enabled. |

The root `RC_FlightController.ino.bin` and `RC_FlightController.ino.merged.bin` files mirror the release build for compatibility with the existing OTA workflow.

## Main OTA Files

Release:

```text
bin_release/RC_FlightController.ino.bin
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin
```

## Build Metrics

| Variant | Program storage | Flash max | RAM globals | RAM max | Free RAM | OTA `.bin` size |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Release (`VERBOSE_ON=0`) | 1,138,695 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,138,848 bytes |
| Debug (`VERBOSE_ON=1`) | 1,149,995 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,150,144 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.4.0-19-g6787b2f
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 6787b2f92d2a
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260709T030553Z-master-6787b2f92d2a
FW_BUILD_TIME_ISO: 2026-07-09T03:05:53Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            8812761404A05FAED6D90FCB4EF50B95F277AAA9BBDDC159280F2AACAB38F68B
bin_release/RC_FlightController.ino.merged.bin     AFB641B1C30CA5EC3821AD4DE7AC38CF9E75682ECDBDD85699A35E95F5EFA848
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            7101120C8AB44F06826D2DD621441E814C82F5D76AD96B5544E9E15D820A98E3
bin_debug/RC_FlightController.ino.merged.bin     3F1E187BCF51D6AAF9F643E3313017C41FAADBE864DAEB68957CDB1C5140FF35
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
