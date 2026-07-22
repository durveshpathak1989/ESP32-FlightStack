#!/usr/bin/env python3
"""Place USB4105 at the physical edge and clear its complete routing bundle."""
import os, pcbnew
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path=os.path.join(root,'ESP32_FlightController.kicad_pcb'); b=pcbnew.LoadBoard(path)
for f in b.GetFootprints():
    if f.GetReference()=='J9':
        f.SetOrientationDegrees(0)
        f.SetPosition(pcbnew.VECTOR2I(pcbnew.FromMM(43.0),pcbnew.FromMM(57.4)))
nets={'/USB_CONN_DP','/USB_CONN_DM','/USB_CC1','/USB_CC2','/USB_VBUS_RAW',
      '/USB_SHIELD','/USB_DM'}
for t in list(b.GetTracks()):
    if t.GetNetname() in nets: b.Remove(t)
pcbnew.SaveBoard(path,b)
pcbnew.ExportSpecctraDSN(b,os.path.join(root,'usb-edge.dsn'))
