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
FW_VERSION: V5.4.0-6-g416381b
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 416381b47066
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260701T012259Z-master-416381b47066
FW_BUILD_TIME_ISO: 2026-07-01T01:22:59Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            46EDBE64E766454220E0291541DA5F3870B1CC6C672DBA920FCE6A77EBADC57E
bin_release/RC_FlightController.ino.merged.bin     B5683B6E39ECF5B6910E121EC29BE0A337AF71E260CB19AC60AE74DF14617135
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            8AFBC90FD0719F193CA04E5AE15F8CED94F92E159C58C23F780386BA85AB8C7E
bin_debug/RC_FlightController.ino.merged.bin     BEB0257F74CB820918582AB2233438EE4FAA1D0E3F59FF615A6A261ED6E3BB73
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
