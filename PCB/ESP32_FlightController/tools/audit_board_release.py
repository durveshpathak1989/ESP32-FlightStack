#!/usr/bin/env python3
"""Static production-intent audit for mechanical, layer and connector rules."""
import math
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
b = pcbnew.LoadBoard(PATH)
errors = []
fps = {f.GetReference(): f for f in b.GetFootprints()}

def mm(v): return pcbnew.ToMM(v)
def xy(ref):
    q = fps[ref].GetPosition()
    return (mm(q.x), mm(q.y))

# Exact centre and stack mounting pattern.
if any(abs(a - z) > 0.001 for a, z in zip(xy("U2"), (30.0, 30.0))):
    errors.append(f"U2 is not at exact board centre: {xy('U2')}")
holes = [xy(f"H{i}") for i in range(1, 5)]
expected = [(14.75,14.75),(45.25,14.75),(14.75,45.25),(45.25,45.25)]
for i, (got, want) in enumerate(zip(holes, expected), 1):
    if math.dist(got, want) > 0.001:
        errors.append(f"H{i} location {got}, expected {want}")

# All JST connectors must be top-entry vertical; J4 must remain Qwiic SH.
for ref in ("J2", "J4", "J5", "J6", "J14"):
    fp = fps[ref].GetFPIDAsString()
    if "JST_" not in fp or "_Vertical" not in fp:
        errors.append(f"{ref} is not a vertical JST footprint: {fp}")
if "JST_SH_BM04B" not in fps["J4"].GetFPIDAsString():
    errors.append(f"J4 is not Qwiic JST-SH BM04B: {fps['J4'].GetFPIDAsString()}")

# All service connectors are perimeter-accessible. Top-entry headers may be
# rotated for routing, but their bodies must remain within 2.5 mm of an edge.
for ref in ("J1", "J2", "J4", "J5", "J6", "J7", "J8", "J14"):
    box = fps[ref].GetBoundingBox(False, False)
    edge_gap = min(
        mm(box.GetLeft()), 60.2 - mm(box.GetRight()),
        mm(box.GetTop()), 60.5 - mm(box.GetBottom())
    )
    if edge_gap > 2.5:
        errors.append(f"{ref} is not perimeter-accessible: {edge_gap:.2f} mm")

# USB-C shell intentionally overhangs the bottom outline while the footprint's
# PCB-edge datum lands exactly on y=60.50 mm.
j9x, j9y = xy("J9")
if abs(j9x - 43.0) > 0.001 or abs(j9y - 57.4) > 0.001:
    errors.append(f"J9 USB-C position changed: {(j9x, j9y)}")

required_silk = {
    "XT60 FC", "ESC FL", "ESC FR", "ESC RL", "ESC RR",
    "ESC 7P", "GPS", "BAT SENSE", "QWIIC", "IBUS", "UART", "USB",
    "RESET", "BOOT", "POWER", "JTAG", "IMU", "+X", "+Y", "+Z"
}
silk_text = {item.GetText() for item in b.GetDrawings()
             if isinstance(item, pcbnew.PCB_TEXT) and
             item.GetLayer() == pcbnew.F_SilkS}
missing_silk = required_silk - silk_text
if missing_silk:
    errors.append(f"missing required front-silkscreen labels: {sorted(missing_silk)}")

# L2 is an uninterrupted reference plane. No routed signal or power item may
# occupy it; microvia endpoints and the GND zone itself are allowed.
for item in b.GetTracks():
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    if item.GetLayer() == pcbnew.In1_Cu and item.GetNetname() != "GND":
        errors.append(f"non-GND track on L2: {item.GetNetname()}")

# The WROOM PCB antenna projects beyond the top board edge.  Preserve a
# board-coordinate all-copper keepout over the in-board antenna strip.  Keeping
# this out of the footprint avoids the local/global-coordinate ambiguity that
# previously displaced the keepout across routed circuitry.
antenna_areas = [z for z in b.Zones() if z.GetIsRuleArea()]
if len(antenna_areas) != 1:
    errors.append(f"antenna keepout count is {len(antenna_areas)}, expected 1")
