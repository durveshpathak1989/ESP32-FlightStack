# RC Flight Controller Tuning Reorganization Notes

## What changed

This version keeps the existing control logic and behavior but collects the important tuning values near the top of the sketch in a new **TUNING DASHBOARD** section.

The following values were moved or mirrored into named tuning constants:

- Loop timing and filter values
- Maximum ANGLE mode angle
- Maximum ACRO/rate command
- PID output authority limits
- Throttle expo and throttle slew-rate limits
- Motor idle and motor maximum output
- Throttle cut and idle ramp end
- Boot-time PID gains

## What did not intentionally change

No functional tuning values were intentionally changed. The constants are the same values that were already in the uploaded sketch:

- `TUNE_MAX_ANGLE_DEG = 12.0f`
- `TUNE_MAX_RATE_DPS = 120.0f`
- `TUNE_ROLL_OUTPUT_LIMIT = 0.050f`
- `TUNE_PITCH_OUTPUT_LIMIT = 0.050f`
- `TUNE_YAW_OUTPUT_LIMIT = 0.020f`
- `TUNE_THROTTLE_EXPO = 0.35f`
- `TUNE_THROTTLE_UP_RATE_PER_SEC = 0.50f`
- `TUNE_THROTTLE_DOWN_RATE_PER_SEC = 1.00f`
- `TUNE_MOTOR_IDLE = 0.08f`
- `TUNE_MOTOR_MAX = 0.75f`
- Current relaxed PID gains are preserved.

## Where to tune now

Edit the `TUNING DASHBOARD` section near the top of the file first.

Suggested tuning order:

1. Verify motor direction and mixer correction with props removed.
2. Tune throttle behavior: `TUNE_THROTTLE_UP_RATE_PER_SEC`, `TUNE_THROTTLE_EXPO`, `TUNE_MOTOR_MAX`.
3. Tune ANGLE mode command authority: `TUNE_MAX_ANGLE_DEG`.
4. Tune rate PID gains: `TUNE_RATE_ROLL_*`, `TUNE_RATE_PITCH_*`.
5. Tune outer angle gains: `TUNE_ANGLE_ROLL_*`, `TUNE_ANGLE_PITCH_*`.
6. Adjust correction limits only after confirming PID outputs are hitting those limits.

## Important

This file is still named `.cpp` for download convenience. In Arduino, you can paste it back into your `.ino` or rename it to your original sketch name.
