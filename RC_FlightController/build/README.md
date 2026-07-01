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
| Release (`VERBOSE_ON=0`) | 1,128,611 bytes | 1,966,080 bytes | 76,116 bytes | 327,680 bytes | 251,564 bytes | 1,128,752 bytes |
| Debug (`VERBOSE_ON=1`) | 1,139,603 bytes | 1,966,080 bytes | 76,116 bytes | 327,680 bytes | 251,564 bytes | 1,139,744 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.4.0-16-ge77429e
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: e77429ed1275
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260701T024145Z-master-e77429ed1275
FW_BUILD_TIME_ISO: 2026-07-01T02:41:45Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            0943B6486F0D9D2B8ABE973ED469BE792BFF74D9BCD33FC01383220843B1214E
bin_release/RC_FlightController.ino.merged.bin     061AD0739E40630AEB29A279705D1426473717DCC3B1727C98ADB21521BB6D94
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            28A5A3311ABDA078C8E9AF64EFE6328DD665ED59BDACB082899C6C96B5FEFF39
bin_debug/RC_FlightController.ino.merged.bin     ADF369021D3C90B245DE951BE8DC1C9ABF756B9B84F0B7E95675D6DBB1F32927
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
