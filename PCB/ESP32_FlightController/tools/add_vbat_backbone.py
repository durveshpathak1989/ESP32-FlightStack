#!/usr/bin/env python3
"""Add a low-resistance parallel VBAT feed from J1 to the buck input."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.environ.get("FC_BOARD_PATH", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb"))
board = pcbnew.LoadBoard(PATH)
net = board.FindNet("/VBAT_RAW")

def p(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))

via_xy = (15.9, 37.365)
points_f = [(14.7, 37.365), via_xy]
points_b = [via_xy, (15.9, 34.0), (15.0, 33.1), (15.0, 30.0),
            (11.2, 26.2), (11.2, 10.0), (3.0, 5.0)]

# Idempotently remove a previous version of this explicit backbone.
targets = {tuple(round(v, 3) for v in xy) for xy in points_f + points_b}
for item in list(board.GetTracks()):
    if item.GetNetname() != "/VBAT_RAW":
        continue
    if isinstance(item, pcbnew.PCB_VIA):
        q = item.GetPosition()
        got = (round(pcbnew.ToMM(q.x), 3), round(pcbnew.ToMM(q.y), 3))
        if got == tuple(round(v, 3) for v in via_xy):
            board.Remove(item)
    else:
        a, z = item.GetStart(), item.GetEnd()
        got = {(round(pcbnew.ToMM(a.x), 3), round(pcbnew.ToMM(a.y), 3)),
               (round(pcbnew.ToMM(z.x), 3), round(pcbnew.ToMM(z.y), 3))}
        if got.issubset(targets) and pcbnew.ToMM(item.GetWidth()) >= 0.79:
            board.Remove(item)

via = pcbnew.PCB_VIA(board)
via.SetPosition(p(*via_xy))
via.SetWidth(pcbnew.FromMM(0.50))
via.SetDrill(pcbnew.FromMM(0.20))
via.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
via.SetNet(net)
board.Add(via)

for layer, points in ((pcbnew.F_Cu, points_f), (pcbnew.B_Cu, points_b)):
    for a, z in zip(points, points[1:]):
        track = pcbnew.PCB_TRACK(board)
        track.SetStart(p(*a)); track.SetEnd(p(*z))
        track.SetLayer(layer); track.SetWidth(pcbnew.FromMM(0.50 if layer == pcbnew.F_Cu else 0.80))
        track.SetNet(net); board.Add(track)

pcbnew.SaveBoard(PATH, board)
print(PATH)
