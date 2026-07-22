#!/usr/bin/env python3
import os, pcbnew
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path=os.path.join(root,'ESP32_FlightController.kicad_pcb'); b=pcbnew.LoadBoard(path)
def P(x,y): return pcbnew.VECTOR2I(pcbnew.FromMM(x),pcbnew.FromMM(y))
def N(n): return b.FindNet(n)
def track(n,layer,pts,w=.15):
    for a,z in zip(pts,pts[1:]):
        t=pcbnew.PCB_TRACK(b); t.SetStart(P(*a)); t.SetEnd(P(*z)); t.SetLayer(layer)
        t.SetWidth(pcbnew.FromMM(w)); t.SetNet(N(n)); b.Add(t)
def via(n,xy,d=.5,h=.2):
    v=pcbnew.PCB_VIA(b); v.SetPosition(P(*xy)); v.SetWidth(pcbnew.FromMM(d)); v.SetDrill(pcbnew.FromMM(h))
    v.SetLayerPair(pcbnew.F_Cu,pcbnew.B_Cu); v.SetNet(N(n)); b.Add(v); return v
def micro(n,xy):
    v=via(n,xy,.3,.1); v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu); return v

# Specctra imports laser microvias as generic buried vias; restore their type.
micro_xy={(50.0375,48.25),(29.4,31.5),(50.0375,49.75)}
for v in b.GetTracks():
    if isinstance(v,pcbnew.PCB_VIA):
        q=v.GetPosition(); xy=(round(pcbnew.ToMM(q.x),4),round(pcbnew.ToMM(q.y),4))
        if xy in micro_xy:
            v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu)

# J9 ground contacts enter the uninterrupted L2 ground plane directly.
micro('GND',(39.8,53.72))
track('GND',pcbnew.In1_Cu,[(50.0375,48.25),(48.9464,47.0)],.2)

# Restore the USB-VBUS decoupler branch on L2.
micro('/USB_VBUS',(49.225,40.0))
track('/USB_VBUS',pcbnew.In1_Cu,[(49.225,40.0),(47.779,49.4962)],.2)

# CC1 uses the rotated manufacturer-clear pad escape then changes to L3.
track('/USB_CC1',pcbnew.F_Cu,[(41.75,53.72),(41.75,54.3133),(41.3039,54.7594)])
via('/USB_CC1',(41.3039,54.7594)); via('/USB_CC1',(33.175,59.6644))
track('/USB_CC1',pcbnew.In2_Cu,[(41.3039,54.7594),(36.0,58.2),(33.175,59.6644)])
track('/USB_CC1',pcbnew.F_Cu,[(33.175,59.6644),(33.175,58.0)])

pcbnew.SaveBoard(path,b)
