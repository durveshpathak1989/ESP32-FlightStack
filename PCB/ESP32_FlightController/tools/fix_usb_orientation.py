#!/usr/bin/env python3
"""Orient USB-C outward, align its PCB-edge datum, and rebuild local fanout."""
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
    v.SetLayerPair(pcbnew.F_Cu,pcbnew.B_Cu); v.SetNet(N(n)); b.Add(v)
def micro(n,xy):
    v=pcbnew.PCB_VIA(b); v.SetPosition(P(*xy)); v.SetWidth(pcbnew.FromMM(.3)); v.SetDrill(pcbnew.FromMM(.1))
    v.SetViaType(pcbnew.VIATYPE_MICROVIA); v.SetLayerPair(pcbnew.F_Cu,pcbnew.In1_Cu); v.SetNet(N(n)); b.Add(v)

for f in b.GetFootprints():
    if f.GetReference()=='J9':
        f.SetOrientationDegrees(0)
        # Footprint's documented local PCB Edge is y=+3.1 mm.
        f.SetPosition(P(43.0,57.4))

rebuild={'/USB_CONN_DP','/USB_CONN_DM','/USB_CC1','/USB_CC2','/USB_VBUS_RAW','/USB_DM','/USB_SHIELD'}
for t in list(b.GetTracks()):
    if t.GetNetname() in rebuild: b.Remove(t)

# CP2102 D- to ESD array, kept above the receptacle hardware on B.Cu.
track('/USB_DM',pcbnew.F_Cu,[(50.0375,49.25),(48.2,49.25)])
via('/USB_DM',(48.2,49.25))
track('/USB_DM',pcbnew.B_Cu,[(48.2,49.25),(49.0,50.2),(59.0,50.2),(59.0,52.95)])
via('/USB_DM',(59.0,52.95))
track('/USB_DM',pcbnew.F_Cu,[(59.0,52.95),(58.1375,52.95)])

# Rotate the original clean staggered pad escapes with the receptacle.  This
# is essential because the duplicated D+/D- contacts alternate in the row.
track('/USB_CONN_DP',pcbnew.F_Cu,
      [(43.75,53.72),(43.75,54.2574),(43.4604,54.547),(42.75,54.547),
       (42.75,53.72)],.15)
track('/USB_CONN_DP',pcbnew.F_Cu,
      [(42.75,53.72),(42.75,54.7409),(40.4918,56.9991)],.15)
micro('/USB_CONN_DP',(40.4918,56.9991)); micro('/USB_CONN_DP',(55.8625,51.05))
track('/USB_CONN_DP',pcbnew.In1_Cu,
      [(40.4918,56.9991),(45.0,57.5),(50.0,54.5),(55.8625,51.05)],.15)

track('/USB_CONN_DM',pcbnew.F_Cu,
      [(43.25,53.72),(43.25,53.141),(42.9755,52.8665),(42.522,52.8665),
       (42.25,53.1385),(42.25,53.72)],.15)
track('/USB_CONN_DM',pcbnew.F_Cu,
      [(42.25,53.72),(42.25,54.657),(40.4817,56.4253)],.15)
via('/USB_CONN_DM',(40.4817,56.4253)); via('/USB_CONN_DM',(56.8,53.8))
track('/USB_CONN_DM',pcbnew.In2_Cu,
      [(40.4817,56.4253),(42.0,57.5),(50.0,57.5),(56.8,53.8)],.15)
track('/USB_CONN_DM',pcbnew.F_Cu,[(56.8,53.8),(55.8625,52.95)],.15)

# USB-C configuration resistors use short front escapes and L2 routing.
track('/USB_CC1',pcbnew.F_Cu,[(41.75,53.72),(41.75,54.3133),(41.3039,54.7594)])
via('/USB_CC1',(41.3039,54.7594)); via('/USB_CC1',(33.175,59.6644))
track('/USB_CC1',pcbnew.In1_Cu,[(41.3039,54.7594),(38.0,57.5),(33.175,59.6644)])
track('/USB_CC1',pcbnew.F_Cu,[(33.175,59.6644),(33.175,58.0)])
track('/USB_CC2',pcbnew.F_Cu,[(44.75,53.72),(44.75,55.8526)])
via('/USB_CC2',(44.75,55.8526)); via('/USB_CC2',(26.175,56.7474))
track('/USB_CC2',pcbnew.In2_Cu,
      [(44.75,55.8526),(45.0,59.3),(30.0,59.3),(26.175,56.7474)])
track('/USB_CC2',pcbnew.F_Cu,[(26.175,56.7474),(26.175,52.0)])

# Both duplicated VBUS groups join below the signal fanout on B.Cu.
micro('/USB_VBUS_RAW',(45.4,53.72)); micro('/USB_VBUS_RAW',(40.6,53.72))
track('/USB_VBUS_RAW',pcbnew.In1_Cu,
      [(45.4,53.72),(45.4,51.5),(40.6,51.5),(40.6,53.72)],.25)
via('/USB_VBUS_RAW',(39.0,51.5))
track('/USB_VBUS_RAW',pcbnew.In1_Cu,[(40.6,51.5),(39.0,51.5)],.3)
via('/USB_VBUS_RAW',(35.8677,55.4677))
track('/USB_VBUS_RAW',pcbnew.B_Cu,
      [(39.0,51.5),(36.5,52.5),(35.8677,55.4677)],.3)

# Reconnect all four shell tabs to the C18 chassis/ground network on B.Cu.
track('/USB_SHIELD',pcbnew.B_Cu,
      [(38.68,60.475),(38.0,58.0),(38.0,56.5),(38.68,54.295)],.25)
track('/USB_SHIELD',pcbnew.B_Cu,
      [(38.0,56.5),(47.0,56.5),(47.32,54.295)],.25)
track('/USB_SHIELD',pcbnew.B_Cu,[(47.32,60.475),(47.0,56.5)],.25)
track('/USB_SHIELD',pcbnew.B_Cu,
      [(47.32,54.295),(48.0,55.5),(58.5,55.5),(58.5,47.0),(57.975,47.0)],.25)

pcbnew.SaveBoard(path,b)
