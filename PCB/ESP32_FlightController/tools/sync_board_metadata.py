#!/usr/bin/env python3
"""Synchronize board-only metadata with the authoritative schematic netlist."""
import os
import xml.etree.ElementTree as ET
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BOARD = os.environ.get(
    "FC_BOARD_PATH", os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
)
XML = os.path.join(ROOT, "ESP32_FlightController.xml")
b = pcbnew.LoadBoard(BOARD)
r = ET.parse(XML).getroot()

parts = {}
for c in r.findall("./components/comp"):
    fields = {f.get("name", ""): (f.text or "") for f in c.findall("./fields/field")}
    parts[c.get("ref")] = {
        "value": c.findtext("value", ""),
        "footprint": c.findtext("footprint", ""),
        "datasheet": fields.get("Datasheet", ""),
    }

fps = {f.GetReference(): f for f in b.GetFootprints()}
for ref, info in parts.items():
    f = fps.get(ref)
    if not f:
        continue
    if info["value"]:
        f.SetValue(info["value"])
    f.SetFPIDAsString(info["footprint"])
    if info["datasheet"]:
        f.SetField("Datasheet", info["datasheet"])

# U1 has an intentional board-specific land-pattern modification.  Do not
# falsely retain the stock-library link or DRC reports a mismatch.
if "U1" in fps:
    fps["U1"].SetFPID(pcbnew.LIB_ID("FlightController_Custom", "ESP32-WROOM-32_Custom"))

for ref, f in fps.items():
    if ref.startswith("H") and ref[1:].isdigit():
        f.SetBoardOnly(True)
    else:
        f.SetBoardOnly(False)
        f.SetExcludedFromBOM(False)

# Preserve intentional one-pin no-connect nets so schematic parity can prove
# that every PCB pad has the same electrical identity as the schematic.
for ne in r.findall("./nets/net"):
    name = ne.get("name") or ""
    # KiCad's current-format schematic serializes literal '/' characters in
    # automatically generated no-connect pin names as {slash} for parity.
    if name.startswith("unconnected-(U6-"):
        name = name.replace("/", "{slash}")
    q = b.FindNet(name)
    if not q:
        q = pcbnew.NETINFO_ITEM(b, name)
        b.Add(q)
    for node in ne.findall("node"):
        f = fps.get(node.get("ref"))
        if not f:
            continue
        for pad in f.Pads():
            if pad.GetNumber() == node.get("pin"):
                pad.SetNet(q)

pcbnew.SaveBoard(BOARD, b)
print(BOARD)
