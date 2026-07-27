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
FW_VERSION: V5.5.0
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 7a09772ea1cc
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260727T021339Z-master-7a09772ea1cc
FW_BUILD_TIME_ISO: 2026-07-27T02:13:39Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            61CCE252BB3ABCCD901F8C77C898B1373236B8715E9735FF0484E15CB28B4936
bin_release/RC_FlightController.ino.merged.bin     70F1BAB26E496980C3AE3A91D99292EB70F0F476A7E2E5E556DCD815A1CAFCBE
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            85778933D97788E2BE1D278C940B100A2FC80052B0FF68B10B30CEC93B2C636E
bin_debug/RC_FlightController.ino.merged.bin     C524B600DDF9F8934AF1876F1183F81AE6D02162CC8748184D277A5605F82354
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
