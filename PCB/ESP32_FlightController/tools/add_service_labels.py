#!/usr/bin/env python3
"""Add compact, idempotent front-silkscreen service labels."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.environ.get("FC_BOARD_PATH", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb"))
board = pcbnew.LoadBoard(PATH)

labels = {
    "ESC 7P": (11.0, 16.0, 90),
    "GPS": (9.0, 27.0, 0),
    "BAT SENSE": (9.0, 47.0, 0),
    "QWIIC": (53.0, 26.0, 0),
    "BOOT": (43.5, 35.5, 0),
    "IBUS": (53.0, 45.0, 0),
    "USB": (42.0, 51.5, 0),
    "UART": (52.0, 53.0, 0),
}

existing = {item.GetText(): item for item in board.GetDrawings()
            if isinstance(item, pcbnew.PCB_TEXT) and
            item.GetLayer() == pcbnew.F_SilkS}
for value, (x, y, rotation) in labels.items():
    item = existing.get(value)
    if item is None:
        item = pcbnew.PCB_TEXT(board)
        item.SetText(value)
        item.SetLayer(pcbnew.F_SilkS)
        board.Add(item)
    item.SetPosition(pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y)))
    item.SetTextSize(pcbnew.VECTOR2I(pcbnew.FromMM(0.7), pcbnew.FromMM(0.7)))
    item.SetTextThickness(pcbnew.FromMM(0.12))
    item.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_CENTER)
    item.SetTextAngle(pcbnew.EDA_ANGLE(rotation, pcbnew.DEGREES_T))

pcbnew.SaveBoard(PATH, board)
print(PATH)
