# JLCPCB BOM and CPL format

These schemas were accepted by JLCPCB's PCBA importer on 2026-07-22.

## BOM

CSV header, in this exact order:

```csv
Comment,Designator,Footprint,LCSC Part #
```

- `Comment`: component value or controlled part description.
- `Designator`: comma-separated references, for example `C11,C12,C13`.
- `Footprint`: package name without the KiCad library prefix.
- `LCSC Part #`: preferred JLC/LCSC stock number; leave empty when the part must
  be matched or manually approved.
- Expand KiCad reference ranges such as `R10-R13` into
  `R10,R11,R12,R13`.

Validated file:

`final-2026-07-22-r3/assembly/BOM-jlcpcb.csv`

## CPL / pick-and-place

CSV header, in this exact order:

```csv
Designator,Mid X,Mid Y,Layer,Rotation
```

- One component per row.
- `Layer` is `top` or `bottom`.
- Rotation is in degrees.
- Preserve KiCad's exported coordinate origin and signs; do not independently
  mirror bottom-side coordinates or rotations.

Validated file:

`final-2026-07-22-r3/assembly/positions-jlcpcb.csv`

## Import result

The generic KiCad exports `BOM.csv` and `positions.csv` were rejected by the
JLCPCB PCBA parser. The two `*-jlcpcb.csv` files above processed successfully.
Always visually confirm placement and rotation in JLCPCB before ordering,
especially U2, polarized parts, switches, USB, and edge connectors.
