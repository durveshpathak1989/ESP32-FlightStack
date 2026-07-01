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
| Release (`VERBOSE_ON=0`) | 1,127,967 bytes | 1,966,080 bytes | 76,084 bytes | 327,680 bytes | 251,596 bytes | 1,128,112 bytes |
| Debug (`VERBOSE_ON=1`) | 1,138,927 bytes | 1,966,080 bytes | 76,092 bytes | 327,680 bytes | 251,588 bytes | 1,139,072 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.4.0-12-g27f79ba
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 27f79badeefb
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260701T021951Z-master-27f79badeefb
FW_BUILD_TIME_ISO: 2026-07-01T02:19:51Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            A827B0BE1F4E022194A0AE865D955865A3DD7E85AB058E514112EA731DFA479C
bin_release/RC_FlightController.ino.merged.bin     888AD1F138511B7FACD234485872B96B267392DB45F9707E31033D9715D58B90
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            D5A653C4F64AC52C47BA0A178523AA5FDFE5CC65B912732E44992DD4CCB95939
bin_debug/RC_FlightController.ino.merged.bin     A5C8C2592604E2BAD1F375514C8AED07111017EED86144CCFD7FCE2B35C9E9F0
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
