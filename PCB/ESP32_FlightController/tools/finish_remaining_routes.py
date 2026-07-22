#!/usr/bin/env python3
"""Close the six nets left by Freerouting using explicit layer changes."""
import os
import pcbnew

ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH=os.path.join(ROOT,"ESP32_FlightController.kicad_pcb")
b=pcbnew.LoadBoard(PATH)
def p(x,y): return pcbnew.VECTOR2I(pcbnew.FromMM(x),pcbnew.FromMM(y))
def net(name): return b.FindNet(name)
def route(name,layer,points,width=0.2):
    q=net(name); points=[p(*v) for v in points]
    for a,z in zip(points,points[1:]):
        t=pcbnew.PCB_TRACK(b); t.SetStart(a); t.SetEnd(z); t.SetLayer(layer)
        t.SetWidth(pcbnew.FromMM(width)); t.SetNet(q); b.Add(t)
def via(name,xy,diameter=0.6,drill=0.3):
    v=pcbnew.PCB_VIA(b); v.SetPosition(p(*xy)); v.SetWidth(pcbnew.FromMM(diameter))
    v.SetDrill(pcbnew.FromMM(drill)); v.SetNet(net(name)); v.SetLayerPair(pcbnew.F_Cu,pcbnew.B_Cu); b.Add(v)
def remove_tracks(name):
    for item in list(b.GetTracks()):
        if item.GetNetname() == name:
            b.Remove(item)

# Battery-sense header branch; the wide J1-C1-U3 backbone already carries load current.
route("/VBAT_RAW",pcbnew.F_Cu,[(1.85,49.625),(2.8,49.625)],0.3)
via("/VBAT_RAW",(2.8,49.625))
route("/VBAT_RAW",pcbnew.B_Cu,[(2.8,49.625),(2.8,40.0),(3.5,38.625)],0.3)

# Join diode/bootstrap PH to the 0.5 mm U3-to-L1 switch-node backbone.
via("/SW_NODE",(18.0,51.775))
via("/SW_NODE",(22.02,40.632))
route("/SW_NODE",pcbnew.B_Cu,
      [(18.0,51.775),(22.5,51.0),(23.5,48.0),(23.5,43.0),(22.02,40.632)],0.5)

# Two isolated 3V3 islands are tied to the L3 3V3 plane with short escapes.
route("+3V3",pcbnew.F_Cu,[(21.25,3.52),(20.5,3.52),(19.5,4.5)],0.2)
via("+3V3",(19.5,4.5))

pcbnew.SaveBoard(PATH,b)
