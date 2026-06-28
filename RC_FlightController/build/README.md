# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-06-28

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
| Release (`VERBOSE_ON=0`) | 1,082,443 bytes | 1,966,080 bytes | 66,420 bytes | 327,680 bytes | 261,260 bytes | 1,082,592 bytes |
| Debug (`VERBOSE_ON=1`) | 1,093,639 bytes | 1,966,080 bytes | 66,420 bytes | 327,680 bytes | 261,260 bytes | 1,093,792 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.2.0
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: b9c6755b65f2
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260628T153606Z-master-b9c6755b65f2
FW_BUILD_TIME_ISO: 2026-06-28T15:36:06Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            7F13E6BFC41612A4CEA585205E12C0C9020382D84A9752B240413D187DDD1906
bin_release/RC_FlightController.ino.merged.bin     9FD7F5DD5A16D67BD6C200F6EE1E76B6CDCB3D5EABEA11C2D0916F2CB64D0A08
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            A25D88C6C76609855C0748755B9A83A2EAF2D4D6DAEDD01151C466C8A89AF874
bin_debug/RC_FlightController.ino.merged.bin     59908AAB9CBC913D1781C26495A280693862254E72B3C7678B9A6CE221B03569
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
