#!/usr/bin/env python3
"""Finalize the continuous ground plane and service/manufacturing details."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
b = pcbnew.LoadBoard(PATH)
footprints = list(b.GetFootprints())

def mm(v):
    return pcbnew.FromMM(v)

# Every top-side SMD ground pad gets a filled/capped L1-L2 microvia in pad.
# This is already required by the MPU and USB-UART fanout fabrication note.
existing = set()
for fp in footprints:
    for pad in fp.Pads():
        if pad.GetNetname() != "GND" or pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH:
            continue
        p = pad.GetPosition()
        key = (p.x, p.y, pad.GetNetCode())
        if key in existing:
            continue
        v = pcbnew.PCB_VIA(b)
        v.SetPosition(p)
        v.SetNet(pad.GetNet())
        v.SetViaType(pcbnew.VIATYPE_MICROVIA)
        v.SetLayerPair(pcbnew.F_Cu, pcbnew.In1_Cu)
        v.SetWidth(mm(0.30))
        v.SetDrill(mm(0.10))
        b.Add(v)
        existing.add(key)

# Normalize local clearances to the verified HDI design rule.  Library
# defaults of 0.20 mm must not silently override the board's 0.15 mm rule.
for fp in footprints:
    fp.SetLocalClearance(mm(0.15))

# Layer 2 is the authoritative uninterrupted ground reference.  Remove the
# fragmented outer-layer GND pours that create isolated islands and thermals.
for z in list(b.Zones()):
    if z.GetNetname() == "GND" and z.GetLayer() in (pcbnew.F_Cu, pcbnew.B_Cu):
        b.Remove(z)

# Replace the rectangular outline with one rounded 60.2 x 60.5 mm rectangle.
for item in list(b.GetDrawings()):
    if item.GetLayer() == pcbnew.Edge_Cuts:
        b.Remove(item)
outline = pcbnew.PCB_SHAPE(b)
outline.SetShape(pcbnew.SHAPE_T_RECT)
outline.SetStart(pcbnew.VECTOR2I_MM(0, 0))
outline.SetEnd(pcbnew.VECTOR2I_MM(60.2, 60.5))
outline.SetCornerRadius(mm(3.0))
outline.SetLayer(pcbnew.Edge_Cuts)
outline.SetWidth(mm(0.15))
b.Add(outline)

pcbnew.SaveBoard(PATH, b)
print(PATH)
