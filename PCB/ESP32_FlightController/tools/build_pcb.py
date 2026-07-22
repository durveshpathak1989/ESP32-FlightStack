import os
import xml.etree.ElementTree as ET
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
XML = os.path.join(ROOT, "ESP32_FlightController.xml")
OUT = os.environ.get(
    "FC_BOARD_OUT", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
)
FP_ROOT = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints"

board = pcbnew.BOARD()
NO_ROUTES = os.environ.get("FC_NO_ROUTES") == "1"
NO_ZONES = os.environ.get("FC_NO_ZONES") == "1"
board.SetCopperLayerCount(4)
board.SetLayerName(pcbnew.In1_Cu, "GND_PLANE")
board.SetLayerName(pcbnew.In2_Cu, "POWER_PLANE")
design = board.GetDesignSettings()
design.m_MinClearance = pcbnew.FromMM(0.15)
design.m_TrackMinWidth = pcbnew.FromMM(0.15)
design.m_MinThroughDrill = pcbnew.FromMM(0.20)
design.m_HoleClearance = pcbnew.FromMM(0.15)
design.m_CopperEdgeClearance = pcbnew.FromMM(0.25)
design.m_MinSilkTextHeight = pcbnew.FromMM(0.60)
design.m_MinSilkTextThickness = pcbnew.FromMM(0.10)
design.m_SilkClearance = pcbnew.FromMM(0.15)
design.m_NetSettings.GetDefaultNetclass().SetClearance(pcbnew.FromMM(0.15))

tree = ET.parse(XML)
root = tree.getroot()
components = {}
for comp in root.findall("./components/comp"):
    ref = comp.get("ref")
    fp = comp.findtext("footprint", "")
    value = comp.findtext("value", "")
    if fp:
        components[ref] = (fp, value)

nets = {}
for idx, net_el in enumerate(root.findall("./nets/net"), 1):
    name = net_el.get("name") or f"Net-{idx}"
    net = pcbnew.NETINFO_ITEM(board, name)
    board.Add(net)
    nets[name] = net

def pos(x, y):
    return pcbnew.VECTOR2I_MM(x, y)

placements = {
    "U1": (30.0, 10.5, 0),
    # Exact mechanical center.  Keep the magnetometer axis reference fixed.
    "U2": (30.0, 30.0, 0),
    # Switching power is confined to the lower-left corner, as far from U2 as
    # the 45 mm outline allows.  The LDO stays outside the IMU quiet region.
    "U3": (12.0, 38.0, 0),
    "U4": (39.0, 35.0, 0),
    "U5": (57.0, 52.0, 0),
    "U6": (52.0, 49.0, 0),
    "U7": (37.0, 49.0, 0),
    # Two large through-hole battery pigtail pads.  These feed only the flight
    # controller; the XT60/high-current battery connection stays on the ESC.
    # FC-only VBAT/GND pigtail pads at the top-left service edge.  Keeping
    # these away from U3 prevents the large PTH annuli from crowding the buck.
    "J1": (3.0, 5.0, 0),
    "J2": (3.7, 49.0, 90),
    "J4": (56.3, 30.0, 270),
    "J5": (3.7, 28.5, 90),
    "J6": (56.3, 40.0, 270),
    "J7": (51.5, 57.0, 90),
    "J8": (12.0, 57.0, 90),
    # USB4105 local PCB-edge datum is +3.1 mm: 57.4 + 3.1 = 60.5 mm.
    "J9": (43.0, 57.4, 0),
    # Verified BLS-60A order: CUR, M4/FL, M3/RL, M2/FR, M1/RR, BAT, GND.
    # The side-entry mouth faces the left board edge.
    "J14": (3.7, 16.0, 90),
    "SW1": (48.0, 31.0, 0), "SW2": (25.5, 57.0, 0),
    "SW3": (47.0, 34.0, 0), "L1": (24.0, 44.0, 0),
    "D1": (5.0, 42.0, 90),
    "C1": (9.0, 32.5, 90), "C2": (12.0, 32.0, 90),
    "C3": (18.0, 51.0, 90), "C4": (34.0, 41.0, 0),
    "C5": (38.0, 31.0, 90), "C6": (43.0, 33.0, 90),
    "C7": (44.0, 30.5, 0),
    "C10": (27.5, 27.0, 0), "C11": (32.5, 27.0, 0),
    "C12": (28.0, 35.0, 0),
    "C13": (50.0, 53.0, 0), "C14": (50.0, 40.0, 0),
    "C15": (39.0, 42.0, 0), "C16": (31.0, 51.0, 0),
    "C17": (54.0, 24.0, 0), "C18": (56.5, 47.0, 0),
    "F1": (34.0, 55.0, 0),
    "Q1": (47.5, 38.5, 0), "Q2": (43.0, 38.5, 0),
    "R1": (18.0, 29.0, 0), "R2": (18.0, 32.0, 0),
    "R3": (25.5, 34.0, 90), "R4": (35.0, 29.0, 90),
    "R5": (35.0, 33.5, 90), "R6": (20.0, 35.0, 0),
    "R7": (22.0, 33.0, 0), "R8": (41.0, 25.0, 0),
    "R9": (45.0, 25.0, 0), "R10": (8.5, 14.5, 90),
    "R11": (8.0, 18.0, 90), "R12": (52.0, 14.5, 90),
    "R13": (51.0, 22.5, 90), "R14": (16.8, 38.0, 90),
    "R15": (34.0, 58.0, 0), "R16": (27.0, 52.0, 0),
    "R17": (23.0, 51.0, 90), "R18": (53.0, 44.0, 90),
    "R19": (50.5, 36.0, 90), "R20": (51.0, 43.0, 90),
    "R21": (10.5, 22.5, 90), "C19": (12.5, 22.5, 90),
    "R22": (36.0, 24.0, 90), "D3": (38.5, 24.0, 90),
    "R23": (48.0, 22.0, 90), "D4": (48.0, 25.0, 90),
}

