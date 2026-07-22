#!/usr/bin/env python3
"""Insert R25 inline with the original J6-to-ESP32 iBUS route."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
fps = {f.GetReference(): f for f in board.GetFootprints()}

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

# Remove all temporary raw routes plus the original protected vertical segment
# that R25 now splits into raw and protected halves.
vertical = {(52.3193, 34.0737), (52.3193, 39.4317)}
old_protected_points = {(52.075, 34.175), (52.625, 31.5)}
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    ends = {pt(item.GetStart()), pt(item.GetEnd())}
    if (item.GetNetname() == "/IBUS_RAW" or
            (item.GetNetname() == "/IBUS_RX" and
             (ends == vertical or bool(ends & old_protected_points)))):
        board.Remove(item)

r25 = fps["R25"]
r25.SetPosition(pcbnew.VECTOR2I_MM(52.18, 35.0))
r25.SetOrientationDegrees(-90)

def add(net_name, a, b):
    item = pcbnew.PCB_TRACK(board)
    item.SetNet(board.FindNet(net_name))
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(0.20))
    item.SetStart(pcbnew.VECTOR2I_MM(*a))
    item.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(item)

# Protected side continues toward the ESP32 and branches to the shunt.
add("/IBUS_RX", (52.18, 34.175), (52.3193, 34.0737))
add("/IBUS_RX", (52.18, 34.175), (52.625, 31.5))
# Raw side reuses the original proven J6 escape geometry.
add("/IBUS_RAW", (52.18, 35.825), (52.3193, 39.4317))
add("/IBUS_RAW", (52.3193, 39.4317), (54.1376, 41.25))
add("/IBUS_RAW", (54.1376, 41.25), (54.35, 41.25))

pcbnew.SaveBoard(path, board)
print("Inserted R25 inline with the original iBUS escape")
