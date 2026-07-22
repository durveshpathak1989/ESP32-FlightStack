# Qualification status and release limits

This design is a **production-intent engineering prototype**, not a validated
production flight article.  DRC/ERC/netlist correctness proves the CAD data is
self-consistent; it cannot prove component authenticity, regulator stability,
sensor performance in the airframe, or safe flight behavior.

The release CAD checks are clean: PCB DRC 0, unconnected items 0, schematic
parity 0, schematic ERC 0, and the independent netlist audit passes 143 pin
assertions. These results do not replace first-article qualification.

## Release blockers before volume manufacture or flight

1. **MPU-9250 lifecycle:** TDK announced end-of-life for MPU-9250/MPU-9255 in
   2018.  Do not buy relabelled marketplace stock.  The first build must use
   traceable, genuine customer-supplied inventory and every assembled unit must
   pass WHO_AM_I, self-test, noise, bias and temperature screening.  A future
   board revision should migrate to a currently supported IMU.
2. **BLS-60A current output:** the marketplace listing establishes the seven-pin
   order but does not publish the CUR transfer function or maximum voltage.
   R21/R24 divide CUR by 1.5, so 5 V nominal becomes 3.33 V at the ESP32 ADC.
   Measure the actual ESC output and transients with a current-limited bench
   setup; the protected ADC node must remain inside the ESP32 absolute limits.
3. **iBUS signal level:** R25/R26 divide the receiver signal by 1.5, allowing a
   nominal 5 V iBUS output to reach 3.33 V at the ESP32. A 3.3 V source becomes
   only 2.2 V, which may not satisfy the ESP32 high-level guarantee under all
   conditions. Receiver-specific VIH/noise-margin testing is still required;
   the interface is not claimed as universal 3.3/5 V level translation.
4. **Magnetometer environment:** placing the MPU-9250 at the board center keeps
   it geometrically symmetric and the onboard switcher/current paths remain
   outside the quiet radius.  It does not cancel the magnetic field from a
   stacked 4-in-1 ESC, battery leads, motors or steel fasteners.  Characterize
   heading error across motor current.  Prefer an external Qwiic magnetometer
   mounted away from propulsion wiring for navigation-grade heading.
5. **Power and USB coexistence:** TPS2113A implements source isolation and the
   499-ohm ILIM resistor sets approximately 1 A.  Still perform the documented
   USB-only, battery-only, simultaneous-source, hot-plug, brownout, leakage and
   thermal tests on assembled hardware.
6. **Mechanical fit:** verify the physical ESC, supplied seven-wire harness,
   frame stack, spacers and USB cable against a 1:1 print/first article.  The
   mounting pattern is 30.5 mm square with four 4.0 mm NPTH holes.

No propeller or flight testing is permitted until these items and the complete
bring-up checklist have objective recorded results.
