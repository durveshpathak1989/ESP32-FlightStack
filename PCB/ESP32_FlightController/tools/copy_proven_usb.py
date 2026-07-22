#!/usr/bin/env python3
"""Copy the already DRC-proven USB routes from the prior clean board."""
import os, pcbnew
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
dst=pcbnew.LoadBoard(os.path.join(root,"ESP32_FlightController.kicad_pcb"))
src=pcbnew.LoadBoard(os.path.join(root,"ESP32_FlightController.clean-before-power-widths.kicad_pcb"))

def clone_track(t):
    if isinstance(t, pcbnew.PCB_VIA):
        n=pcbnew.PCB_VIA(dst); n.SetPosition(t.GetPosition()); n.SetWidth(t.GetWidth())
        n.SetDrill(t.GetDrillValue()); n.SetLayerPair(t.TopLayer(),t.BottomLayer())
    else:
        n=pcbnew.PCB_TRACK(dst); n.SetStart(t.GetStart()); n.SetEnd(t.GetEnd())
        n.SetWidth(t.GetWidth()); n.SetLayer(t.GetLayer())
    n.SetNet(dst.FindNet(t.GetNetname())); dst.Add(n)

for name in ("/USB_DM","/USB_VBUS_RAW"):
    for t in list(dst.GetTracks()):
        if t.GetNetname()==name: dst.Remove(t)
    for t in src.GetTracks():
        if t.GetNetname()==name: clone_track(t)
pcbnew.SaveBoard(os.path.join(root,"ESP32_FlightController.kicad_pcb"),dst)
