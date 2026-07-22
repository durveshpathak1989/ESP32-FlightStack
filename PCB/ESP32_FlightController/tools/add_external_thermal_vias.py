#!/usr/bin/env python3
"""Add the minimum external thermal/ground vias beside the TPS5430 package."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
net = board.FindNet("GND")

locations = ((15.9, 38.2), (15.9, 38.8))
existing = {
    (round(pcbnew.ToMM(item.GetPosition().x), 4),
     round(pcbnew.ToMM(item.GetPosition().y), 4))
    for item in board.GetTracks()
    if isinstance(item, pcbnew.PCB_VIA)
}
added = 0
for x, y in locations:
    if (x, y) in existing:
        continue
    via = pcbnew.PCB_VIA(board)
    via.SetPosition(pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y)))
    via.SetWidth(pcbnew.FromMM(0.5))
    via.SetDrill(pcbnew.FromMM(0.2))
    via.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
    via.SetNet(net)
    board.Add(via)
    added += 1

pcbnew.SaveBoard(path, board)
print(f"Added {added} external TPS5430 thermal/ground vias")
