#!/usr/bin/env python3
"""Replace generated input-divider land patterns with canonical KiCad parts."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
lib = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints/Resistor_SMD.pretty"
board = pcbnew.LoadBoard(path)
io = pcbnew.PCB_IO_KICAD_SEXPR()

specs = {
    "R24": ("20k 1%", (14.9, 23.275), 0.0, False,
            {"1": "/ESC_CURRENT_ADC", "2": "GND"}),
    "R25": ("10k 1%", (53.3, 41.25), 0.0, True,
            {"1": "/IBUS_RX", "2": "/IBUS_RAW"}),
    "R26": ("20k 1%", (51.0, 42.075), -90.0, True,
            {"1": "/IBUS_RX", "2": "GND"}),
}

for ref, (value, pos, angle, back, nets) in specs.items():
    old = board.FindFootprintByReference(ref)
    if old is None:
        raise RuntimeError(f"missing {ref}")
    board.Remove(old)
    fp = io.FootprintLoad(lib, "R_0603_1608Metric", False)
    fp.SetFPID(pcbnew.LIB_ID("Resistor_SMD", "R_0603_1608Metric"))
    fp.SetReference(ref)
    fp.SetValue(value)
    fp.SetPosition(pcbnew.VECTOR2I_MM(*pos))
    if back:
        fp.Flip(fp.GetPosition(), False)
    fp.SetOrientationDegrees(angle)
    for pad in fp.Pads():
        pad.SetNet(board.FindNet(nets[pad.GetNumber()]))
    board.Add(fp)

pcbnew.SaveBoard(path, board)
print("Refreshed R24-R26 from the canonical KiCad 0603 library footprint")
