#!/usr/bin/env python3
"""Close the final MPU9250 +3V3 branch without using the L2 ground plane."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.environ.get(
    "FC_BOARD_PATH", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
)
b = pcbnew.LoadBoard(PATH)
n = b.FindNet("+3V3")

def p(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))

def track(layer, a, z, width=0.15):
    t = pcbnew.PCB_TRACK(b)
    t.SetStart(p(*a)); t.SetEnd(p(*z)); t.SetLayer(layer)
    t.SetWidth(pcbnew.FromMM(width)); t.SetNet(n); b.Add(t)

# Stacked, filled/capped microvias connect pad 8 to L3 without consuming a
# surface escape channel.  L2 sees only the via antipad, not a signal trace.
for top, bottom in ((pcbnew.F_Cu, pcbnew.In1_Cu),
                    (pcbnew.In1_Cu, pcbnew.In2_Cu)):
    v = pcbnew.PCB_VIA(b)
    v.SetPosition(p(29.4, 31.5))
    v.SetWidth(pcbnew.FromMM(0.30)); v.SetDrill(pcbnew.FromMM(0.10))
    v.SetViaType(pcbnew.VIATYPE_MICROVIA)
    v.SetLayerPair(top, bottom); v.SetNet(n); b.Add(v)
track(pcbnew.In2_Cu, (29.4, 31.5), (31.5673, 29.9713))

pcbnew.SaveBoard(PATH, b)
