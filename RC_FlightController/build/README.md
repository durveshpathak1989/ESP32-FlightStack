# RC_FlightController Build Notes

This folder contains the compiled firmware files for the stability branch.

## Main Firmware File

Use this file for normal OTA upload:

```text
RC_FlightController.ino.bin
```

## Generated Files

| File | What it is for |
| --- | --- |
| `RC_FlightController.ino.bin` | Main firmware binary for OTA upload. |
| `RC_FlightController.ino.merged.bin` | Full flash image that includes firmware plus boot/partition data. |
| `RC_FlightController.ino.bootloader.bin` | ESP32 bootloader image. |
| `RC_FlightController.ino.partitions.bin` | ESP32 partition table image. |
| `BUILD_METRICS.txt` | Flash/RAM numbers from the compile. |

## Build Metrics

Normal build:

| Metric | Value |
| --- | ---: |
| Flash used | 1,095,887 bytes |
| Flash maximum | 1,966,080 bytes |
| Flash utilization | 55% |
| RAM used by global variables | 98,036 bytes |
| RAM maximum | 327,680 bytes |
| RAM utilization | 29% |
| Free RAM for local variables | 229,644 bytes |

Quiet build check with `VERBOSE_ON=0`:

| Metric | Value |
| --- | ---: |
| Flash used | 1,084,583 bytes |
| Flash maximum | 1,966,080 bytes |
| Flash utilization | 55% |
| RAM used by global variables | 98,028 bytes |
| RAM maximum | 327,680 bytes |
| RAM utilization | 29% |
| Free RAM for local variables | 229,652 bytes |

## Build Command

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:UploadSpeed=921600,CPUFreq=240,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default,ZigbeeMode=default --output-dir build .
```
