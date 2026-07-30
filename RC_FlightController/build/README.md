# RC_FlightController Build Notes

This folder contains compiled firmware artifacts for ESP32-FlightStack.

Last regenerated: 2026-07-31

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
| Release (`VERBOSE_ON=0`) | 1,138,771 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,138,912 bytes |
| Debug (`VERBOSE_ON=1`) | 1,150,079 bytes | 1,966,080 bytes | 76,260 bytes | 327,680 bytes | 251,420 bytes | 1,150,224 bytes |

## Firmware Identity

```text
FW_NAME: RC_FlightController
FW_VERSION: V5.5.1-1-g7525558
FW_SOURCE_BRANCH: master
FW_SOURCE_COMMIT: 752555854cc7
FW_SOURCE_DIRTY: 0
FW_BUILD_ID: 20260731T035051Z-master-752555854cc7
FW_BUILD_TIME_ISO: 2026-07-31T03:50:51Z
```

## SHA256

Release:

```text
bin_release/RC_FlightController.ino.bin            59B294B434F520E65D26302C58C90593D9D4B68FE8085FA4D57F2592351273D2
bin_release/RC_FlightController.ino.merged.bin     5ED672D685966B309F5E48AD505CB5D07E4799117090196CBFA809585BE03228
bin_release/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_release/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Debug:

```text
bin_debug/RC_FlightController.ino.bin            4C7D871E8816D306A497896E13FC56A6C8EB0466FCDFC77639CA255D61E7F45D
bin_debug/RC_FlightController.ino.merged.bin     81C872ED144331C53F46B5CB77F39C03FC367A9BEC95815BAF128FE59088B350
bin_debug/RC_FlightController.ino.bootloader.bin F508DFE30F34C2490EC08CAAA96F20DC2853F66A0A92F6FB759B205E82924F29
bin_debug/RC_FlightController.ino.partitions.bin 0A8B5720E7B77FF11F1462458C3A509DEE79224E5279898F26D6A2E3AE0517B7
```

Core utilization is measured only while the ESP32 is running. The firmware sends it through telemetry as `cpu_core0_pct`, `cpu_core1_pct`, and `cpu_valid`.
