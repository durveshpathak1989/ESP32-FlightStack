#!/usr/bin/env python3
"""Seed wide power routes from the last DRC-clean topology before autorouting."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEST = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
SOURCE = os.path.join(ROOT, "ESP32_FlightController.clean-before-power-widths.kicad_pcb")
dst = pcbnew.LoadBoard(DEST)
src = pcbnew.LoadBoard(SOURCE)

# The former diode/bootstrap net was corrected in the schematic and is now PH.
mapping = {
    "/SW_NODE": ("/SW_NODE", 0.50),
    "Net-(D1-K)": ("/SW_NODE", 0.50),
}

for old in src.GetTracks():
    spec = mapping.get(old.GetNetname())
    if not spec:
        continue
    new_name, width = spec
    q = dst.FindNet(new_name)
    if isinstance(old, pcbnew.PCB_VIA):
        item = pcbnew.PCB_VIA(dst)
        item.SetPosition(old.GetPosition())
        item.SetWidth(old.GetWidth()); item.SetDrill(old.GetDrill())
        item.SetLayerPair(old.TopLayer(), old.BottomLayer())
    else:
        item = pcbnew.PCB_TRACK(dst)
        item.SetStart(old.GetStart()); item.SetEnd(old.GetEnd())
        item.SetLayer(old.GetLayer()); item.SetWidth(pcbnew.FromMM(width))
    item.SetNet(q); dst.Add(item)

def p(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))

def route(name, layer, points, width):
    q = dst.FindNet(name)
    points = [p(*xy) for xy in points]
    for a, z in zip(points, points[1:]):
        t = pcbnew.PCB_TRACK(dst)
        t.SetStart(a); t.SetEnd(z); t.SetLayer(layer)
        t.SetWidth(pcbnew.FromMM(width)); t.SetNet(q); dst.Add(t)

def via(name, xy, diameter=0.8, drill=0.4):
    v = pcbnew.PCB_VIA(dst)
    v.SetPosition(p(*xy)); v.SetWidth(pcbnew.FromMM(diameter))
    v.SetDrill(pcbnew.FromMM(drill)); v.SetNet(dst.FindNet(name))
    v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu); dst.Add(v)

# Main 3S current path is a dedicated 0.8 mm rear-layer backbone.  The second
# battery header, ADC divider, and EN switch are low-current branches routed later.
route("/VBAT_RAW", pcbnew.F_Cu, [(1.85,38.625),(3.5,38.625)], 0.8)
via("/VBAT_RAW", (3.5,38.625))
via("/VBAT_RAW", (9.0,33.975))
route("/VBAT_RAW", pcbnew.F_Cu,
      [(14.7,37.365),(15.5,37.365),(16.0,38.0)], 0.5)
via("/VBAT_RAW", (16.0,38.0))
route("/VBAT_RAW", pcbnew.B_Cu,
      [(3.5,38.625),(5.0,36.5),(9.0,33.975),(16.0,33.0),(16.0,38.0)], 0.8)

pcbnew.SaveBoard(DEST, dst)
print(DEST)
