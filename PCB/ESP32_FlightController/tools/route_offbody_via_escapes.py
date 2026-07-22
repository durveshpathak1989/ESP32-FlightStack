#!/usr/bin/env python3
"""Reroute via transitions outside IC bodies with clearance-aware local A*."""
import heapq
import math
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.environ.get(
    "FC_BOARD_PATH", os.path.join(root, "ESP32_FlightController.kicad_pcb")
)
board = pcbnew.LoadBoard(path)


def mm(value):
    return pcbnew.ToMM(value)


def point(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))


def xy(p):
    return (round(mm(p.x), 4), round(mm(p.y), 4))


moves = {
    (22.4548, 3.5650): (19.7, 3.5),
    (35.8525, 4.2352): (40.7, 4.8),
    (35.0835, 14.7166): (40.7, 8.6),
    (22.4548, 8.3605): (19.7, 8.6),
    (24.1089, 18.5775): (19.7, 11.1),
    (36.7303, 20.2411): (40.5, 20.24),
    (22.4537, 13.7899): (19.7, 13.7),
    (23.6262, 12.8382): (19.7, 12.4),
    (23.1782, 19.2073): (19.7, 15.0),
    (36.6216, 15.7972): (40.7, 17.5),
    (35.0835, 13.3997): (40.7, 13.7),
    (37.5402, 13.0450): (40.7, 11.1),
    (56.9748, 51.4685): (59.2, 52.0),
    (56.9748, 52.5470): (59.2, 53.0),
    (37.8225, 48.6879): (40.3, 48.7),
    (39.0000, 34.2115): (36.9, 34.1),
    (46.8093, 38.5000): (46.0, 37.55),
}

u1_escapes = {
    "+3V3": ((21.25, 3.52), (19.7, 3.5)),
    "/BAT_ADC": ((21.25, 8.6), (19.7, 8.6)),
    "/MOTOR_RR_RAW": ((21.25, 11.14), (19.7, 11.1)),
    "/IMU_CS": ((21.25, 12.41), (19.7, 12.4)),
    "/MOTOR_FL_RAW": ((21.25, 13.68), (19.7, 13.7)),
    "/MOTOR_RL_RAW": ((21.25, 14.95), (19.7, 15.0)),
    "/BOOT_STATUS_LED": ((35.72, 20.01), (40.5, 20.24)),
    "/MOTOR_FR_RAW": ((38.75, 17.49), (40.7, 17.5)),
    "/SPI_SCK": ((38.75, 13.68), (40.7, 13.7)),
    "/SPI_MISO": ((38.75, 11.14), (40.7, 11.1)),
    "/I2C_SDA": ((38.75, 8.6), (40.7, 8.6)),
    "/I2C_SCL": ((38.75, 4.79), (40.7, 4.8)),
}

tracks = []
for item in board.GetTracks():
    tracks.append(item)

# Spatially index copper obstacles once. Calling SWIG HitTest on every board
# item for every A* node is prohibitively slow on a routed board.
GRID = 1.0
obstacle_bins = {}


def index_obstacle(item, layer, net_name):
    box = item.GetBoundingBox()
    xmin, xmax = mm(box.GetLeft()), mm(box.GetRight())
    ymin, ymax = mm(box.GetTop()), mm(box.GetBottom())
    for ix in range(math.floor(xmin / GRID), math.floor(xmax / GRID) + 1):
        for iy in range(math.floor(ymin / GRID), math.floor(ymax / GRID) + 1):
            obstacle_bins.setdefault((layer, ix, iy), []).append((item, net_name))


for item in tracks:
    if isinstance(item, pcbnew.PCB_VIA):
        for layer in (pcbnew.F_Cu, pcbnew.In1_Cu, pcbnew.In2_Cu, pcbnew.B_Cu):
            if item.IsOnLayer(layer):
                index_obstacle(item, layer, item.GetNetname())
    else:
        index_obstacle(item, item.GetLayer(), item.GetNetname())
for footprint in board.GetFootprints():
    for pad in footprint.Pads():
        for layer in (pcbnew.F_Cu, pcbnew.In1_Cu, pcbnew.In2_Cu, pcbnew.B_Cu):
            if pad.IsOnLayer(layer):
                index_obstacle(pad, layer, pad.GetNetname())

# Record which layers terminated at each old via before modifying geometry.
via_info = {}
for item in tracks:
    if not isinstance(item, pcbnew.PCB_VIA):
        continue
    old = xy(item.GetPosition())
    if old not in moves:
        continue
    attached = []
    for track in tracks:
        if isinstance(track, pcbnew.PCB_VIA) or track.GetNetname() != item.GetNetname():
            continue
        if xy(track.GetStart()) == old:
            attached.append((track, track.GetLayer(), track.GetWidth(),
                             xy(track.GetEnd())))
        elif xy(track.GetEnd()) == old:
            attached.append((track, track.GetLayer(), track.GetWidth(),
                             xy(track.GetStart())))
    via_info[old] = (item, attached)

# Remove only U1's old front fanout. The other routes remain as targets for
# the new local escape paths.
removed = set()
for track in tracks:
    if isinstance(track, pcbnew.PCB_VIA) or track.GetLayer() != pcbnew.F_Cu:
        continue
    if track.GetNetname() not in u1_escapes:
        continue
    endpoints = (track.GetStart(), track.GetEnd())
    if all(19.0 <= mm(p.x) <= 41.0 and 0.0 <= mm(p.y) <= 21.5
           for p in endpoints):
        board.Remove(track)
        removed.add(id(track))

