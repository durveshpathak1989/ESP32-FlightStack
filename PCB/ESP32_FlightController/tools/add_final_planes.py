#!/usr/bin/env python3
"""Add the production four-layer plane strategy to an already-routed board."""
import os
import pcbnew

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(HERE, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(PATH)

def p(x, y):
    return pcbnew.VECTOR2I(pcbnew.FromMM(x), pcbnew.FromMM(y))

def net(name):
    return board.FindNet(name)

def zone(name, layer, points, priority=0):
    z = pcbnew.ZONE(board)
    z.SetLayer(layer)
    z.SetNet(net(name))
    z.SetAssignedPriority(priority)
    if name == "GND":
        z.SetPadConnection(pcbnew.ZONE_CONNECTION_FULL)
    poly = z.Outline()
    poly.NewOutline()
    for x, y in points:
        q = p(x, y)
        poly.Append(q.x, q.y)
    board.Add(z)

# L2 is an uninterrupted ground reference.  Ground pours on both outer layers
# reduce return-path inductance and provide shielding without cutting L2.
zone("GND", pcbnew.In1_Cu, [(0.3,0.3),(59.7,0.3),(59.7,59.7),(0.3,59.7)])
zone("GND", pcbnew.F_Cu, [(0.4,0.4),(59.6,0.4),(59.6,59.6),(0.4,59.6)])
zone("GND", pcbnew.B_Cu, [(0.4,0.4),(59.6,0.4),(59.6,59.6),(0.4,59.6)])

# L3 low-voltage planes avoid the centered MPU-9250 quiet region.  The lower
# plane is the muxed 5 V rail; the upper/right plane is 3.3 V.
zone("+3V3", pcbnew.In2_Cu,
     [(18,4),(59.3,4),(59.3,37),(39,37),(39,20),(18,20)], 2)
zone("+5V", pcbnew.In2_Cu,
     [(0.7,37),(59.3,37),(59.3,59.3),(0.7,59.3)], 1)

pcbnew.SaveBoard(PATH, board)
print(PATH)
