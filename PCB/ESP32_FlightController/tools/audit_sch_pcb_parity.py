#!/usr/bin/env python3
"""Verify schematic XML and PCB agree on parts, footprints and every pad net."""
import os
import xml.etree.ElementTree as ET
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
XML = os.path.join(ROOT, "ESP32_FlightController.xml")
PCB = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")

root = ET.parse(XML).getroot()
board = pcbnew.LoadBoard(PCB)
errors = []

parts = {}
for comp in root.findall("./components/comp"):
    parts[comp.get("ref")] = {
        "footprint": comp.findtext("footprint", ""),
        "value": comp.findtext("value", ""),
    }

schematic_nets = {}
for net in root.findall("./nets/net"):
    name = net.get("name", "")
    if name.startswith("unconnected-(U6-"):
        name = name.replace("/", "{slash}")
    for node in net.findall("node"):
        schematic_nets[(node.get("ref"), node.get("pin"))] = name

footprints = {f.GetReference(): f for f in board.GetFootprints()
              if not (f.GetReference().startswith("H") and
                      f.GetReference()[1:].isdigit())}
for ref in sorted(set(parts) - set(footprints)):
    errors.append(f"schematic part missing from PCB: {ref}")
for ref in sorted(set(footprints) - set(parts)):
    errors.append(f"PCB footprint missing from schematic: {ref}")

for ref in sorted(set(parts) & set(footprints)):
    fp = footprints[ref]
    expected_fp = parts[ref]["footprint"]
    actual_fp = fp.GetFPIDAsString()
    # U1 intentionally uses an electrically identical custom land pattern
    # without drilled exposed-pad subpads.
    allowed = {expected_fp}
    if ref == "U1":
        allowed.add("FlightController_Custom:ESP32-WROOM-32_Custom")
    if actual_fp not in allowed:
        errors.append(f"{ref} footprint: schematic {expected_fp}, PCB {actual_fp}")
    for pad in fp.Pads():
        key = (ref, pad.GetNumber())
        expected_net = schematic_nets.get(key)
        actual_net = pad.GetNetname()
        if expected_net is None:
            # Mechanical pads and deliberately unconnected IC pins are absent
            # from the XML net list and must remain electrically blank.
            if actual_net:
                errors.append(f"PCB-only connected pad: {ref}.{pad.GetNumber()} "
                              f"({actual_net})")
        elif actual_net != expected_net:
            errors.append(
                f"{ref}.{pad.GetNumber()} net: schematic {expected_net}, PCB {actual_net}"
            )

if errors:
    print("Schematic/PCB parity FAILED:")
    print("\n".join(f"- {error}" for error in errors))
    raise SystemExit(1)
print(f"Schematic/PCB parity passed ({len(parts)} parts, "
      f"{len(schematic_nets)} pins).")
