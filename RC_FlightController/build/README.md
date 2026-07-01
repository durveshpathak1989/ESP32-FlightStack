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
| Release (`VERBOSE_ON=0`) | 1,127,899 bytes | 1,966,080 bytes | 76,076 bytes | 327,680 bytes | 251,604 bytes | 1,128,048 bytes |
| Debug (`VERBOSE_ON=1`) | 1,138,867 bytes | 1,966,080 bytes | 76,084 bytes | 327,680 bytes | 251,596 bytes | 1,139,008 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.4.0-14-g14ecc4a
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 14ecc4a25a83
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260701T022956Z-master-14ecc4a25a83
FW_BUILD_TIME_ISO: 2026-07-01T02:29:56Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            BD694320718E990DC286B28CCD39B4A237094879117B79FC244FFE3DB2EC568C
bin_release/RC_FlightController.ino.merged.bin     2351241C5BC8836A72CE5F85CC956FEC1C63C415C45351640D600574CD82080C
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            5C68CFFF289E64085FAE18E1F021EC03D78193E9C1542A65B73178E5A5E57254
bin_debug/RC_FlightController.ino.merged.bin     314FABB661EBBF7BEBD4B483C0CC609081D7CA6964BBE4241FDB86868380E157
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