else:
    z = antenna_areas[0]
    required_layers = (pcbnew.F_Cu, pcbnew.In1_Cu, pcbnew.In2_Cu, pcbnew.B_Cu)
    if not all(z.IsOnLayer(layer) for layer in required_layers):
        errors.append("U1 antenna keepout does not cover all four copper layers")
    if not all((z.GetDoNotAllowTracks(), z.GetDoNotAllowVias(),
                z.GetDoNotAllowPads(), z.GetDoNotAllowZoneFills())):
        errors.append("U1 antenna keepout does not prohibit every copper feature")
    box = z.GetBoundingBox()
    if (abs(mm(box.GetLeft()) - 20.5) > 0.001 or
            abs(mm(box.GetRight()) - 39.5) > 0.001 or
            abs(mm(box.GetTop())) > 0.001 or
            abs(mm(box.GetBottom()) - 0.7) > 0.001):
        errors.append("antenna keepout geometry changed")

# External 5 V-capable inputs retain their divider parts and exact side.  The
# lone iBUS layer transition is deliberate and outside every component body.
for ref, want_layer in (("R24", "F.Cu"), ("R25", "F.Cu"), ("R26", "F.Cu")):
    if ref not in fps:
        errors.append(f"missing protected-input part {ref}")
    elif fps[ref].GetLayerName() != want_layer:
        errors.append(f"{ref} is on {fps[ref].GetLayerName()}, expected {want_layer}")

# Via-count guardrail: count physical XY drill sites, not KiCad via objects.
# A stacked microvia occupies one visible XY location but is represented by
# multiple objects (one per adjacent-layer transition).  This dense 87-net FC
# still gets a hard ceiling so later edits cannot grow an unbounded via field.
vias = [x for x in b.GetTracks() if isinstance(x, pcbnew.PCB_VIA)]
microvias = [x for x in vias if x.GetViaType() == pcbnew.VIATYPE_MICROVIA]
drill_sites = {(round(mm(x.GetPosition().x), 4),
                round(mm(x.GetPosition().y), 4)) for x in vias}
microvia_sites = {(round(mm(x.GetPosition().x), 4),
                   round(mm(x.GetPosition().y), 4)) for x in microvias}
if len(drill_sites) > 133:
    errors.append(f"excessive via drill sites: {len(drill_sites)} (limit 133)")
if len(microvia_sites) > 31:
    errors.append(
        f"excessive microvia drill sites: {len(microvia_sites)} (limit 31)"
    )

# The 1+N+1 process permits laser microvias only on the outer buildup pairs.
# An L2-L3 0.10 mm hole through the 1.04 mm core is not manufacturable.
for via in microvias:
    pair = {via.TopLayer(), via.BottomLayer()}
    if pair not in ({pcbnew.F_Cu, pcbnew.In1_Cu},
                    {pcbnew.B_Cu, pcbnew.In2_Cu}):
        p = via.GetPosition()
        errors.append(
            "invalid microvia layer pair at "
            f"({mm(p.x):.4f}, {mm(p.y):.4f})"
        )

# U3 has no drilled thermal pad. Its two replacement thermal/ground vias are
# deliberately outside the package courtyard and must remain present.
required_u3_thermal_sites = {(15.9, 38.2), (15.9, 38.8)}
missing_u3_thermal = required_u3_thermal_sites - drill_sites
if missing_u3_thermal:
    errors.append(
        f"missing external U3 thermal via sites: {sorted(missing_u3_thermal)}"
    )

# The controller supply has a dedicated low-resistance path. Its only added
# transition is deliberately outside the U3 body and solder lands.
if (15.9, 37.365) not in drill_sites:
    errors.append("missing wide-VBAT backbone via at (15.9, 37.365)")
