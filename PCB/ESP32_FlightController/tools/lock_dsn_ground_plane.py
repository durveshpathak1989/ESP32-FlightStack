#!/usr/bin/env python3
"""Mark L2 as plane-only in a KiCad-exported Specctra DSN.

Freerouting otherwise treats every copper layer as a signal layer and can cut
the intended ground reference with traces. The output is rejected unless the
named layer exists exactly once.
"""
import pathlib
import re
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: lock_dsn_ground_plane.py BOARD.dsn")

path = pathlib.Path(sys.argv[1])
text = path.read_text()
source = "(layer GND_PLANE\n      (type signal)"
target = "(layer GND_PLANE\n      (type power)"
if text.count(source) != 1:
    raise SystemExit("expected exactly one signal-type GND_PLANE layer")
text = text.replace(source, target, 1)

# Give the autorouter the actual electrical widths instead of widening an
# already-packed 0.20 mm route afterward.  Remove these nets from KiCad's
# default class and add explicit Specctra classes.
classes = {
    "fc_vbat": (1000, ["/VBAT_RAW"]),
    "fc_switch": (800, ["/SW_NODE"]),
    "fc_5v_power": (600, ["/+5V_BUCK", "+5V"]),
    "fc_usb_power": (500, ["/USB_VBUS", "/USB_VBUS_RAW"]),
    "fc_3v3_power": (400, ["+3V3"]),
}
start = text.index("    (class kicad_default ")
circuit = text.index("      (circuit", start)
header = text[start:circuit]
for _, nets in classes.values():
    for net in nets:
        header, count = re.subn(rf"(?<!\S){re.escape(net)}(?!\S)", "", header)
        if count != 1:
            raise SystemExit(f"expected {net} exactly once in kicad_default")
text = text[:start] + header + text[circuit:]

blocks = []
for name, (width, nets) in classes.items():
    blocks.append(
        f"    (class {name} {' '.join(nets)}\n"
        "      (circuit\n"
        "        (use_via \"Via[0-3]_600:300_um\")\n"
        "      )\n"
        "      (rule\n"
        f"        (width {width})\n"
        "        (clearance 150)\n"
        "      )\n"
        "    )\n"
    )
text = text[:start] + "".join(blocks) + text[start:]
path.write_text(text)
print(f"locked GND_PLANE in {path}")
