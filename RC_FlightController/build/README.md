# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-07-27

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
| Release (`VERBOSE_ON=0`) | 1,138,803 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,138,944 bytes |
| Debug (`VERBOSE_ON=1`) | 1,150,103 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,150,256 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.5.1
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: e413dfc97e6a
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260727T022516Z-master-e413dfc97e6a
FW_BUILD_TIME_ISO: 2026-07-27T02:25:16Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            DC8ED8D798668017568F9149092F5E9678843D9126B575E9C5808ECE8B78679D
bin_release/RC_FlightController.ino.merged.bin     E1922DE8017A180F23D5ABB6411B894F071DF8BC9DD17A29658F975A3C87070E
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            183A4419622F6F8184F42630E3F0A38D9C62E203CCE732F2DC17C8691F80F735
bin_debug/RC_FlightController.ino.merged.bin     03CEA8305FB98E09093B6BFEF25EB0D94F400F89B5A4849E2B1C56D7BCD7EA29
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
