ARMED MOTOR TIMING + TUNING FIX

Symptoms fixed:
1) As soon as ARM was enabled, [TIME] period jumped to ~40000us and phase motor≈39800us.
2) GCS tuning appeared to update one value and then revert / stop updating.
3) Core 0 appeared near 99% with Wi-Fi/GCS active.

Files to replace together:
- RC_FlightController.ino
- MotorControl.h
- MotorControl.cpp
- TelemetryWiFi.h
- TelemetryWiFi.cpp
- DroneGCS_Apple_OTA_upload.html and/or TestScreen_OTA_upload.html

Main motor fix:
- MotorControl now uses explicit LEDC channels, not pin-keyed auto LEDC writes.
- PWM default changed to 400Hz, 12-bit, 1000-2000us pulses.
- Redundant motor writes are skipped if the commanded pulse is unchanged.

If your ESC does not accept 400Hz standard PWM:
- In MotorControl.h change MOTOR_PWM_FREQ_HZ from 400 to 50.
- Keep the explicit-channel implementation.

Expected after fix when armed with throttle low:
[TIME] period≈2500us, ctrl<~500us, phase motor close to 0-50us, not 39800us.

Tuning fix:
- /tune applies immediately while DISARMED, then telemetry reports updated values.
- /tune remains rejected while ARMED.
- Apple GCS now posts to tuneUrl2/configured IP and blurs active inputs before verification.

Core 0 fix:
- Wi-Fi task delay changed from 2ms to 10ms to leave more idle time.
