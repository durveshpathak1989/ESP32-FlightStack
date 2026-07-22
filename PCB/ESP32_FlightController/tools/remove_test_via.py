#!/usr/bin/env python3
"""Remove one PCB via at X,Y from a disposable board for necessity testing."""
import os
import sys
import pcbnew

if len(sys.argv) != 4:
    raise SystemExit("usage: remove_test_via.py BOARD X_MM Y_MM")
path, x_text, y_text = sys.argv[1:]
x, y = float(x_text), float(y_text)
board = pcbnew.LoadBoard(path)
matches = []
for item in board.GetTracks():
    if not isinstance(item, pcbnew.PCB_VIA):
        continue
    pos = item.GetPosition()
    if abs(pcbnew.ToMM(pos.x) - x) < 0.001 and abs(pcbnew.ToMM(pos.y) - y) < 0.001:
        matches.append(item)
if not matches:
    raise SystemExit(f"no via at {x:.4f},{y:.4f}")
for item in matches:
    board.Remove(item)
pcbnew.SaveBoard(path, board)
print(f"removed {len(matches)} via object(s) at {x:.4f},{y:.4f}")
