#!/usr/bin/env python3
"""Add 5 V-tolerant passive dividers to iBUS and ESC current inputs."""
from pathlib import Path
import re
import uuid

root = Path(__file__).resolve().parent.parent
path = root / "ESP32_FlightController.kicad_sch"
text = path.read_text()

if 'property "Reference" "R24"' in text:
    raise SystemExit("input protection already present")

def symbol_block(source: str, ref: str) -> str:
    marker = f'(property "Reference" "{ref}"'
    hit = source.index(marker)
    start = source.rfind("\n\t(symbol", 0, hit) + 1
    depth = 0
    for pos in range(start, len(source)):
        if source[pos] == "(":
            depth += 1
        elif source[pos] == ")":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise RuntimeError(f"unterminated symbol {ref}")

def clone_resistor(template: str, old_ref: str, new_ref: str, value: str,
                   old_xy: tuple[float, float], new_xy: tuple[float, float]) -> str:
    dx, dy = new_xy[0] - old_xy[0], new_xy[1] - old_xy[1]
    block = template.replace(f'"{old_ref}"', f'"{new_ref}"')
    old_value = re.search(r'\(property "Value" "([^"]+)"', block).group(1)
    block = block.replace(f'(property "Value" "{old_value}"',
                          f'(property "Value" "{value}"', 1)
    block = block.replace("Resistor_SMD:R_0805_2012Metric",
                          "Resistor_SMD:R_0603_1608Metric")

    def shift_at(match: re.Match) -> str:
        x, y, angle = float(match.group(1)), float(match.group(2)), match.group(3)
        return f'(at {x + dx:g} {y + dy:g} {angle})'
    block = re.sub(r'\(at (-?\d+(?:\.\d+)?) (-?\d+(?:\.\d+)?) (\d+)\)',
                   shift_at, block)
    block = re.sub(r'\(uuid "[^"]+"\)',
                   lambda _: f'(uuid "{uuid.uuid4()}")', block)
    return block

vertical = symbol_block(text, "R2")
horizontal = symbol_block(text, "R21")
r24 = clone_resistor(vertical, "R2", "R24", "20k 1%",
                     (62.23, 80.01), (101.6, 137.16))
r25 = clone_resistor(horizontal, "R21", "R25", "10k 1%",
                     (101.6, 128.27), (279.4, 147.32))
r26 = clone_resistor(vertical, "R2", "R26", "20k 1%",
                     (62.23, 80.01), (283.21, 154.94))

# Change the existing ESC series element to the upper divider resistor.
r21 = symbol_block(text, "R21")
r21_new = r21.replace('(property "Value" "1k"',
                      '(property "Value" "10k 1%"', 1)
text = text.replace(r21, r21_new, 1)

# Only J6's connector-side label becomes the raw net.  The ESP32-side label
# remains IBUS_RX and is fed through R25/R26.
j6_label = '''\t(label "IBUS_RX"
\t\t(at 266.7 133.35 0)'''
if text.count(j6_label) != 1:
    raise RuntimeError("could not uniquely locate J6 iBUS label")
text = text.replace(j6_label, '''\t(label "IBUS_RAW"
\t\t(at 266.7 133.35 0)''', 1)

def label(name: str, x: float, y: float, angle: int = 0) -> str:
    justify = "\n\t\t\t(justify right bottom)" if angle == 180 else ""
    return f'''\t(label "{name}"
\t\t(at {x:g} {y:g} {angle})
\t\t(effects
\t\t\t(font
\t\t\t\t(size 1.27 1.27)
\t\t\t){justify}
\t\t)
\t\t(uuid "{uuid.uuid4()}")
\t)'''

def wire(x1: float, y1: float, x2: float, y2: float) -> str:
    return f'''\t(wire
\t\t(pts
\t\t\t(xy {x1:g} {y1:g}) (xy {x2:g} {y2:g})
\t\t)
\t\t(stroke
\t\t\t(width 0)
\t\t\t(type default)
\t\t)
\t\t(uuid "{uuid.uuid4()}")
\t)'''

graphics = "\n".join([
    label("ESC_CURRENT_ADC", 101.6, 133.35, 90),
    label("GND", 101.6, 140.97, 270),
    label("IBUS_RAW", 275.59, 147.32, 180),
    label("IBUS_RX", 283.21, 147.32, 0),
    wire(283.21, 147.32, 283.21, 151.13),
    label("GND", 283.21, 158.75, 270),
    f'''\t(text "PROTECTED EXTERNAL INPUTS: 10k/20k DIVIDERS — 5V INPUT = 3.33V NOMINAL"
\t\t(exclude_from_sim no)
\t\t(at 193.04 144.78 0)
\t\t(effects
\t\t\t(font (size 1.27 1.27) (bold yes))
\t\t\t(justify left bottom)
\t\t)
\t\t(uuid "{uuid.uuid4()}")
\t)'''
])

insert = f"\n{graphics}\n{r24}\n{r25}\n{r26}\n"
anchor = "\n\t(sheet_instances"
if anchor not in text:
    raise RuntimeError("sheet_instances anchor missing")
text = text.replace(anchor, insert + anchor, 1)
path.write_text(text)
print("Added R24-R26 external-input dividers and changed R21 to 10k")
