# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-08-03

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
| Release (`VERBOSE_ON=0`) | 1,138,823 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,138,976 bytes |
| Debug (`VERBOSE_ON=1`) | 1,150,115 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,150,256 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V6.0.0
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 333e251845a0
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260803T203310Z-master-333e251845a0
FW_BUILD_TIME_ISO: 2026-08-03T20:33:10Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            F6B6A9CDD0FF9C443CAB1A880456F9C14F14D856235194A9D8D8BBDC1B5BCBDC
bin_release/RC_FlightController.ino.merged.bin     55F0D7ABE027969EF861F8106E6EB7A5E99BCC54F3230BDDAEAC6C191CF896D0
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            3FB7A6BFEADF21E30DC4442627AEB7B2B25B0B602885978EA03D59D0DEB4E3F5
bin_debug/RC_FlightController.ino.merged.bin     268543572E23AAFFF00CD7A136A8B8B339234B27DB354126F9F85FCE5EEB3265
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
