#!/usr/bin/env python3
import os, pcbnew
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path=os.path.join(root,'ESP32_FlightController.kicad_pcb'); b=pcbnew.LoadBoard(path)
def P(x,y): return pcbnew.VECTOR2I(pcbnew.FromMM(x),pcbnew.FromMM(y))
def N(n): return b.FindNet(n)
def same(a,x,y): return abs(pcbnew.ToMM(a.x)-x)<.002 and abs(pcbnew.ToMM(a.y)-y)<.002
for t in list(b.GetTracks()):
    a,z=t.GetStart(),t.GetEnd()
    if t.GetNetname()=='/USB_VBUS' and ((same(a,49.225,40) and same(z,47.779,49.4962)) or
       (isinstance(t,pcbnew.PCB_VIA) and same(a,49.225,40))): b.Remove(t)
    elif t.GetNetname()=='/USB_CC1' and t.GetLayer()==pcbnew.In2_Cu: b.Remove(t)
def track(n,layer,pts,w=.2):
    for a,z in zip(pts,pts[1:]):
        t=pcbnew.PCB_TRACK(b); t.SetStart(P(*a)); t.SetEnd(P(*z)); t.SetLayer(layer); t.SetWidth(pcbnew.FromMM(w)); t.SetNet(N(n)); b.Add(t)
def via(n,xy,d=.5,h=.2):
    v=pcbnew.PCB_VIA(b); v.SetPosition(P(*xy)); v.SetWidth(pcbnew.FromMM(d)); v.SetDrill(pcbnew.FromMM(h)); v.SetLayerPair(pcbnew.F_Cu,pcbnew.B_Cu); v.SetNet(N(n)); b.Add(v)
via('/USB_VBUS',(49.225,40)); track('/USB_VBUS',pcbnew.B_Cu,[(49.225,40),(46.5,44),(46.5,48),(47.779,49.4962)])
track('/USB_CC1',pcbnew.B_Cu,[(41.3039,54.7594),(36.0,58.2),(33.175,59.6644)],.15)
pcbnew.SaveBoard(path,b)
