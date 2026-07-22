#!/usr/bin/env python3
"""Replace the impossible L2-L3 laser microvia stack with one through via."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
target = pcbnew.VECTOR2I_MM(33.0, 31.9)
removed = 0

for item in list(board.GetTracks()):
    if not isinstance(item, pcbnew.PCB_VIA):
        continue
    if item.GetPosition() != target:
        continue
    if item.GetNetname() != "+3V3":
        raise RuntimeError("unexpected net at replacement site")
    board.Remove(item)
    removed += 1

if removed != 2:
    raise RuntimeError(f"expected two stacked microvias, found {removed}")

via = pcbnew.PCB_VIA(board)
via.SetPosition(target)
via.SetViaType(pcbnew.VIATYPE_THROUGH)
via.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
via.SetWidth(pcbnew.FromMM(0.50))
via.SetDrill(pcbnew.FromMM(0.20))
via.SetNet(board.FindNet("+3V3"))
board.Add(via)

pcbnew.SaveBoard(path, board)
print("Replaced two stacked +3V3 microvias with one 0.50/0.20 mm through via")
