#!/usr/bin/env python3
"""Add concise connector-function labels to the front silkscreen."""
import os
import pcbnew

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(root, "ESP32_FlightController.kicad_pcb")
board = pcbnew.LoadBoard(path)

# Work in board-relative coordinates. KiCad may normalize the absolute page
# origin when saving, but J9's verified location remains (43.0, 57.4) relative
# to the nominal board origin.
j9 = next(f for f in board.GetFootprints() if f.GetReference() == "J9")
origin_x = pcbnew.ToMM(j9.GetPosition().x) - 43.0
origin_y = pcbnew.ToMM(j9.GetPosition().y) - 57.4

labels = {
    "J14 ESC", "1CUR 2M4 3M3 4M2 5M1 6BAT 7GND", "J5 GPS",
    "J1 VBAT+/GND", "J2 BAT SENSE", "J4 QWIIC", "J6 IBUS",
    "J8 JTAG", "J7 UART/BOOT", "J9 USB PROG", "IMU U2",
    "+X", "+Y", "+Z OUT",
}
legacy_labels = {
    "ESC", "ESC FL", "ESC FR", "ESC RL", "ESC RR",
    "CUR M4 M3 M2 M1 BAT GND", "GPS", "BAT", "BAT IN",
    "BAT SENSE", "VBAT", "VSENSE", "I2C", "QWIIC", "IBUS",
    "JTAG", "UART", "UART PROG", "USB", "USB PROG",
}

# Remove prior connector labels so this script is idempotent.
for item in list(board.GetDrawings()):
    if isinstance(item, pcbnew.PCB_TEXT) and item.GetText() in labels | legacy_labels:
        board.Remove(item)
    elif isinstance(item, pcbnew.PCB_SHAPE) and item.GetLayer() == pcbnew.F_SilkS:
        box = item.GetBoundingBox()
        cx = pcbnew.ToMM(box.GetCenter().x) - origin_x
        cy = pcbnew.ToMM(box.GetCenter().y) - origin_y
        if 29.0 <= cx <= 33.5 and 26.0 <= cy <= 31.0:
            board.Remove(item)


def add(text, x, y, angle=0, size=0.85):
    item = pcbnew.PCB_TEXT(board)
    item.SetText(text)
    item.SetLayer(pcbnew.F_SilkS)
    item.SetPosition(pcbnew.VECTOR2I(pcbnew.FromMM(origin_x + x),
                                    pcbnew.FromMM(origin_y + y)))
    item.SetTextSize(pcbnew.VECTOR2I(pcbnew.FromMM(size), pcbnew.FromMM(size)))
    item.SetTextThickness(pcbnew.FromMM(0.15))
    item.SetTextAngle(pcbnew.EDA_ANGLE(angle, pcbnew.DEGREES_T))
    item.SetHorizJustify(pcbnew.GR_TEXT_H_ALIGN_CENTER)
    item.SetVertJustify(pcbnew.GR_TEXT_V_ALIGN_CENTER)
    board.Add(item)


def line(x1, y1, x2, y2, width=0.18):
    item = pcbnew.PCB_SHAPE(board)
    item.SetShape(pcbnew.SHAPE_T_SEGMENT)
    item.SetLayer(pcbnew.F_SilkS)
    item.SetStart(pcbnew.VECTOR2I(pcbnew.FromMM(origin_x + x1),
                                  pcbnew.FromMM(origin_y + y1)))
    item.SetEnd(pcbnew.VECTOR2I(pcbnew.FromMM(origin_x + x2),
                                pcbnew.FromMM(origin_y + y2)))
    item.SetWidth(pcbnew.FromMM(width))
    board.Add(item)


# Connector references, functions, and exact pin order.
add("J14 ESC", 8.0, 11.0, size=0.62)
add("1CUR 2M4 3M3 4M2 5M1 6BAT 7GND", 8.4, 20.0, angle=90, size=0.42)
add("J5 GPS", 8.0, 28.5, size=0.60)
add("J1 VBAT+/GND", 8.0, 8.0, size=0.52)
add("J2 BAT SENSE", 9.0, 49.0, size=0.48)

# Right edge, top to bottom.
add("J4 QWIIC", 51.0, 30.0, size=0.58)
add("J6 IBUS", 52.0, 40.0, angle=90, size=0.58)

# Bottom-edge programming and debug interfaces.
add("J8 JTAG", 14.0, 53.0, size=0.58)
add("J9 USB PROG", 43.0, 47.0, size=0.55)
add("J7 UART/BOOT", 53.0, 54.0, size=0.48)

# MPU9250 package/firmware orientation.  U2 is fixed at 0 degrees: +X points
# right, +Y points toward the board top, and +Z is normal to the component side.
add("IMU U2", 30.0, 25.5, size=0.48)
add("+X", 34.0, 30.0, size=0.48)
add("+Y", 30.0, 26.5, size=0.48)
add("+Z OUT", 30.0, 33.8, size=0.42)

# +X points right, +Y points toward the board top, +Z exits the component
# side.  Arrowheads make the direction unambiguous even if text is obscured.
line(30.0, 30.0, 33.2, 30.0)
line(33.2, 30.0, 32.6, 29.6)
line(33.2, 30.0, 32.6, 30.4)
line(30.0, 30.0, 30.0, 26.8)
line(30.0, 26.8, 29.6, 27.4)
line(30.0, 26.8, 30.4, 27.4)

pcbnew.SaveBoard(path, board)
