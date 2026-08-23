# Local build pipeline - mirrors .github/workflows/esp32-arduino-ci.yml exactly.
# Usage: powershell -File scripts\build_local.ps1   (from repo root)
# Requires: arduino-cli on PATH (or set $env:ARDUINO_CLI), esp32 core 3.3.8 installed.

$ErrorActionPreference = 'Stop'
$cli = if ($env:ARDUINO_CLI) { $env:ARDUINO_CLI } else { "arduino-cli" }
$core = "esp32:esp32@3.3.8"
$fqbn = "esp32:esp32:esp32:UploadSpeed=921600,CPUFreq=240,FlashFreq=80,FlashMode=qio,FlashSize=4M,PartitionScheme=min_spiffs,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default,ZigbeeMode=default"

& $cli core update-index
$installed = & $cli core list | Select-String "esp32:esp32"
if (-not $installed) { & $cli core install $core }

Write-Host "`n=== Compiling RC_FlightController for ESP32 Dev Module ===" -ForegroundColor Cyan
& $cli compile --fqbn $fqbn RC_FlightController
if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBUILD OK" -ForegroundColor Green
} else {
    Write-Host "`nBUILD FAILED" -ForegroundColor Red
    exit 1
}
