#!/usr/bin/env python3
"""Normalize cloned 0603 child rotations and close the remaining GND edge."""
import os
import re
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
text = open(path).read()

def footprint_block(source, ref):
    hit = source.index(f'(property "Reference" "{ref}"')
    start = source.rfind("\n\t(footprint", 0, hit) + 1
    depth = 0
    for pos in range(start, len(source)):
        if source[pos] == "(":
            depth += 1
        elif source[pos] == ")":
            depth -= 1
            if depth == 0:
                return start, pos + 1, source[start:pos + 1]
    raise RuntimeError(ref)

for ref, angle in (("R24", "0"), ("R25", "-90"), ("R26", "180")):
    start, end, block = footprint_block(text, ref)
    # Leave the first/global at-expression unchanged; normalize every child
    # property, text, and pad angle to the footprint's placed rotation.
    first = True
    def repl(match):
        nonlocal_first = getattr(repl, "seen", False)
        if not nonlocal_first:
            repl.seen = True
            return match.group(0)
        return f'(at {match.group(1)} {match.group(2)} {angle})'
    repl.seen = False
    block = re.sub(
        r'\(at\s+([-0-9.]+)\s+([-0-9.]+)(?:\s+[-0-9.]+)?\)',
        repl,
        block,
    )
    text = text[:start] + block + text[end:]

open(path, "w").write(text)

# Add an explicit, short ground tie between SW3 and R26.  Both endpoints are
# exposed front pads; this needs no via and removes dependence on a narrow zone
# neck that KiCad correctly reported as isolated.
board = pcbnew.LoadBoard(path)
item = pcbnew.PCB_TRACK(board)
item.SetNet(board.FindNet("GND"))
item.SetLayer(pcbnew.F_Cu)
item.SetWidth(pcbnew.FromMM(0.25))
item.SetStart(pcbnew.VECTOR2I_MM(48.625, 34.0))
item.SetEnd(pcbnew.VECTOR2I_MM(50.975, 31.5))
board.Add(item)
pcbnew.SaveBoard(path, board)
print("Normalized R24-R26 and tied SW3 ground without a via")
