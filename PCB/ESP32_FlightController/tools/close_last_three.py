#!/usr/bin/env python3
import os, pcbnew
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path=os.path.join(root,'ESP32_FlightController.kicad_pcb')
b=pcbnew.LoadBoard(path)
def P(x,y): return pcbnew.VECTOR2I(pcbnew.FromMM(x),pcbnew.FromMM(y))
def N(n): return b.FindNet(n)
def track(n,layer,pts,w=.2):
    for a,z in zip(pts,pts[1:]):
        t=pcbnew.PCB_TRACK(b); t.SetStart(P(*a)); t.SetEnd(P(*z)); t.SetLayer(layer)
        t.SetWidth(pcbnew.FromMM(w)); t.SetNet(N(n)); b.Add(t)
def via(n,xy,d=.5,h=.2):
    v=pcbnew.PCB_VIA(b); v.SetPosition(P(*xy)); v.SetWidth(pcbnew.FromMM(d)); v.SetDrill(pcbnew.FromMM(h))
    v.SetLayerPair(pcbnew.F_Cu,pcbnew.B_Cu); v.SetNet(N(n)); b.Add(v)

# MPU pin 8: filled/capped L1-L2 microvia-in-pad, then join existing 3V3.
v=pcbnew.PCB_VIA(b); v.SetPosition(P(29.4,31.5)); v.SetWidth(pcbnew.FromMM(.3)); v.SetDrill(pcbnew.FromMM(.1))
v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu); v.SetNet(N('+3V3')); b.Add(v)
track('+3V3',pcbnew.In1_Cu,[(29.4,31.5),(32.3916,32.5547)],.15)

# Remove only the local CP2102 pad-2 ground escape; pad 2 remains joined to
# the exposed GND pad and planes on the right, while this opens the USB pair.
# D- threads between the existing D+ and 3V3 vias, then passes below the
# shield hardware on B.Cu before returning to the ESD array.
# Replace the pad-5 through-via escape with an L1-L2 microvia so D- has room.
cut={(50.0375,49.75),(49.3507,49.75),(49.1726,49.9281),(53.4213,49.9281)}
for t in list(b.GetTracks()):
    a,z=t.GetStart(),t.GetEnd(); ax,ay=pcbnew.ToMM(a.x),pcbnew.ToMM(a.y); zx,zy=pcbnew.ToMM(z.x),pcbnew.ToMM(z.y)
    ends={(round(ax,4),round(ay,4)),(round(zx,4),round(zy,4))}
    if t.GetNetname()=='+3V3' and ends & cut:
        b.Remove(t)
    elif t.GetNetname()=='/USB_DP':
        b.Remove(t)
    elif t.GetNetname()=='GND' and (
        (t.GetLayer()==pcbnew.F_Cu and min(ax,zx)>=43.4 and max(ax,zx)<=49.0 and min(ay,zy)>=47.0 and max(ay,zy)<=52.9) or
        (t.GetLayer()==pcbnew.B_Cu and min(ax,zx)>=48.9 and max(ax,zx)<=55.1 and min(ay,zy)>=42.9 and max(ay,zy)<=47.1) or
        (isinstance(t,pcbnew.PCB_VIA) and abs(ax-48.9464)<.01 and abs(ay-47.0)<.01)):
        b.Remove(t)
    elif t.GetNetname()=='GND' and (50.0375,48.25) in ends and (48.9464,48.25) in ends:
        b.Remove(t)
v=pcbnew.PCB_VIA(b); v.SetPosition(P(50.0375,49.75)); v.SetWidth(pcbnew.FromMM(.3)); v.SetDrill(pcbnew.FromMM(.1))
v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu); v.SetNet(N('+3V3')); b.Add(v)
track('+3V3',pcbnew.In1_Cu,[(50.0375,49.75),(52.8,49.75),(54.3854,50.8922)],.15)
# Move D+ upward so D- can fan straight left from the adjacent 0.5 mm pad.
track('/USB_DP',pcbnew.F_Cu,[(50.0375,48.75),(48.7,48.75)],.15)
via('/USB_DP',(48.7,48.75))
track('/USB_DP',pcbnew.In1_Cu,[(48.7,48.75),(57.8,48.75),(58.4,48.9)],.15)
via('/USB_DP',(58.4,48.9))
track('/USB_DP',pcbnew.F_Cu,[(58.4,48.9),(58.1375,49.1625),(58.1375,51.05)],.15)
track('/USB_DM',pcbnew.F_Cu,
      [(50.0375,49.25),(48.2,49.25)],.15)
via('/USB_DM',(48.2,49.25))
track('/USB_DM',pcbnew.B_Cu,
      [(48.2,49.25),(46.2,51.5),(46.2,55.5),(59.0,55.5),(59.0,52.95)],.15)
via('/USB_DM',(59.0,52.95))
track('/USB_DM',pcbnew.F_Cu,[(59.0,52.95),(58.1375,52.95)],.15)

# Restore a clean F/B ground-plane stitching point after removing the old
# CP2102 escape branch.
via('GND',(48.9464,47.0),.6,.3)
track('GND',pcbnew.B_Cu,[(48.9464,47.0),(50.2208,47.0),(55.025,47.0)],.2)
v=pcbnew.PCB_VIA(b); v.SetPosition(P(50.0375,48.25)); v.SetWidth(pcbnew.FromMM(.3)); v.SetDrill(pcbnew.FromMM(.1))
v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu); v.SetNet(N('GND')); b.Add(v)

# Escape the second USB-C VBUS pad below the receptacle, avoiding CC/data pins.
track('/USB_VBUS_RAW',pcbnew.F_Cu,[(45.4,58.88),(45.4,59.7)],.25)
via('/USB_VBUS_RAW',(45.4,59.7))
via('/USB_VBUS_RAW',(35.8677,55.4677))
track('/USB_VBUS_RAW',pcbnew.B_Cu,[(45.4,59.7),(37.0,59.7),(35.8677,58.5),(35.8677,55.4677)],.3)

pcbnew.SaveBoard(path,b)
