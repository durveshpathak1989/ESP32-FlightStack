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
FW_SOURCE_COMMIT: 6ca42c1e3b25
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260628T155741Z-master-6ca42c1e3b25
FW_BUILD_TIME_ISO: 2026-06-28T15:57:41Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            094F50F676E7F541636157516FEB8CA7A843FA36C9AEFF6714D64733770E3258
bin_release/RC_FlightController.ino.merged.bin     11962B703946AB5DB50712AB5672005AA35689CD7BC595E538A59FB12CFCD641
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            41EDEA5ED8DA264464AB01D853F7BE94B8BFD30F18E1330B1991AD06582B2A1A
bin_debug/RC_FlightController.ino.merged.bin     BC7F5628687DDF111C7BB1895C70D32328C70ACB217D7C9ECCD2A996C00A0302
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
