#!/usr/bin/env python3
"""Place and route the schematic-approved external-input dividers."""
import os
import sys
import re
import uuid
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
source = sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, "ESP32_FlightController.kicad_pcb")
output = sys.argv[2] if len(sys.argv) > 2 else source
raw = open(source).read()
if any(f'(property "Reference" "{ref}"' in raw for ref in ("R24", "R25", "R26")):
    raise SystemExit("input-protection footprints already present")

def footprint_block(source_text, ref):
    hit = source_text.index(f'(property "Reference" "{ref}"')
    start = source_text.rfind("\n\t(footprint", 0, hit) + 1
    depth = 0
    for pos in range(start, len(source_text)):
        if source_text[pos] == "(":
            depth += 1
        elif source_text[pos] == ")":
            depth -= 1
            if depth == 0:
                return source_text[start:pos + 1]
    raise RuntimeError(f"unterminated footprint {ref}")

def clone_resistor(template, old_ref, ref, value, x, y, angle):
    block = template.replace(f'"{old_ref}"', f'"{ref}"')
    old_value = re.search(r'\(property "Value" "([^"]+)"', block).group(1)
    block = block.replace(f'(property "Value" "{old_value}"',
                          f'(property "Value" "{value}"', 1)
    # The first at-expression is the footprint's global placement.  All later
    # coordinates are local land-pattern geometry and must remain unchanged.
    block = re.sub(r'\(at [-0-9.]+ [-0-9.]+(?: [-0-9.]+)?\)',
                   f'(at {x:g} {y:g} {angle:g})', block, count=1)
    block = re.sub(r'\(uuid "[^"]+"\)',
                   lambda _: f'(uuid "{uuid.uuid4()}")', block)
    return block

template = footprint_block(raw, "R21")
clones = "\n".join([
    clone_resistor(template, "R21", "R24", "20k 1%", 14.30, 23.275, 0),
    clone_resistor(template, "R21", "R25", "10k 1%", 53.30, 41.25, 0),
    clone_resistor(template, "R21", "R26", "20k 1%", 51.00, 42.075, 270),
])
insert_at = raw.index("\n\t(gr_")
raw = raw[:insert_at] + "\n" + clones + raw[insert_at:]
with open(output, "w") as handle:
    handle.write(raw)

board = pcbnew.LoadBoard(output)
fps = {f.GetReference(): f for f in board.GetFootprints()}

def net(name):
    found = board.FindNet(name)
    if found:
        return found
    item = pcbnew.NETINFO_ITEM(board, name)
    board.Add(item)
    return item

def assign_resistor(ref, pin1, pin2):
    fp = fps[ref]
    for pad in fp.Pads():
        pad.SetNet(net(pin1 if pad.GetNumber() == "1" else pin2))
    return fp

def add_track(net_name, start, end, width=0.20):
    t = pcbnew.PCB_TRACK(board)
    t.SetNet(net(net_name))
    t.SetLayer(pcbnew.F_Cu)
    t.SetWidth(pcbnew.FromMM(width))
    t.SetStart(pcbnew.VECTOR2I_MM(*start))
    t.SetEnd(pcbnew.VECTOR2I_MM(*end))
    board.Add(t)
    return t

# ESC current divider.  R21 is already the series element; R24 is a shunt next
# to C19 and reaches the existing ADC node with one short, via-free branch.
fps["R21"].SetValue("10k 1%")
r24 = assign_resistor("R24", "/ESC_CURRENT_ADC", "GND")
add_track("/ESC_CURRENT_ADC", (12.50, 23.275), (13.475, 23.275))

# iBUS divider fits directly in the connector escape.  Pin 3 and its short
# stub become IBUS_RAW; the original long route remains on protected IBUS_RX.
ibus_raw = net("/IBUS_RAW")
j6p3 = next(p for p in fps["J6"].Pads() if p.GetNumber() == "3")
j6p3.SetNet(ibus_raw)

def mmpt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

old_stub = None
old_diag = None
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA) or item.GetNetname() != "/IBUS_RX":
        continue
    endpoints = {mmpt(item.GetStart()), mmpt(item.GetEnd())}
    if endpoints == {(54.35, 41.25), (54.1376, 41.25)}:
        old_stub = item
    elif endpoints == {(54.1376, 41.25), (52.3193, 39.4317)}:
        old_diag = item
if old_stub is None or old_diag is None:
    raise RuntimeError("expected original J6 iBUS escape tracks were not found")
board.Remove(old_stub)
board.Remove(old_diag)

r25 = assign_resistor("R25", "/IBUS_RX", "/IBUS_RAW")
r26 = assign_resistor("R26", "/IBUS_RX", "GND")
add_track("/IBUS_RAW", (54.35, 41.25), (54.125, 41.25))
add_track("/IBUS_RX", (52.475, 41.25), (52.3193, 39.4317))
add_track("/IBUS_RX", (52.475, 41.25), (51.00, 41.25))

pcbnew.SaveBoard(output, board)
print(f"Added R24-R26 and routed protected inputs to {output}")
