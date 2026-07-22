#!/usr/bin/env python3
"""Move every routed via out from beneath top-side IC/module body outlines."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.environ.get(
    "FC_BOARD_PATH", os.path.join(root, "ESP32_FlightController.kicad_pcb")
)
board = pcbnew.LoadBoard(path)
all_tracks = []
for board_item in board.GetTracks():
    all_tracks.append(board_item)


def point(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))


def coord(p):
    return (round(pcbnew.ToMM(p.x), 4), round(pcbnew.ToMM(p.y), 4))


# Old via location -> new off-body location.
moves = {
    (22.4548, 3.5650): (19.7, 3.52),
    (35.8525, 4.2352): (39.8, 4.79),
    (35.0835, 14.7166): (39.8, 8.6),
    (22.4548, 8.3605): (19.7, 8.6),
    (24.1089, 18.5775): (19.7, 11.14),
    (36.7303, 20.2411): (35.72, 21.2),
    (22.4537, 13.7899): (19.7, 13.68),
    (23.6262, 12.8382): (19.7, 12.41),
    (23.1782, 19.2073): (19.7, 14.95),
    (36.6216, 15.7972): (39.8, 17.49),
    (35.0835, 13.3997): (39.8, 13.68),
    (37.5402, 13.0450): (39.8, 11.14),
    (56.9748, 51.4685): (59.2, 52.0),
    (56.9748, 52.5470): (59.2, 52.95),
    (37.8225, 48.6879): (40.1, 48.675),
    (39.0000, 34.2115): (36.9, 34.05),
    (46.8093, 38.5000): (45.6, 37.55),
}

u1_pad_escape = {
    "+3V3": ((21.25, 3.52), (19.7, 3.52)),
    "/BAT_ADC": ((21.25, 8.6), (19.7, 8.6)),
    "/MOTOR_RR_RAW": ((21.25, 11.14), (19.7, 11.14)),
    "/IMU_CS": ((21.25, 12.41), (19.7, 12.41)),
    "/MOTOR_FL_RAW": ((21.25, 13.68), (19.7, 13.68)),
    "/MOTOR_RL_RAW": ((21.25, 14.95), (19.7, 14.95)),
    "/BOOT_STATUS_LED": ((35.72, 20.01), (35.72, 21.2)),
    "/MOTOR_FR_RAW": ((38.75, 17.49), (39.8, 17.49)),
    "/SPI_SCK": ((38.75, 13.68), (39.8, 13.68)),
    "/SPI_MISO": ((38.75, 11.14), (39.8, 11.14)),
    "/I2C_SDA": ((38.75, 8.6), (39.8, 8.6)),
    "/I2C_SCL": ((38.75, 4.79), (39.8, 4.79)),
}

# Remove the old U1 front-layer fanout only. Other-layer routes are retained
# and their endpoints are translated below.
removed_tracks = set()
for track in all_tracks:
    if isinstance(track, pcbnew.PCB_VIA) or track.GetLayer() != pcbnew.F_Cu:
        continue
    if track.GetNetname() not in u1_pad_escape:
        continue
    a = track.GetStart()
    z = track.GetEnd()
    if all(19.0 <= pcbnew.ToMM(p.x) <= 41.0 and
           0.0 <= pcbnew.ToMM(p.y) <= 21.5 for p in (a, z)):
        board.Remove(track)
        removed_tracks.add(id(track))

# Move vias. Front-layer endpoints follow the via; existing inner/back routes
# remain untouched and receive a short local stub from the new via position.
via_by_old = {}
for item in all_tracks:
    if id(item) in removed_tracks:
        continue
    if isinstance(item, pcbnew.PCB_VIA):
        old = coord(item.GetPosition())
        if old in moves:
            via_by_old[old] = item
            item.SetPosition(point(*moves[old]))
        continue
    if item.GetLayer() != pcbnew.F_Cu:
        continue
    start = coord(item.GetStart())
    end = coord(item.GetEnd())
    if start in moves:
        item.SetStart(point(*moves[start]))
    if end in moves:
        item.SetEnd(point(*moves[end]))

for old, via in via_by_old.items():
    layers = {}
    for track in all_tracks:
        if isinstance(track, pcbnew.PCB_VIA) or id(track) in removed_tracks:
            continue
        if track.GetLayer() == pcbnew.F_Cu or track.GetNetname() != via.GetNetname():
            continue
        if coord(track.GetStart()) == old or coord(track.GetEnd()) == old:
            layers[track.GetLayer()] = track.GetWidth()
    for layer, width in layers.items():
        stub = pcbnew.PCB_TRACK(board)
        stub.SetStart(point(*moves[old]))
        stub.SetEnd(point(*old))
        stub.SetLayer(layer)
        stub.SetWidth(width)
        stub.SetNet(via.GetNet())
        board.Add(stub)

# Rebuild direct, short, outward-facing U1 pad escapes.
for net_name, (start, end) in u1_pad_escape.items():
    track = pcbnew.PCB_TRACK(board)
    track.SetStart(point(*start))
    track.SetEnd(point(*end))
    track.SetLayer(pcbnew.F_Cu)
    track.SetWidth(pcbnew.FromMM(0.2))
    track.SetNet(board.FindNet(net_name))
    board.Add(track)

pcbnew.SaveBoard(path, board)
print(path)
