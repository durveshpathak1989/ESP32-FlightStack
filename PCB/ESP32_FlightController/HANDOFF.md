# Flight-controller PCB handoff

Last updated: 2026-07-22

## Resume here

The JLCPCB **parts-selection assistance** request was submitted and the USD 10
service checkout/address step was completed. The next action is to open:

`JLCPCB User Center > Orders > PCBA Parts Selection`

Wait for JLCPCB to return its completed component selections and quotation.
Before approving assembly or paying for the PCB/PCBA order, compare every
proposed manufacturer part number against the schematic value, footprint,
pinout, rating, and lifecycle. Pay particular attention to U2 (MPU-9250), the
ESP32 module, power-path/buck components, USB parts, L1, switches, and all
connectors. Do not accept substitutions without engineering review.

The submitted assistance package used:

- `manufacturing/final-2026-07-22-r3/assembly/BOM-jlcpcb.csv`
- `manufacturing/final-2026-07-22-r3/assembly/positions-jlcpcb.csv`
- `manufacturing/controlled-parts-r3.zip`
- Gerbers: `manufacturing/ESP32_FlightController-JLCPCB-Gerber-r3.zip`

The quote was configured as a four-layer, 1.6 mm ENIG board with 1 oz copper,
both-side standard PCBA, quantity 2, plugged vias, JLC041611-7628 stackup, and
placement confirmation. Reconfirm these choices when JLCPCB creates the final
assembly quotation.

## CAD release state

- PCB DRC: 0 violations; 0 unconnected items
- Schematic ERC: 0 errors and 0 warnings
- Schematic/PCB parity: 69 parts and 265 pins passed
- Independent critical-net audit: 143 assertions passed
- No via-in-pad; MPU-9250 courtyard is via-free
- Release archive: `manufacturing/ESP32_FlightController-production-intent-2026-07-22-r3.zip`
- Release SHA-256: `6c5d8e9233e1674ac935716f6dc0d0e65e27e762c65e9077957261418ff05fc1`

This is a **production-intent prototype**, not a flight-qualified production
article. Read
`manufacturing/final-2026-07-22-r3/QUALIFICATION_STATUS.md` and complete the
first-article tests before connecting propellers or attempting flight.

