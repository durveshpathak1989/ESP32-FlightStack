#!/usr/bin/env python3
"""Move external-input protection clear of front-side routing and courtyards."""
import os
import sys
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
fps = {f.GetReference(): f for f in board.GetFootprints()}

def mmpt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

def net(name):
    found = board.FindNet(name)
    if not found:
        raise RuntimeError(f"missing net {name}")
    return found

def add_track(net_name, start, end, layer, width=0.20):
    t = pcbnew.PCB_TRACK(board)
    t.SetNet(net(net_name))
    t.SetLayer(layer)
    t.SetWidth(pcbnew.FromMM(width))
    t.SetStart(pcbnew.VECTOR2I_MM(*start))
    t.SetEnd(pcbnew.VECTOR2I_MM(*end))
    board.Add(t)

# Remove only the short routes created by add_input_protection_board.py.
new_segments = {
    ((12.5, 23.275), (13.475, 23.275)),
    ((54.35, 41.25), (54.125, 41.25)),
    ((52.475, 41.25), (52.3193, 39.4317)),
    ((52.475, 41.25), (51.0, 41.25)),
}
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    ends = tuple(sorted((mmpt(item.GetStart()), mmpt(item.GetEnd()))))
    if any(ends == tuple(sorted(pair)) for pair in new_segments):
        board.Remove(item)

# Move the analog divider shunt beyond the C19 courtyard.
r24 = fps["R24"]
r24.SetPosition(pcbnew.VECTOR2I_MM(14.9, 23.275))
add_track("/ESC_CURRENT_ADC", (12.5, 23.275), (14.075, 23.275), pcbnew.F_Cu)

# Put the compact iBUS divider on the back.  This avoids the front-side
# AUTO_RTS escape without adding long front-side detours.  A single via at the
# existing IBUS_RX endpoint is the only added via and is clear of component
# bodies/courtyards.
r25 = fps["R25"]
r25.Flip(r25.GetPosition(), False)
r25.SetOrientationDegrees(0)
r26 = fps["R26"]
r26.Flip(r26.GetPosition(), False)

add_track("/IBUS_RAW", (54.35, 41.25), (54.125, 41.25), pcbnew.B_Cu)
add_track("/IBUS_RX", (52.475, 41.25), (52.3193, 39.4317), pcbnew.B_Cu)
add_track("/IBUS_RX", (52.475, 41.25), (51.0, 41.25), pcbnew.B_Cu)

v = pcbnew.PCB_VIA(board)
v.SetPosition(pcbnew.VECTOR2I_MM(52.3193, 39.4317))
v.SetNet(net("/IBUS_RX"))
v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
v.SetWidth(pcbnew.FromMM(0.60))
v.SetDrill(pcbnew.FromMM(0.30))
board.Add(v)

pcbnew.SaveBoard(path, board)
print("Moved R24 and routed R25/R26 on B.Cu with one clear transition via")
