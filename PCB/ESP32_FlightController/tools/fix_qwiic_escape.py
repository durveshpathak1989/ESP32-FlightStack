#!/usr/bin/env python3
"""Clear the Qwiic SDA escape and synchronize J4 metadata."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
j4 = next(f for f in board.GetFootprints() if f.GetReference() == "J4")
j4.SetFPIDAsString(
    "Connector_JST:JST_SH_BM04B-SRSS-TB_1x04-1MP_P1.00mm_Vertical")
j4.SetValue("QWIIC_I2C")
j4.Reference().SetVisible(False)

pad3 = next(p for p in j4.Pads() if p.GetNumber() == "3")
pad_pos = pcbnew.VECTOR2I(pad3.GetPosition())

# Move the old SDA escape via inward, away from Qwiic pins 1/2.
candidates = [t for t in board.GetTracks()
              if isinstance(t, pcbnew.PCB_VIA) and t.GetNetname() == "/I2C_SDA"]
via = max(candidates, key=lambda v: v.GetPosition().x)
old_pos = pcbnew.VECTOR2I(via.GetPosition())
new_pos = pcbnew.VECTOR2I(pad_pos.x - pcbnew.FromMM(1.25), pad_pos.y)

# Delete the temporary front fanout, and drag inner-layer endpoints with the via.
for item in list(board.GetTracks()):
    if item is via:
        continue
    if item.GetNetname() != "/I2C_SDA":
        continue
    if item.GetLayer() == pcbnew.F_Cu and max(item.GetStart().x, item.GetEnd().x) > pad_pos.x - pcbnew.FromMM(2.0):
        board.Remove(item)
        continue
    if item.GetStart() == old_pos:
        item.SetStart(new_pos)
    if item.GetEnd() == old_pos:
        item.SetEnd(new_pos)

via.SetPosition(new_pos)

track = pcbnew.PCB_TRACK(board)
track.SetStart(pad_pos)
track.SetEnd(new_pos)
track.SetLayer(pcbnew.F_Cu)
track.SetWidth(pcbnew.FromMM(0.2))
track.SetNet(board.FindNet("/I2C_SDA"))
board.Add(track)

pcbnew.SaveBoard(path, board)
