# Production and bring-up checklist

Read `QUALIFICATION_STATUS.md` first.  This package is not released for flight
or volume manufacture until every blocker there has objective test evidence.

## Fabrication release

- Use a four-layer HDI 1+N+1 stack: L1 signals/components, L2 continuous GND,
  L3 power/signals, L4 signals/GND.
- Require 0.30 mm pad / 0.10 mm laser microvias from L1 to L2. Every microvia
  is outside component solder lands; CAM must not move drills into pads.
- The released layout contains 133 unique drill sites, including 31 unique
  laser-microvia sites. Two added GND microvias close the split front-pour
  islands around RESET/iBUS without crossing signal layers.
- Confirm 0.15 mm minimum trace/space support. USB is Full-Speed only and uses
  a short L2-referenced route; do not qualify it as USB High-Speed.
- Review the fabrication drawing and drill report before ordering; verify the
  separate `front-in1` and `back-in2` files are accepted as L1-L2 and L4-L3
  laser-microvia operations. Reject any L2-L3 laser-drill file.
- Verify J9 is fitted at 0 degrees with its USB4105 `PCB Edge` datum coincident
  with the nominal lower board edge at Y = 60.5 mm; the receptacle mouth faces
  outward and must not be pulled inward by panel rails or assembly tooling.
- Inspect Gerbers in an independent viewer and confirm all four copper layers,
  two masks, two silkscreens, outline, PTH, NPTH and laser-drill files are present.

## Assembly inspection

- AOI/X-ray the MPU-9250, CP2102N and TPS5430 exposed pads. Confirm there are
  no drilled holes in any of those solder lands.
- Check diode, regulator, electrolytic/tantalum capacitor and USB-C orientation.
- Verify no solder bridges at the MPU-9250 or USB-C connector.
- Measure resistance from VBAT, +5V, +3V3 and USB VBUS to GND before power-up.

## Current-limited bring-up

1. Remove propellers and disconnect all ESC power leads.
2. Apply USB only; verify no voltage appears on the raw 3S connector.
3. Verify CP2102 enumeration, 5 V mux output and 3.3 V rail.
4. Confirm automatic download mode, manual RESET and BOOT behavior.
5. Apply a current-limited 9-12.6 V bench supply with USB disconnected; verify
   buck output, mux output, 3.3 V and switch OFF current.
6. Connect USB and battery simultaneously; verify neither source backfeeds and
   check TPS2113A temperature/source selection.
7. Read MPU-9250 WHO_AM_I, gyro, accelerometer and magnetometer data; verify the
   axes match the PCB orientation markings.
8. Check GPS, iBUS, I2C sensors, JTAG and all four ESC signal outputs.
9. Verify J6 iBUS and J14 CUR remain within 0-3.3 V at the controller pins over
   every operating and fault condition. R25/R26 and R21/R24 are 10 kΩ/20 kΩ
   input dividers (nominal 5 V input becomes 3.33 V); disconnect immediately if
   either protected node can exceed the ESP32 rail.

## Qualification before flight

- Load-test the 5 V and 3.3 V rails across input voltage and temperature; record
  ripple, efficiency, startup and shutdown waveforms.
- Probe the TPS5430 switch node and verify diode/inductor temperatures at worst
  case load.
- Perform conducted/radiated emissions pre-scan with motors and ESCs operating.
- Calibrate the magnetometer in the assembled 450 mm airframe and measure heading
  error while motor current is swept.
- Perform vibration, brownout, USB hot-plug, ESD and failsafe tests.
- Complete restrained low-power motor tests before any propeller or flight test.

The motor power path must remain in the ESC/PDB wiring. This controller carries
only ESC control signals and signal ground; it is not designed to conduct motor
or battery-distribution current.