near = {
    "U1": [(17.5, 12.8), (19.0, 13.8), (20.5, 13.8), (22.0, 13.8), (23.5, 13.8), (25.0, 13.8), (26.5, 13.8)],
    "U2": [(19.0, 19.0), (20.5, 19.0), (24.5, 19.0), (26.0, 19.0), (19.0, 26.0), (20.5, 26.0), (24.5, 26.0), (26.0, 26.0)],
    "U3": [(5.5, 30.0), (7.0, 30.0), (8.5, 30.0), (10.0, 30.0), (11.5, 30.0), (13.0, 30.0), (14.5, 30.0), (15.0, 36.5)],
    "U4": [(19.0, 16.0), (20.5, 16.0), (24.5, 16.0), (26.0, 16.0)],
}

def anchor_for_ref(ref):
    if ref.startswith("C1") and ref not in ("C1",):
        return "U2"
    if ref in {"C10", "C11", "C12"}:
        return "U2"
    if ref in {"C3", "C4", "R6", "R7", "R14"}:
        return "U3"
    if ref in {"C5", "C6", "C7", "R8", "R9"}:
        return "U4"
    if ref in {"R1", "R2", "R3", "C2"}:
        return "U3"
    return "U1"

used_near = {k: 0 for k in near}
footprints = {}
for ref, (libfp, value) in components.items():
    if ref == "C4":
        # Compact low-ESR polymer/electrolytic package; select an exact 220 uF
        # 10 V part matching this land pattern during BOM release.
        libfp = "Capacitor_SMD:CP_Elec_6.3x5.8"
    lib, name = libfp.split(":", 1)
    lib_path = (os.path.join(ROOT, lib + ".pretty")
                if lib == "FlightController_Custom"
                else os.path.join(FP_ROOT, lib + ".pretty"))
    fp = pcbnew.FootprintLoad(lib_path, name)
    if fp is None:
        raise RuntimeError(f"Unable to load footprint {libfp} for {ref}")
    fp.SetReference(ref)
    fp.SetValue(value)
    fp.SetFPIDAsString(libfp)
    # Keep fabrication identifiers in the board data, but suppress the dense
    # auto-generated reference/value text on the user-facing silkscreen.
    fp.Reference().SetVisible(False)
    fp.Value().SetVisible(False)
    if ref in placements:
        x, y, rot = placements[ref]
    else:
        anchor = anchor_for_ref(ref)
        slots = near[anchor]
        x, y = slots[used_near[anchor] % len(slots)]
        y += 1.1 * (used_near[anchor] // len(slots))
        used_near[anchor] += 1
        rot = 0
    fp.SetPosition(pos(x, y))
    fp.SetOrientationDegrees(rot)
    board.Add(fp)
    if ref in {"C15", "C18", "R18", "R20"}:
        fp.Flip(fp.GetPosition(), False)
    if ref == "U1":
        # The module land pattern is intentionally customized on this board.
        fp.SetFPID(pcbnew.LIB_ID("FlightController_Custom", "ESP32-WROOM-32_Custom"))
        for graphic in fp.GraphicalItems():
            if graphic.GetLayer() == pcbnew.F_SilkS:
                graphic.SetLayer(pcbnew.F_Fab)
        for pad in fp.Pads():
            if pad.GetNumber() == "39" and pad.GetDrillSize().x:
                pad.SetDrillSize(pos(0.30, 0.30))
                pad.SetSize(pos(0.65, 0.65))
    if ref == "U7":
        # The 0.65 mm-pitch TSSOP is routed to the board's validated 0.15 mm
        # rule instead of the library footprint's 0.20 mm local override.
        fp.SetLocalClearance(pcbnew.FromMM(0.15))
    footprints[ref] = fp

for net_el in root.findall("./nets/net"):
    net_name = net_el.get("name") or ""
    net = nets[net_name]
    for node in net_el.findall("node"):
        ref, pin = node.get("ref"), node.get("pin")
        fp = footprints.get(ref)
        if fp is None:
            continue
        # A footprint may contain several copper shapes with the same pad
        # number (ESP32 exposed pad and TPS5430 thermal-via array).  Assign
        # every matching pad, not only FindPadByNumber()'s first result.
        for pad in fp.Pads():
            if pad.GetNumber() == pin:
                pad.SetNet(net)

outline = pcbnew.PCB_SHAPE(board)
outline.SetShape(pcbnew.SHAPE_T_RECT)
outline.SetStart(pos(0, 0)); outline.SetEnd(pos(60.2, 60.5))
outline.SetCornerRadius(pcbnew.FromMM(3.0))
outline.SetLayer(pcbnew.Edge_Cuts); outline.SetWidth(pcbnew.FromMM(0.15))
board.Add(outline)

for i, (x,y) in enumerate([(14.75,14.75),(45.25,14.75),(14.75,45.25),(45.25,45.25)], 1):
    mh = pcbnew.FootprintLoad(os.path.join(FP_ROOT, "MountingHole.pretty"), "MountingHole_4mm")
    mh.SetReference(f"H{i}"); mh.SetValue("M4 / 30.5mm STACK")
    mh.SetBoardOnly(True)
    mh.Reference().SetVisible(False); mh.Value().SetVisible(False)
    mh.SetPosition(pos(x,y)); board.Add(mh)

def add_zone(net_name, layer, corners, priority=0):
    if NO_ZONES:
        return None
    zone = pcbnew.ZONE(board)
    zone.SetLayer(layer)
    zone.SetNet(nets[net_name])
    zone.SetAssignedPriority(priority)
    # Solid connections are deliberate on the uninterrupted ground reference
    # and low-voltage planes: they avoid fragile one-spoke thermals on the
    # dense flight-controller layout and reduce return-path inductance.
    zone.SetPadConnection(pcbnew.ZONE_CONNECTION_FULL)
    poly = zone.Outline()
    poly.NewOutline()
    for x,y in corners:
        p = pos(x,y)
        poly.Append(p.x, p.y)
    board.Add(zone)
    return zone

gnd_name = next((n for n in nets if n in ("GND", "/GND") or n.endswith("GND")), None)
if gnd_name:
    add_zone(gnd_name, pcbnew.In1_Cu, [(0.3,0.3),(59.7,0.3),(59.7,59.7),(0.3,59.7)], 0)
    add_zone(gnd_name, pcbnew.F_Cu, [(0.4,0.4),(59.6,0.4),(59.6,59.6),(0.4,59.6)], 0)
    add_zone(gnd_name, pcbnew.B_Cu, [(0.4,0.4),(59.6,0.4),(59.6,59.6),(0.4,59.6)], 0)

# L-shaped low-voltage distribution intentionally avoids the 14 x 14 mm IMU
# quiet region.  Only the short, low-current IMU supply spur enters that area.
if "+3V3" in nets:
    add_zone("+3V3", pcbnew.In2_Cu,
             [(18,4),(59.3,4),(59.3,37),(39,37),(39,20),(18,20)], 2)
if "+5V" in nets:
    add_zone("+5V", pcbnew.In2_Cu, [(0.7,37),(59.3,37),(59.3,59.3),(0.7,59.3)], 1)

def track(a, b, net, layer, width=0.20):
    if NO_ROUTES:
        return
    if a == b:
        return
    t = pcbnew.PCB_TRACK(board)
    t.SetStart(a); t.SetEnd(b); t.SetLayer(layer)
    t.SetWidth(pcbnew.FromMM(width)); t.SetNet(net); board.Add(t)

def chamfered_l(a, corner, b, net, layer, width=0.20, chamfer_mm=0.45):
    """Route an L connection with two 45-degree corner transitions."""
    if a == b:
        return
    if a.x == b.x or a.y == b.y:
        track(a, b, net, layer, width)
        return
    r = pcbnew.FromMM(chamfer_mm)
    ax = 1 if corner.x > a.x else -1
    ay = 1 if corner.y > a.y else -1
    bx = 1 if b.x > corner.x else -1
    by = 1 if b.y > corner.y else -1
    # Points immediately before/after the nominal 90-degree corner.  Clamp
    # the chamfer to half of the adjacent segment length.
    da = min(r, abs(corner.x - a.x) // 2 if corner.x != a.x else abs(corner.y - a.y) // 2)
    db = min(r, abs(b.x - corner.x) // 2 if b.x != corner.x else abs(b.y - corner.y) // 2)
    if corner.y == a.y:
        p1 = pcbnew.VECTOR2I(corner.x - ax * da, corner.y)
        p2 = pcbnew.VECTOR2I(corner.x, corner.y + by * db)
    else:
        p1 = pcbnew.VECTOR2I(corner.x, corner.y - ay * da)
        p2 = pcbnew.VECTOR2I(corner.x + bx * db, corner.y)
    track(a, p1, net, layer, width)
    track(p1, p2, net, layer, width)
    track(p2, b, net, layer, width)

via_keys = set()
def via(at, net):
    if NO_ROUTES:
        return
    key = (at.x, at.y, net.GetNetCode())
    if key in via_keys:
        return
    via_keys.add(key)
    v = pcbnew.PCB_VIA(board)
    v.SetPosition(at); v.SetWidth(pcbnew.FromMM(0.6)); v.SetDrill(pcbnew.FromMM(0.3)); v.SetNet(net)
    v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
    board.Add(v)

power_names = {"GND", "+3V3", "+5V", "/VBAT_RAW", "/SW_NODE", "/BUCK_EN"}
signal_nets = []
for name, net in nets.items():
    pads = [p for fp in footprints.values() for p in fp.Pads()
            if p.GetNetCode() == net.GetNetCode()]
    if len(pads) < 2 or name in power_names or name.startswith("unconnected-"):
        continue
    signal_nets.append((name, net, pads))

# Orthogonal two-layer signal routing: horizontal runs use B.Cu, vertical
# trunks use F.Cu.  Unique trunks prevent same-layer intersections and keep
# long routes referenced to the uninterrupted L2 ground plane.
left_lanes = [4.0 + 0.55 * i for i in range(23)]
right_lanes = [41.0 - 0.55 * i for i in range(23)]
left_i = right_i = 0
imu_signal_names = {"/SPI_MOSI", "/SPI_MISO", "/SPI_SCK", "/IMU_CS", "/IMU_INT"}
for idx, (name, net, pads) in enumerate(signal_nets):
    avg_x = sum(p.GetPosition().x for p in pads) / len(pads)
    if avg_x < pcbnew.FromMM(22.5):
        lane_x = left_lanes[left_i % len(left_lanes)]
        left_i += 1
    else:
        lane_x = right_lanes[right_i % len(right_lanes)]
        right_i += 1
    trunk_x = pcbnew.FromMM(lane_x)
    origin = pads[0].GetPosition()
    escape_sign = 1 if trunk_x > origin.x else -1
    origin_escape = pcbnew.VECTOR2I(origin.x + escape_sign * pcbnew.FromMM(0.35),
                                    origin.y + pcbnew.FromMM(0.35))
    track(origin, origin_escape, net, pcbnew.F_Cu, 0.20)
    via(origin_escape, net)
    origin_trunk = pcbnew.VECTOR2I(trunk_x, origin.y)
    chamfered_l(origin_escape,
                pcbnew.VECTOR2I(trunk_x, origin_escape.y),
                origin_trunk, net, pcbnew.B_Cu, 0.20, 0.35)
    via(origin_trunk, net)
    for branch, pad in enumerate(pads[1:], 1):
        end = pad.GetPosition()
        branch_y = end.y + pcbnew.FromMM(0.18 * (branch - 1))
        trunk_end = pcbnew.VECTOR2I(trunk_x, branch_y)
        track(origin_trunk, trunk_end, net, pcbnew.F_Cu, 0.20)
        via(trunk_end, net)
        end_sign = -1 if trunk_x > end.x else 1
        end_escape = pcbnew.VECTOR2I(end.x + end_sign * pcbnew.FromMM(0.35),
                                     end.y + (pcbnew.FromMM(0.35) if branch_y >= end.y else -pcbnew.FromMM(0.35)))
        chamfered_l(trunk_end,
                    pcbnew.VECTOR2I(end_escape.x, trunk_end.y),
                    end_escape, net, pcbnew.B_Cu, 0.20, 0.35)
        via(end_escape, net)
        track(end_escape, end, net, pcbnew.F_Cu, 0.20)

# Complete the plane and local-power nets explicitly.  These tracks are the
# deterministic electrical backbone; the filled zones add parallel copper.
def pads_for(net):
    return [p for fp in footprints.values() for p in fp.Pads()
            if p.GetNetCode() == net.GetNetCode()]

power_route = {
    "GND": (pcbnew.In1_Cu, 0.50),
    "+3V3": (pcbnew.In2_Cu, 0.50),
    "+5V": (pcbnew.In2_Cu, 0.80),
    "/VBAT_RAW": (pcbnew.F_Cu, 1.20),
    "/SW_NODE": (pcbnew.F_Cu, 1.00),
    "/BUCK_EN": (pcbnew.B_Cu, 0.20),
}
for name, (layer, width) in power_route.items():
    net = nets.get(name)
    if not net:
        continue
    pp = pads_for(net)
    if len(pp) < 2:
        continue
    root_pad = pp[0]
    root_at = root_pad.GetPosition()
    if layer in (pcbnew.In1_Cu, pcbnew.In2_Cu, pcbnew.B_Cu):
        via(root_at, net)
    for pad in pp[1:]:
        end = pad.GetPosition()
        if layer in (pcbnew.In1_Cu, pcbnew.In2_Cu, pcbnew.B_Cu):
            via(end, net)
        # Keep the 3V3 sensor spur vertical through the quiet region and move
        # bulk distribution outside it; other nets use the shortest L route.
        if name == "+3V3" and 17.0 < pcbnew.ToMM(end.x) < 28.0 and 17.0 < pcbnew.ToMM(end.y) < 28.0:
            mid1 = pcbnew.VECTOR2I(end.x, pcbnew.FromMM(29.5))
            mid2 = pcbnew.VECTOR2I(root_at.x, pcbnew.FromMM(29.5))
            chamfered_l(end, mid1, mid2, net, layer, width, 0.60)
            track(mid2, root_at, net, layer, width)
        else:
            mid = pcbnew.VECTOR2I(end.x, root_at.y)
            chamfered_l(root_at, mid, end, net, layer, width,
                        0.80 if width >= 0.8 else 0.45)

# Document the magnetometer quiet region on the fabrication drawing.  No
# switching node, battery, +5 V plane, inductor, or connector is placed here.
quiet = pcbnew.PCB_SHAPE(board)
quiet.SetShape(pcbnew.SHAPE_T_CIRCLE)
quiet.SetCenter(pos(30.0, 30.0))
quiet.SetEnd(pos(38.0, 30.0))
quiet.SetLayer(pcbnew.Dwgs_User)
quiet.SetWidth(pcbnew.FromMM(0.20))
board.Add(quiet)

# Ground stitching is added after the planes are filled in KiCad.  Do not add
# free-standing vias here: until a zone is filled they appear as unrouted
# copper items and obscure the real routing-completion count.

for text_value, x, y, size in [
    ("FRONT", 30.0, 1.2, 0.8),
    ("BAT", 6.0, 34.5, 0.8),
    ("ESC FL", 9.0, 12.0, 0.8),
    ("ESC FR", 9.0, 20.0, 0.8),
    ("ESC RL", 51.0, 12.0, 0.8),
    ("ESC RR", 51.0, 20.0, 0.8),
    ("JTAG", 14.0, 53.0, 0.8),
    ("RESET", 48.0, 28.0, 0.8),
    ("POWER", 24.0, 54.0, 0.8),
]:
    txt = pcbnew.PCB_TEXT(board)
    txt.SetText(text_value); txt.SetPosition(pos(x,y)); txt.SetLayer(pcbnew.F_SilkS)
    txt.SetTextSize(pos(size,size)); txt.SetTextThickness(pcbnew.FromMM(0.15)); txt.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_CENTER)
    board.Add(txt)

pcbnew.SaveBoard(OUT, board)
print(OUT)
