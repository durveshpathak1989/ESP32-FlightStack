#!/usr/bin/env python3
"""Enforce production-width copper on supply and switching-current routes."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
b = pcbnew.LoadBoard(PATH)

widths = {
    "/VBAT_RAW": 1.00,
    "/SW_NODE": 0.80,
    "/+5V_BUCK": 0.60,
    "+5V": 0.60,
    "/USB_VBUS": 0.50,
    "/USB_VBUS_RAW": 0.50,
    "+3V3": 0.40,
}
for t in b.GetTracks():
    if isinstance(t, pcbnew.PCB_VIA):
        continue
    width = widths.get(t.GetNetname())
    # The routed board intentionally retains 0.20 mm neckdowns at fine-pitch
    # pads and congestion points.  Only already-designated trunks are widened;
    # making every segment full width creates DRC shorts around the MPU and
    # USB power mux.
    if width and pcbnew.ToMM(t.GetWidth()) > 0.20:
        t.SetWidth(pcbnew.FromMM(width))

pcbnew.SaveBoard(PATH, b)
print(PATH)
