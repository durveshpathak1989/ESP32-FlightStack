#!/usr/bin/env python3
"""Route both external-input dividers on F.Cu without added vias."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
fps = {f.GetReference(): f for f in board.GetFootprints()}

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

def add_track(net_name, a, b, width=0.20):
    item = pcbnew.PCB_TRACK(board)
    item.SetNet(board.FindNet(net_name))
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(width))
    item.SetStart(pcbnew.VECTOR2I_MM(*a))
    item.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(item)

# Remove the crossing R24 dogleg plus the temporary back-side iBUS routes and
# transition via in one collection pass (the SWIG track iterator is invalidated
# after a removal).
remove_specs = (
    ((15.725, 23.275), (15.725, 21.725), "GND"),
    ((15.725, 21.725), (12.5, 21.725), "GND"),
    ((54.35, 41.25), (54.125, 41.25), "/IBUS_RAW"),
    ((52.475, 41.25), (52.3193, 39.4317), "/IBUS_RX"),
    ((52.475, 41.25), (51.0, 41.25), "/IBUS_RX"),
)
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        matched = (pt(item.GetPosition()) == (52.3193, 39.4317) and
                   item.GetNetname() == "/IBUS_RX")
    else:
        matched = any(
            item.GetNetname() == name and
            {pt(item.GetStart()), pt(item.GetEnd())} == {a, b}
            for a, b, name in remove_specs
        )
    if matched:
        board.Remove(item)
add_track("GND", (15.725, 23.275), (15.647, 21.408), 0.25)

# Put the divider in the clear front-side gap between J4 and J6.  The raw
# signal stays along the connector edge, while the protected node joins the
# existing iBUS route at its current endpoint.
r25 = fps["R25"]
if r25.GetLayer() == pcbnew.B_Cu:
    r25.Flip(r25.GetPosition(), False)
r25.SetPosition(pcbnew.VECTOR2I_MM(51.2, 34.0))
r25.SetOrientationDegrees(0)

r26 = fps["R26"]
if r26.GetLayer() == pcbnew.B_Cu:
    r26.Flip(r26.GetPosition(), False)
r26.SetPosition(pcbnew.VECTOR2I_MM(48.0, 33.0))
r26.SetOrientationDegrees(180)

add_track("/IBUS_RAW", (54.35, 41.25), (55.0, 41.25))
add_track("/IBUS_RAW", (55.0, 41.25), (55.0, 35.2))
add_track("/IBUS_RAW", (55.0, 35.2), (53.0, 35.2))
add_track("/IBUS_RAW", (53.0, 35.2), (52.025, 34.0))
add_track("/IBUS_RX", (50.375, 34.0), (52.3193, 34.0737))
add_track("/IBUS_RX", (50.375, 34.0), (48.825, 33.0))

pcbnew.SaveBoard(path, board)
print("Rerouted input protection on F.Cu with no added vias")
