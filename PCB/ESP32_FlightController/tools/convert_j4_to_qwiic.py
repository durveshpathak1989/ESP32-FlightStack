#!/usr/bin/env python3
"""Replace J4 with the standard Qwiic JST-SH connector and reroute locally."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)
old = next(f for f in board.GetFootprints() if f.GetReference() == "J4")
origin = old.GetPosition()
orientation = old.GetOrientation()
old_pad_positions = {
    p.GetNumber(): pcbnew.VECTOR2I(p.GetPosition())
    for p in old.Pads() if p.GetNumber() in {"1", "2"}
}
old_signal_targets = {
    "/I2C_SDA": next(p.GetPosition() for p in old.Pads() if p.GetNumber() == "4"),
    "/I2C_SCL": next(p.GetPosition() for p in old.Pads() if p.GetNumber() == "3"),
}
signal_via_positions = {}
for net_name, target in old_signal_targets.items():
    candidates = [t for t in board.GetTracks()
                  if isinstance(t, pcbnew.PCB_VIA) and t.GetNetname() == net_name]
    chosen = min(candidates, key=lambda v:
                 (v.GetPosition().x - target.x) ** 2 +
                 (v.GetPosition().y - target.y) ** 2)
    signal_via_positions[net_name] = pcbnew.VECTOR2I(chosen.GetPosition())

lib = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints/Connector_JST.pretty"
name = "JST_SH_BM04B-SRSS-TB_1x04-1MP_P1.00mm_Vertical"
new = pcbnew.FootprintLoad(lib, name)
if new is None:
    raise RuntimeError("Unable to load the KiCad JST-SH Qwiic footprint")

new.SetReference("J4")
new.SetValue("QWIIC_I2C")
new.SetPosition(origin)
new.SetOrientation(orientation)

pin_nets = {
    "1": "GND",
    "2": "+3V3",
    "3": "/I2C_SDA",
    "4": "/I2C_SCL",
}
for pad in new.Pads():
    if pad.GetNumber() in pin_nets:
        pad.SetNet(board.FindNet(pin_nets[pad.GetNumber()]))
new_pad_positions = {
    p.GetNumber(): pcbnew.VECTOR2I(p.GetPosition())
    for p in new.Pads() if p.GetNumber() in pin_nets
}

# Preserve the existing signal vias but remove the old GH-pad fanout.
for item in list(board.GetTracks()):
    if isinstance(item, pcbnew.PCB_VIA):
        continue
    if item.GetLayer() != pcbnew.F_Cu:
        continue
    if item.GetNetname() not in {"/I2C_SDA", "/I2C_SCL"}:
        continue
    if max(item.GetStart().x, item.GetEnd().x) > origin.x + pcbnew.FromMM(0.5):
        board.Remove(item)

board.Remove(old)
board.Add(new)


def add_track(net_name, start, end, width=0.2):
    item = pcbnew.PCB_TRACK(board)
    item.SetStart(start)
    item.SetEnd(end)
    item.SetLayer(pcbnew.F_Cu)
    item.SetWidth(pcbnew.FromMM(width))
    item.SetNet(board.FindNet(net_name))
    board.Add(item)


# Short power stubs reuse the already verified power/ground routing.
add_track("GND", new_pad_positions["1"], old_pad_positions["1"])
add_track("+3V3", new_pad_positions["2"], old_pad_positions["2"])

# Route Qwiic SDA/SCL directly to their existing escape vias without crossing.
for number, net_name in (("3", "/I2C_SDA"), ("4", "/I2C_SCL")):
    start = new_pad_positions[number]
    destination = signal_via_positions[net_name]
    # One 45-degree corner keeps the short fanout readable and manufacturable.
    dx = destination.x - start.x
    dy = destination.y - start.y
    corner = pcbnew.VECTOR2I(start.x + min(abs(dx), abs(dy)) * (1 if dx >= 0 else -1),
                             start.y + min(abs(dx), abs(dy)) * (1 if dy >= 0 else -1))
    add_track(net_name, start, corner)
    add_track(net_name, corner, destination)

pcbnew.SaveBoard(path, board)