# Detach the original via-end segments. Reconnecting to their far endpoints
# avoids routing back into the congested component body merely to meet the old
# via coordinate. Zero-length stubs are discarded.
for _, (_, attached) in via_info.items():
    for track, _, _, _ in attached:
        if id(track) not in removed:
            board.Remove(track)
            removed.add(id(track))

for old, (via, _) in via_info.items():
    via.SetPosition(point(*moves[old]))


def blocked(x, y, layer, net_name, width, endpoints):
    if x < 0.5 or x > 59.7 or y < 0.5 or y > 60.0:
        return True
    p = point(x, y)
    accuracy = pcbnew.FromMM(0.15 + width / 2)
    nearby = obstacle_bins.get((layer, math.floor(x / GRID),
                                math.floor(y / GRID)), ())
    for item, item_net in nearby:
        if id(item) in removed or item_net == net_name:
            continue
        if isinstance(item, pcbnew.PCB_VIA):
            if item.IsOnLayer(layer) and item.HitTest(p, accuracy):
                return True
        elif item.HitTest(p, accuracy):
            return True
    return False


def route_astar(net_name, layer, start, end, width):
    scale = 10  # 0.10 mm grid for dense routed areas
    s = (round(start[0] * scale), round(start[1] * scale))
    g = (round(end[0] * scale), round(end[1] * scale))
    margin = 8.0
    xmin = int((min(start[0], end[0]) - margin) * scale)
    xmax = int((max(start[0], end[0]) + margin) * scale)
    ymin = int((min(start[1], end[1]) - margin) * scale)
    ymax = int((max(start[1], end[1]) + margin) * scale)
    directions = ((1, 0), (-1, 0), (0, 1), (0, -1),
                  (1, 1), (1, -1), (-1, 1), (-1, -1))
    queue = [(0.0, 0.0, s, None)]
    best = {(s, None): 0.0}
    parent = {}
    finish = None
    while queue:
        _, cost, node, prev_dir = heapq.heappop(queue)
        if node == g:
            finish = (node, prev_dir)
            break
        for direction in directions:
            nxt = (node[0] + direction[0], node[1] + direction[1])
            if not (xmin <= nxt[0] <= xmax and ymin <= nxt[1] <= ymax):
                continue
            nx, ny = nxt[0] / scale, nxt[1] / scale
            mx = (node[0] + nxt[0]) / (2 * scale)
            my = (node[1] + nxt[1]) / (2 * scale)
            if nxt not in (s, g) and (
                blocked(nx, ny, layer, net_name, width, (start, end)) or
                blocked(mx, my, layer, net_name, width, (start, end))
            ):
                continue
            step = math.sqrt(2) if direction[0] and direction[1] else 1.0
            turn = 0.35 if prev_dir is not None and prev_dir != direction else 0.0
            new_cost = cost + step + turn
            state = (nxt, direction)
            if new_cost >= best.get(state, float("inf")):
                continue
            best[state] = new_cost
            parent[state] = (node, prev_dir)
            heuristic = math.hypot(g[0] - nxt[0], g[1] - nxt[1])
            heapq.heappush(queue, (new_cost + heuristic, new_cost, nxt, direction))
    if finish is None:
        print(f"WARN direct fallback: {net_name} on {board.GetLayerName(layer)}")
        pts = [start, end]
        for a, z in zip(pts, pts[1:]):
            track = pcbnew.PCB_TRACK(board)
            track.SetStart(point(*a))
            track.SetEnd(point(*z))
            track.SetLayer(layer)
            track.SetWidth(pcbnew.FromMM(width))
            track.SetNet(board.FindNet(net_name))
            board.Add(track)
            tracks.append(track)
        return
    nodes = []
    state = finish
    while state in parent:
        nodes.append(state[0])
        state = parent[state]
    nodes.append(s)
    nodes.reverse()
    pts = [start]
    for node in nodes[1:-1]:
        pts.append((node[0] / scale, node[1] / scale))
    pts.append(end)
    # Collapse collinear grid steps.
    clean = [pts[0]]
    for p in pts[1:]:
        if len(clean) >= 2:
            a, z = clean[-2], clean[-1]
            if abs((z[0]-a[0])*(p[1]-z[1]) -
                   (z[1]-a[1])*(p[0]-z[0])) < 1e-6:
                clean[-1] = p
                continue
        clean.append(p)
    for a, z in zip(clean, clean[1:]):
        track = pcbnew.PCB_TRACK(board)
        track.SetStart(point(*a))
        track.SetEnd(point(*z))
        track.SetLayer(layer)
        track.SetWidth(pcbnew.FromMM(width))
        track.SetNet(board.FindNet(net_name))
        board.Add(track)
        tracks.append(track)


# Preserve every original layer connection with a local path to its old node.
for old, (via, attached) in via_info.items():
    new = moves[old]
    for _, layer, width_internal, far in attached:
        if layer == pcbnew.F_Cu and via.GetNetname() in u1_escapes:
            continue
        if math.hypot(far[0] - old[0], far[1] - old[1]) < 0.01:
            continue
        route_astar(via.GetNetname(), layer, new, far,
                    max(0.15, mm(width_internal)))

# Rebuild U1's front escapes from the castellated pad directly outward.
for net_name, (start, end) in u1_escapes.items():
    route_astar(net_name, pcbnew.F_Cu, start, end, 0.2)

pcbnew.SaveBoard(path, board)
print(path)
