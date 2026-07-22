#!/usr/bin/env python3
"""Repair Specctra via types and close the USB-edge routing stubs."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)


def point(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))


def net(name):
    return board.FindNet(name)


def track(name, layer, points, width=0.2):
    for start, end in zip(points, points[1:]):
        item = pcbnew.PCB_TRACK(board)
        item.SetStart(point(*start))
        item.SetEnd(point(*end))
        item.SetLayer(layer)
        item.SetWidth(pcbnew.FromMM(width))
        item.SetNet(net(name))
        board.Add(item)


def via(name, xy, diameter=0.5, drill=0.2):
    item = pcbnew.PCB_VIA(board)
    item.SetPosition(point(*xy))
    item.SetWidth(pcbnew.FromMM(diameter))
    item.SetDrill(pcbnew.FromMM(drill))
    item.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
    item.SetNet(net(name))
    board.Add(item)
    return item


def microvia(name, xy):
    item = via(name, xy, 0.3, 0.1)
    item.SetViaType(pcbnew.VIATYPE_MICROVIA)
    item.SetLayerPair(pcbnew.F_Cu, pcbnew.In1_Cu)
    return item


# Specctra imports these existing laser microvias as generic buried vias.
laser_vias = {(50.0375, 48.25), (29.4, 31.5), (50.0375, 49.75)}
for item in board.GetTracks():
    if not isinstance(item, pcbnew.PCB_VIA):
        continue
    pos = item.GetPosition()
    xy = (round(pcbnew.ToMM(pos.x), 4), round(pcbnew.ToMM(pos.y), 4))
    if xy in laser_vias:
        item.SetViaType(pcbnew.VIATYPE_MICROVIA)
        item.SetLayerPair(pcbnew.F_Cu, pcbnew.In1_Cu)

# Remove the obsolete right-side ground branch left by the rotated connector route.
obsolete_ground_points = {
    (48.4797, 52.8609), (49.1923, 53.5735), (49.9065, 53.5735),
    (50.48, 53.0), (51.4227, 53.0), (52.1046, 53.6819),
}
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA) or item.GetNetname() != "GND":
        continue
    ends = {(round(pcbnew.ToMM(p.x), 4), round(pcbnew.ToMM(p.y), 4))
            for p in (item.GetStart(), item.GetEnd())}
    if len(ends & obsolete_ground_points) == 2:
        board.Remove(item)

# Join connector ground pads to existing ground vias without cutting L2.
via("GND", (39.8, 53.72))
track("GND", pcbnew.B_Cu,
      [(39.8, 53.72), (39.0, 52.92), (37.8, 52.92),
       (36.8, 53.92), (36.0, 55.0), (34.7, 55.0),
       (34.2, 55.5), (32.0, 55.5), (30.4513, 54.7157)])
via("GND", (46.2, 53.72))
track("GND", pcbnew.B_Cu,
      [(46.2, 53.72), (45.5, 53.02), (45.5, 50.0),
       (48.0, 47.5), (48.9464, 47.0)])

# Join the duplicated VBUS contact groups on the back layer.
via("/USB_VBUS_RAW", (40.6, 53.72))
via("/USB_VBUS_RAW", (45.4, 53.72))
track("/USB_VBUS_RAW", pcbnew.B_Cu, [(40.6, 53.72), (45.4, 53.72)], 0.3)

# D+ connector side: laser vias in the pads and a short L2 route.
microvia("/USB_CONN_DP", (43.75, 53.72))
microvia("/USB_CONN_DP", (55.8625, 51.05))
track("/USB_CONN_DP", pcbnew.In1_Cu,
      [(43.75, 53.72), (44.25, 54.22), (44.25, 53.0), (45.5, 52.7),
       (49.0, 52.7), (50.5, 54.22), (51.8, 52.92),
       (51.8, 51.5), (55.4125, 51.5), (55.8625, 51.05)])

# D- transceiver side: laser vias in the pads and a short L2 route.
microvia("/USB_DM", (50.0375, 49.25))
microvia("/USB_DM", (58.1375, 52.95))
track("/USB_DM", pcbnew.In1_Cu,
      [(50.0375, 49.25), (57.2, 49.25), (59.0, 51.05),
       (59.0, 52.0875), (58.1375, 52.95)])

pcbnew.SaveBoard(path, board)
