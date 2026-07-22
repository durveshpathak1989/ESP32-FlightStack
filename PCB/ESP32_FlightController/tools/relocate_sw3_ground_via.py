#!/usr/bin/env python3
"""Move the SW3 ground via clear of ESP_EN and the B.Cu 5 V trunk."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

for item in list(board.GetTracks()):
    remove = False
    if isinstance(item, pcbnew.PCB_VIA):
        remove = item.GetNetname() == "GND" and pt(item.GetPosition()) == (49.4, 34.0)
    elif item.GetNetname() == "GND":
        remove = {pt(item.GetStart()), pt(item.GetEnd())} == {
            (48.625, 34.0), (49.4, 34.0)
        }
    if remove:
        board.Remove(item)

gnd = board.FindNet("GND")
def add(a, b):
    t = pcbnew.PCB_TRACK(board)
    t.SetNet(gnd)
    t.SetLayer(pcbnew.F_Cu)
    t.SetWidth(pcbnew.FromMM(0.25))
    t.SetStart(pcbnew.VECTOR2I_MM(*a))
    t.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(t)

add((48.625, 34.0), (48.625, 35.4))
add((48.625, 35.4), (47.5, 35.4))
v = pcbnew.PCB_VIA(board)
v.SetPosition(pcbnew.VECTOR2I_MM(47.5, 35.4))
v.SetNet(gnd)
v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
v.SetWidth(pcbnew.FromMM(0.60))
v.SetDrill(pcbnew.FromMM(0.30))
board.Add(v)

pcbnew.SaveBoard(path, board)
print("Relocated SW3 ground via outside component courtyards and power routes")
