#!/usr/bin/env python3
"""Enforce production minimum widths on board power distribution tracks."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.environ.get("FC_BOARD_PATH", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb"))
board = pcbnew.LoadBoard(PATH)

# These nets carry controller supply current. Fine-pitch signal/ADC branches
# remain separate nets after their series components and are not widened here.
minimums = {
    "/VBAT_RAW": 0.40,
}

changed = 0
for item in board.GetTracks():
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    wanted = minimums.get(item.GetNetname())
    if wanted is None or pcbnew.ToMM(item.GetWidth()) >= wanted - 0.001:
        continue
    item.SetWidth(pcbnew.FromMM(wanted))
    changed += 1

pcbnew.SaveBoard(PATH, board)
print(f"widened {changed} power-track segments")
