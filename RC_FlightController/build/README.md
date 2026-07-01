# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-07-01

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
| Release (`VERBOSE_ON=0`) | 1,127,419 bytes | 1,966,080 bytes | 76,028 bytes | 327,680 bytes | 251,652 bytes | 1,127,568 bytes |
| Debug (`VERBOSE_ON=1`) | 1,138,395 bytes | 1,966,080 bytes | 76,028 bytes | 327,680 bytes | 251,652 bytes | 1,138,544 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.4.0-8-g86ab81b
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 86ab81b5d250
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260701T014251Z-master-86ab81b5d250
FW_BUILD_TIME_ISO: 2026-07-01T01:42:51Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            CABD55A97517FFA187914B1BDB6EC8FFD0E510D1998DED0C63EBC6E8C480A890
bin_release/RC_FlightController.ino.merged.bin     29EC3ECE2DA6544F294FBFCD4440369E437F8AACA300E989C149B6AF16418117
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            EE6C39953609E333F649F1B9B2F89EE04F61808C1849EA1D67308B925FBFC541
bin_debug/RC_FlightController.ino.merged.bin     3CCEE86B1DCC0A994EBC99BB2DD140128AC93AA90869817B77270DAB49025054
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
