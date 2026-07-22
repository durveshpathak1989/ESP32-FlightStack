#!/usr/bin/env python3
"""Finish the no-via-in-pad production candidate from a supplied board."""
import argparse
import pcbnew


def point(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))


def at(item, x, y, tolerance_mm=0.002):
    p = item.GetPosition()
    return abs(pcbnew.ToMM(p.x) - x) < tolerance_mm and abs(pcbnew.ToMM(p.y) - y) < tolerance_mm


parser = argparse.ArgumentParser()
parser.add_argument("source")
parser.add_argument("output")
args = parser.parse_args()
board = pcbnew.LoadBoard(args.source)


def net(name):
    return board.FindNet(name)


def add_track(name, layer, start, end, width=0.25):
    track = pcbnew.PCB_TRACK(board)
    track.SetStart(point(*start))
    track.SetEnd(point(*end))
    track.SetLayer(layer)
    track.SetWidth(pcbnew.FromMM(width))
    track.SetNet(net(name))
    board.Add(track)


def add_microvia(name, location):
    via = pcbnew.PCB_VIA(board)
    via.SetPosition(point(*location))
    via.SetWidth(pcbnew.FromMM(0.3))
    via.SetDrill(pcbnew.FromMM(0.1))
    via.SetViaType(pcbnew.VIATYPE_MICROVIA)
    via.SetLayerPair(pcbnew.F_Cu, pcbnew.In1_Cu)
    via.SetNet(net(name))
    board.Add(via)


def add_through_via(name, location):
    via = pcbnew.PCB_VIA(board)
    via.SetPosition(point(*location))
    via.SetWidth(pcbnew.FromMM(0.5))
    via.SetDrill(pcbnew.FromMM(0.2))
    via.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
    via.SetNet(net(name))
    board.Add(via)


# Remove the two crossing ground escapes and their vias.
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        if (at(item, 39.8, 52.7) or at(item, 33.9, 48.675)):
            board.Remove(item)
    else:
        endpoints = (item.GetStart(), item.GetEnd())
        coords = {
            (round(pcbnew.ToMM(p.x), 4), round(pcbnew.ToMM(p.y), 4))
            for p in endpoints
        }
        if coords in (
            {(39.8, 53.72), (39.8, 52.7)},
            {(34.85, 48.675), (33.9, 48.675)},
            {(31.6263, 31.8463), (31.6263, 31.0)},
            {(34.0483, 49.325), (34.0483, 43.7483)},
            {(34.85, 49.325), (34.0483, 49.325)},
            {(37.5096, 52.8904), (40.1609, 52.8904)},
            {(40.1609, 52.8904), (40.6, 53.3295)},
        ):
            board.Remove(item)

# Shift the buck trunk left, leaving room for U7 pad 2 to escape outward.
add_track("/+5V_BUCK", pcbnew.F_Cu, (34.85, 49.325), (33.0, 49.325), 0.4)
add_track("/+5V_BUCK", pcbnew.F_Cu, (33.0, 49.325), (33.0, 43.7483), 0.4)
add_track("/+5V_BUCK", pcbnew.F_Cu, (33.0, 43.7483), (34.0483, 43.7483), 0.4)
add_track("GND", pcbnew.F_Cu, (34.85, 48.675), (33.6, 48.675))
add_microvia("GND", (33.6, 48.675))

# Put the obstructing USB VBUS section on the back using two off-body vias,
# then connect the left receptacle GND contacts to the existing U7 dogbone.
add_track("/USB_VBUS_RAW", pcbnew.F_Cu,
          (37.5096, 52.8904), (37.5096, 51.8), 0.3)
add_track("/USB_VBUS_RAW", pcbnew.F_Cu,
          (41.4658, 52.4637), (42.2, 51.8), 0.3)
add_through_via("/USB_VBUS_RAW", (37.5096, 51.8))
add_through_via("/USB_VBUS_RAW", (42.2, 51.8))
add_track("/USB_VBUS_RAW", pcbnew.In2_Cu,
          (37.5096, 51.8), (42.2, 51.8), 0.3)
add_track("GND", pcbnew.F_Cu, (39.8, 53.72), (39.8, 50.5))
add_track("GND", pcbnew.F_Cu, (39.8, 50.5), (40.1, 49.975))

# Join U6 exposed pad to its existing off-pad dogbone without drilling it.
add_track("GND", pcbnew.F_Cu, (50.65, 48.25), (50.0375, 48.25))

# The isolated USB-side back-copper island needs one second stitch.  This
# point is in open copper, outside every footprint bounding box.
add_through_via("GND", (54.7, 51.7))

# C14's small front-copper island escapes beyond the component body.
add_track("GND", pcbnew.F_Cu, (50.775, 40.0), (51.1, 40.0))
add_track("GND", pcbnew.F_Cu, (51.1, 40.0), (51.1, 37.7))
add_track("GND", pcbnew.F_Cu, (51.1, 37.7), (50.9, 37.7))
add_microvia("GND", (50.9, 37.7))

pcbnew.SaveBoard(args.output, board)
