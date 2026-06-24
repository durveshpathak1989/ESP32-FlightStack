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

```text
Program storage used: 1,095,887 bytes out of 1,966,080 bytes (55%)
Global variables used: 98,036 bytes out of 327,680 bytes (29%)
Free RAM for local variables: 229,644 bytes
```

Quiet build check with `VERBOSE_ON=0`:

```text
Program storage used: 1,084,583 bytes out of 1,966,080 bytes (55%)
Global variables used: 98,028 bytes out of 327,680 bytes (29%)
Free RAM for local variables: 229,652 bytes
```

## Build Command

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:UploadSpeed=921600,CPUFreq=240,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default,ZigbeeMode=default --output-dir build .
```
