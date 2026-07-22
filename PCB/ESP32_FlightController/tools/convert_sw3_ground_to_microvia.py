#!/usr/bin/env python3
"""Connect SW3 directly to L2 GND with an outer-layer laser microvia."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
for item in board.GetTracks():
    if (isinstance(item, pcbnew.PCB_VIA) and
            item.GetNetname() == "GND" and
            round(pcbnew.ToMM(item.GetPosition().x), 4) == 47.5 and
            round(pcbnew.ToMM(item.GetPosition().y), 4) == 35.4):
        item.SetViaType(pcbnew.VIATYPE_MICROVIA)
        item.SetLayerPair(pcbnew.F_Cu, pcbnew.In1_Cu)
        item.SetWidth(pcbnew.FromMM(0.30))
        item.SetDrill(pcbnew.FromMM(0.10))
        break
else:
    raise RuntimeError("SW3 ground via not found")
pcbnew.SaveBoard(path, board)
print("Converted SW3 ground transition to F.Cu-L2 microvia")
