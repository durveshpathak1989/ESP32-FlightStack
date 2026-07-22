#!/usr/bin/env python3
"""Fit the iBUS divider into the narrow service-connector channel."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
fps = {f.GetReference(): f for f in board.GetFootprints()}

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

# IBUS_RAW is new, so every existing segment belongs to the previous attempted
# route.  Remove only the two protected-node branches added in that attempt.
remove_rx_points = {(50.375, 34.0), (48.825, 33.0)}
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    if item.GetNetname() == "/IBUS_RAW" or (
            item.GetNetname() == "/IBUS_RX" and
            (pt(item.GetStart()) in remove_rx_points or
             pt(item.GetEnd()) in remove_rx_points)):
        board.Remove(item)

r25 = fps["R25"]
r25.SetPosition(pcbnew.VECTOR2I_MM(52.075, 35.0))
r25.SetOrientationDegrees(-90)
r26 = fps["R26"]
r26.SetPosition(pcbnew.VECTOR2I_MM(51.8, 31.5))
r26.SetOrientationDegrees(180)

def add(net_name, a, b):
    item = pcbnew.PCB_TRACK(board)
    item.SetNet(board.FindNet(net_name))
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(0.20))
    item.SetStart(pcbnew.VECTOR2I_MM(*a))
    item.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(item)

add("/IBUS_RAW", (54.35, 41.25), (55.4, 41.25))
add("/IBUS_RAW", (55.4, 41.25), (55.4, 35.6))
add("/IBUS_RAW", (55.4, 35.6), (52.075, 35.825))
add("/IBUS_RX", (52.075, 34.175), (52.3193, 34.0737))
add("/IBUS_RX", (52.075, 34.175), (52.625, 31.5))

pcbnew.SaveBoard(path, board)
print("Adjusted R25/R26 into clear connector-channel courtyards")
