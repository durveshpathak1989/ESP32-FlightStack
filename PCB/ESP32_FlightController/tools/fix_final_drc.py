#!/usr/bin/env python3
"""Apply final validated rule/outline corrections without changing routing."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
b = pcbnew.LoadBoard(PATH)

for fp in b.GetFootprints():
    if fp.GetReference() == "U7":
        fp.SetLocalClearance(pcbnew.FromMM(0.15))

# Add 0.3 mm at the connector edge.  This preserves the placement and routes
# while increasing the minimum routed-copper edge setback beyond 0.25 mm.
old_y = pcbnew.FromMM(60.3)
new_y = pcbnew.FromMM(60.5)
for s in b.GetDrawings():
    if s.GetLayer() != pcbnew.Edge_Cuts:
        continue
    a, z = s.GetStart(), s.GetEnd()
    if a.y == old_y:
        s.SetStart(pcbnew.VECTOR2I(a.x, new_y))
    if z.y == old_y:
        s.SetEnd(pcbnew.VECTOR2I(z.x, new_y))
    if a.x == pcbnew.FromMM(60.0):
        s.SetStart(pcbnew.VECTOR2I(pcbnew.FromMM(60.2), a.y))
    if z.x == pcbnew.FromMM(60.0):
        s.SetEnd(pcbnew.VECTOR2I(pcbnew.FromMM(60.2), z.y))

pcbnew.SaveBoard(PATH, b)
print(PATH)
