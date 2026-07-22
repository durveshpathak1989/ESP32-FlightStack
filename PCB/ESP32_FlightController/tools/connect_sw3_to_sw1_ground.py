#!/usr/bin/env python3
"""Connect SW3 to the existing SW1 ground pad without a new via."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

old_points = {(48.625, 35.4), (47.5, 35.4)}
for item in list(board.GetTracks()):
    remove = False
    if isinstance(item, pcbnew.PCB_VIA):
        remove = item.GetNetname() == "GND" and pt(item.GetPosition()) == (47.5, 35.4)
    elif item.GetNetname() == "GND":
        ends = {pt(item.GetStart()), pt(item.GetEnd())}
        remove = bool(ends & old_points)
    if remove:
        board.Remove(item)

t = pcbnew.PCB_TRACK(board)
t.SetNet(board.FindNet("GND"))
t.SetLayer(pcbnew.F_Cu)
t.SetWidth(pcbnew.FromMM(0.25))
t.SetStart(pcbnew.VECTOR2I_MM(48.625, 34.0))
t.SetEnd(pcbnew.VECTOR2I_MM(46.375, 31.0))
board.Add(t)
pcbnew.SaveBoard(path, board)
print("Connected SW3 ground directly to SW1 ground with no added via")
