#!/usr/bin/env python3
"""Clear the last iBUS/AUTO_RTS and SW3-ground crossings without vias."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
r25 = board.FindFootprintByReference("R25")

def pt(v):
    return (round(pcbnew.ToMM(v.x), 4), round(pcbnew.ToMM(v.y), 4))

old_points = {(52.18, 34.175), (52.18, 35.825)}
direct_ground = {(48.625, 34.0), (50.975, 31.5)}
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    ends = {pt(item.GetStart()), pt(item.GetEnd())}
    if (bool(ends & old_points) or
            (item.GetNetname() == "GND" and ends == direct_ground)):
        board.Remove(item)

# Move 20 um right for AUTO_RTS clearance and 0.7 mm upward so the courtyard
# remains clear of J6 despite that horizontal shift.
r25.SetPosition(pcbnew.VECTOR2I_MM(52.2, 34.3))
r25.SetOrientationDegrees(-90)

def add(net_name, a, b, width=0.20):
    item = pcbnew.PCB_TRACK(board)
    item.SetNet(board.FindNet(net_name))
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(width))
    item.SetStart(pcbnew.VECTOR2I_MM(*a))
    item.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(item)

add("/IBUS_RX", (52.2, 33.475), (52.3193, 34.0737))
add("/IBUS_RX", (52.2, 33.475), (52.625, 31.5))
add("/IBUS_RAW", (52.2, 35.125), (52.3193, 39.4317))

# Route SW3 ground around, rather than diagonally through, the signal channel.
add("GND", (48.625, 34.0), (50.7, 34.0), 0.25)
add("GND", (50.7, 34.0), (50.975, 33.725), 0.25)
add("GND", (50.975, 33.725), (50.975, 31.5), 0.25)

pcbnew.SaveBoard(path, board)
print("Cleared final input and ground routing conflicts without vias")
