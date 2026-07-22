#!/usr/bin/env python3
"""Add DRC-safe, deterministic MPU and USB-C routes before general routing."""
import os
import pcbnew

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, "ESP32_FlightController.kicad_pcb")
b = pcbnew.LoadBoard(PATH)

def p(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))

def net(name):
    q = b.FindNet(name)
    if not q:
        raise RuntimeError(name)
    return q

def route(name, layer, points, width=0.15):
    q = net(name)
    points = [p(*xy) for xy in points]
    for a, z in zip(points, points[1:]):
        t = pcbnew.PCB_TRACK(b)
        t.SetStart(a); t.SetEnd(z); t.SetLayer(layer)
        t.SetWidth(pcbnew.FromMM(width)); t.SetNet(q); b.Add(t)

def via(name, xy, diameter=0.50, drill=0.20):
    v = pcbnew.PCB_VIA(b)
    v.SetPosition(p(*xy)); v.SetWidth(pcbnew.FromMM(diameter))
    v.SetDrill(pcbnew.FromMM(drill)); v.SetNet(net(name))
    v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu); b.Add(v)

def signal(name, u2_pad, u2_escape, rear_path, u1_escape, u1_pad):
    route(name, pcbnew.F_Cu, [u2_pad] + u2_escape)
    via(name, u2_escape[-1])
    route(name, pcbnew.B_Cu, [u2_escape[-1]] + rear_path + [u1_escape], 0.20)
    via(name, u1_escape)
    route(name, pcbnew.F_Cu, [u1_escape, u1_pad])

# Fan straight out from the 0.4 mm-pitch MPU-9250 pads before changing angle.
# The rear-layer trunks do not cross one another and remain referenced to L2.
# CS stays on B.Cu; SCK crosses its vertical leg briefly on L3, outside the
# actual 3V3-pour polygon in the sensor region.
route("/IMU_CS", pcbnew.F_Cu, [(29.8,28.5),(29.8,27.0),(30.8,25.5)])
via("/IMU_CS", (30.8,25.5))
route("/IMU_CS", pcbnew.B_Cu,
      [(30.8,25.5),(32.0,24.0),(32.0,22.0),(18.0,22.0),(18.0,12.41),(19.5,12.41)], 0.20)
via("/IMU_CS", (19.5,12.41))
route("/IMU_CS", pcbnew.F_Cu, [(19.5,12.41),(21.25,12.41)])

# INT changes to L3 only where it passes the right-going SPI bundle.
route("/IMU_INT", pcbnew.F_Cu, [(31.0,31.5),(31.0,32.2),(32.5,33.5)])
via("/IMU_INT", (32.5,33.5))
route("/IMU_INT", pcbnew.B_Cu, [(32.5,33.5),(37.0,34.5),(37.0,29.0)], 0.20)
via("/IMU_INT", (37.0,29.0))
route("/IMU_INT", pcbnew.F_Cu, [(37.0,29.0),(37.0,20.5)], 0.20)
via("/IMU_INT", (37.0,20.5))
route("/IMU_INT", pcbnew.B_Cu,
      [(37.0,20.5),(37.0,18.0),(20.0,18.0),(19.5,16.22)], 0.20)
via("/IMU_INT", (19.5,16.22))
route("/IMU_INT", pcbnew.F_Cu, [(19.5,16.22),(21.25,16.22)])

route("/SPI_SCK", pcbnew.F_Cu, [(29.4,28.5),(29.4,25.5)])
via("/SPI_SCK", (29.4,25.5))
route("/SPI_SCK", pcbnew.B_Cu, [(29.4,25.5),(29.4,23.0)], 0.20)
via("/SPI_SCK", (29.4,23.0))
route("/SPI_SCK", pcbnew.In2_Cu,
      [(29.4,23.0),(40.0,23.0),(42.5,20.5)], 0.20)
via("/SPI_SCK", (42.5,20.5))
route("/SPI_SCK", pcbnew.B_Cu,
      [(42.5,20.5),(42.5,13.68)], 0.20)
via("/SPI_SCK", (42.5,13.68))
route("/SPI_SCK", pcbnew.F_Cu, [(42.5,13.68),(38.75,13.68)])
signal("/SPI_MOSI", (29.0,28.5), [(29.0,27.0),(28.0,25.5)],
       [(26.5,26.5),(40.0,26.5),(41.0,25.5),(41.0,12.41)],
       (40.5,12.41), (38.75,12.41))
signal("/SPI_MISO", (29.8,31.5), [(29.8,32.0),(28.5,33.3)],
       [(35.0,37.0),(49.0,37.0),(49.0,25.0),(50.0,23.0),(50.0,9.0),(40.5,9.0)],
       (40.5,11.14), (38.75,11.14))

pcbnew.SaveBoard(PATH, b)
print(PATH)
