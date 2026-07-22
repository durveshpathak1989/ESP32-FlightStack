#!/usr/bin/env python3
"""Ground SW3 with one off-courtyard via instead of crossing signal tracks."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

old = {
    frozenset(((48.625, 34.0), (50.7, 34.0))),
    frozenset(((50.7, 34.0), (50.975, 33.725))),
    frozenset(((50.975, 33.725), (50.975, 31.5))),
}
for item in list(board.GetTracks()):
    if (not isinstance(item, pcbnew.PCB_VIA) and
            item.GetNetname() == "GND" and
            frozenset((pt(item.GetStart()), pt(item.GetEnd()))) in old):
        board.Remove(item)

gnd = board.FindNet("GND")
t = pcbnew.PCB_TRACK(board)
t.SetNet(gnd)
t.SetLayer(pcbnew.F_Cu)
t.SetWidth(pcbnew.FromMM(0.25))
t.SetStart(pcbnew.VECTOR2I_MM(48.625, 34.0))
t.SetEnd(pcbnew.VECTOR2I_MM(49.4, 34.0))
board.Add(t)

v = pcbnew.PCB_VIA(board)
v.SetPosition(pcbnew.VECTOR2I_MM(49.4, 34.0))
v.SetNet(gnd)
v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
v.SetWidth(pcbnew.FromMM(0.60))
v.SetDrill(pcbnew.FromMM(0.30))
board.Add(v)

pcbnew.SaveBoard(path, board)
print("Grounded SW3 with one off-courtyard through-via")
