#!/usr/bin/env python3
"""Give the ESC-current divider an explicit short ground return."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
gnd = board.FindNet("GND")

def track(a, b):
    item = pcbnew.PCB_TRACK(board)
    item.SetNet(gnd)
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(0.25))
    item.SetStart(pcbnew.VECTOR2I_MM(*a))
    item.SetEnd(pcbnew.VECTOR2I_MM(*b))
    board.Add(item)

track((15.725, 23.275), (15.725, 21.725))
track((15.725, 21.725), (12.5, 21.725))
pcbnew.SaveBoard(path, board)
print("Connected R24 ground explicitly to C19 ground")
