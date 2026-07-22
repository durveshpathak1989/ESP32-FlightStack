#!/usr/bin/env python3
"""Remove drilled thermal-pad subpads that sit beneath assembled packages."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)

targets = {"U1": "39", "U3": "9"}
removed = []
to_remove = []
for footprint in board.GetFootprints():
    wanted = targets.get(footprint.GetReference())
    if wanted is None:
        continue
    pads = []
    for pad in footprint.Pads():
        pads.append(pad)
    for pad in pads:
        drill = pad.GetDrillSize()
        if pad.GetNumber() == wanted and (drill.x > 0 or drill.y > 0):
            position = pad.GetPosition()
            removed.append(
                (footprint.GetReference(), pad.GetNumber(),
                 pcbnew.ToMM(position.x), pcbnew.ToMM(position.y))
            )
            to_remove.append((footprint, pad))

for footprint, pad in to_remove:
    footprint.Remove(pad)

for footprint in board.GetFootprints():
    if footprint.GetReference() == "U3":
        footprint.SetFPID(
            pcbnew.LIB_ID(
                "FlightController_Custom", "TI_SO-PowerPAD-8_NoUnderDrills"
            )
        )

pcbnew.SaveBoard(path, board)
print(f"Removed {len(removed)} drilled under-component pads")
for ref, number, x, y in removed:
    print(f"- {ref} pad {number} at ({x:.4f}, {y:.4f}) mm")