wide_vbat_length = sum(
    mm(item.GetLength()) for item in b.GetTracks()
    if not isinstance(item, pcbnew.PCB_VIA) and
    item.GetNetname() == "/VBAT_RAW" and mm(item.GetWidth()) >= 0.79
)
if wide_vbat_length < 40.0:
    errors.append(
        f"wide VBAT backbone too short/missing: {wide_vbat_length:.2f} mm"
    )

# Never permit via-in-pad.  A via only conflicts with a pad when their copper
# layers overlap; a blind via on the opposite side is not a solder aperture.
copper_layers = (pcbnew.F_Cu, pcbnew.B_Cu, pcbnew.In1_Cu, pcbnew.In2_Cu)
for via in vias:
    for footprint in b.GetFootprints():
        for pad in footprint.Pads():
            shared_layer = any(
                via.IsOnLayer(layer) and pad.IsOnLayer(layer)
                for layer in copper_layers
            )
            if shared_layer and pad.HitTest(via.GetPosition()):
                p = via.GetPosition()
                errors.append(
                    "via-in-pad at "
                    f"({mm(p.x):.4f}, {mm(p.y):.4f}) under "
                    f"{footprint.GetReference()} pad {pad.GetNumber()}"
                )

# A drilled footprint subpad is still a hole under a component even though
# KiCad does not represent it as PCB_VIA. Reject every drilled pad belonging
# to a top-side SMD package. Connector and intentional through-hole footprints
# remain allowed.
for footprint in b.GetFootprints():
    if footprint.GetReference()[:1] not in {"U", "Q", "D", "R", "C", "L"}:
        continue
    has_smd_pad = any(
        pad.GetAttribute() == pcbnew.PAD_ATTRIB_SMD for pad in footprint.Pads()
    )
    if not has_smd_pad:
        continue
    for pad in footprint.Pads():
        drill = pad.GetDrillSize()
        if drill.x > 0 or drill.y > 0:
            errors.append(
                f"drilled pad under SMD footprint {footprint.GetReference()} "
                f"pad {pad.GetNumber()} ({mm(drill.x):.3f} x "
                f"{mm(drill.y):.3f} mm drill)"
            )

# The IMU is the assembly-sensitive device.  Keep its complete courtyard free
# of drilled vias, not merely its solder lands.
imu_courtyard = fps["U2"].GetCourtyard(pcbnew.F_CrtYd)
for via in vias:
    if any(
        imu_courtyard.Outline(i).PointInside(via.GetPosition())
        for i in range(imu_courtyard.OutlineCount())
    ):
        p = via.GetPosition()
        errors.append(
            f"via inside U2 IMU courtyard at ({mm(p.x):.4f}, {mm(p.y):.4f})"
        )

# Keep switching/high-current controller nets outside the marked 8 mm IMU
# quiet radius. Propulsion current is not carried by this PCB at all.
noisy_nets = {"/VBAT_RAW", "/SW_NODE", "/+5V_BUCK", "+5V"}
imu_xy = xy("U2")
for item in b.GetTracks():
    if item.GetNetname() not in noisy_nets:
        continue
    samples = [item.GetPosition()] if isinstance(item, pcbnew.PCB_VIA) else [
        item.GetStart(), item.GetEnd()
    ]
    for sample in samples:
        got = (mm(sample.x), mm(sample.y))
        if math.dist(got, imu_xy) < 8.0:
            errors.append(
                f"noisy net {item.GetNetname()} enters IMU quiet radius at {got}"
            )

# High-current propulsion never traverses this board; only the FC pigtail is
# represented. Verify the two battery pads retain their polarity nets.
j1 = fps["J1"]
j1nets = {p.GetNumber(): p.GetNetname() for p in j1.Pads()}
if j1nets != {"1": "/VBAT_RAW", "2": "GND"}:
    errors.append(f"J1 polarity/net mismatch: {j1nets}")

if errors:
    print("Board release audit FAILED:")
    print("\n".join(f"- {e}" for e in errors))
    raise SystemExit(1)
print("Board release audit passed")
