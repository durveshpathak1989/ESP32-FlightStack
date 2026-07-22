#!/usr/bin/env python3
"""Fail unless safety-critical schematic pins land on the intended nets."""
import sys
import xml.etree.ElementTree as ET

path = sys.argv[1]
root = ET.parse(path).getroot()
pin_net = {}
for net in root.findall("./nets/net"):
    name = net.get("name", "")
    for node in net.findall("node"):
        pin_net[(node.get("ref"), node.get("pin"))] = name

expected = {
    # USB-C receptacle: duplicated USB 2.0 pins must be paired.
    ("J9", "A4"): "/USB_VBUS_RAW", ("J9", "A9"): "/USB_VBUS_RAW",
    ("J9", "B4"): "/USB_VBUS_RAW", ("J9", "B9"): "/USB_VBUS_RAW",
    ("J9", "A5"): "/USB_CC1", ("J9", "B5"): "/USB_CC2",
    ("J9", "A6"): "/USB_CONN_DP", ("J9", "B6"): "/USB_CONN_DP",
    ("J9", "A7"): "/USB_CONN_DM", ("J9", "B7"): "/USB_CONN_DM",
    # CP2102N USB, UART and modem-control interface.
    ("U6", "3"): "/USB_DP", ("U6", "4"): "/USB_DM",
    ("U6", "7"): "/USB_VBUS", ("U6", "8"): "/USB_VBUS",
    ("U6", "2"): "GND", ("U6", "25"): "GND",
    ("U6", "5"): "+3V3", ("U6", "6"): "/CP2102_VDD",
    ("U6", "9"): "+3V3",
    ("U6", "19"): "/AUTO_RTS", ("U6", "23"): "/AUTO_DTR",
    ("U6", "20"): "/UART0_TX", ("U6", "21"): "/UART0_RX",
    # Power mux: battery-derived 5 V is preferred over protected USB VBUS.
    ("U7", "8"): "/+5V_BUCK", ("U7", "6"): "/USB_VBUS",
    ("U7", "7"): "+5V", ("U7", "3"): "/+5V_BUCK",
    ("U7", "2"): "GND", ("U7", "5"): "GND",
    ("U7", "4"): "Net-(U7-ILIM)",
    # TPS5430 power stage: catch diode and bootstrap capacitor must join PH.
    ("U3", "8"): "/SW_NODE", ("L1", "1"): "/SW_NODE",
    ("D1", "1"): "/SW_NODE", ("C3", "1"): "/SW_NODE",
    ("C3", "2"): "Net-(U3-BOOT)", ("U3", "1"): "Net-(U3-BOOT)",
    ("C15", "2"): "GND", ("C16", "2"): "GND", ("C17", "2"): "GND",
    # Espressif two-transistor automatic download/reset truth-table network.
    # Q_NPN_BEC uses pin 2=emitter and pin 3=collector.  Espressif's circuit
    # cross-couples the emitters to the opposite modem-control signal.
    ("Q1", "1"): "Net-(Q1-B)", ("Q1", "2"): "/AUTO_RTS",
    ("Q1", "3"): "/ESP_EN",
    ("Q2", "1"): "Net-(Q2-B)", ("Q2", "2"): "/AUTO_DTR",
    ("Q2", "3"): "/BOOT_IO0",
    # Passive two-pin part numbering is orientation-dependent but asserted
    # here so accidental schematic rewiring remains visible.
    ("R18", "1"): "Net-(Q1-B)", ("R18", "2"): "/AUTO_DTR",
    ("R19", "1"): "Net-(Q2-B)", ("R19", "2"): "/AUTO_RTS",
    ("SW3", "1"): "/BOOT_IO0", ("SW3", "2"): "GND",
    # Manual controls and isolated JTAG header.
    ("SW1", "1"): "GND", ("SW1", "2"): "/ESP_EN",
    ("SW2", "1"): "/VBAT_RAW", ("SW2", "2"): "/BUCK_EN",
    ("J8", "1"): "/JTAG_TDI", ("J8", "2"): "/JTAG_TCK",
    ("J8", "3"): "/JTAG_TMS", ("J8", "4"): "/JTAG_TDO",
    ("J8", "5"): "+3V3", ("J8", "6"): "GND",
    ("U1", "13"): "/JTAG_TMS", ("U1", "14"): "/JTAG_TDI",
    ("U1", "16"): "/JTAG_TCK", ("U1", "23"): "/JTAG_TDO",
    # Raw 3S input reaches only the buck/switch path and monitor connector.
    ("J1", "1"): "/VBAT_RAW", ("J1", "2"): "GND",
    ("U3", "7"): "/VBAT_RAW", ("U3", "5"): "/BUCK_EN",
    # External interfaces and exact seven-wire BLS-60A harness assignment.
    ("J2", "1"): "/VBAT_RAW", ("J2", "2"): "GND",
    ("J4", "1"): "GND", ("J4", "2"): "+3V3",
    ("J4", "3"): "/I2C_SCL", ("J4", "4"): "/I2C_SDA",
    ("J5", "1"): "GND", ("J5", "2"): "+3V3",
    ("J5", "3"): "/GPS_TX", ("J5", "4"): "/GPS_RX",
    ("J6", "1"): "GND", ("J6", "2"): "+5V",
    ("J6", "3"): "/IBUS_RAW",
    ("J14", "1"): "/ESC_CUR_RAW", ("J14", "2"): "/MOTOR_FL",
    ("J14", "3"): "/MOTOR_RL", ("J14", "4"): "/MOTOR_FR",
    ("J14", "5"): "/MOTOR_RR", ("J14", "6"): "/VBAT_RAW",
    ("J14", "7"): "GND",
    # ESP32 flight-control assignments and IMU SPI interface.
    ("U1", "6"): "/BAT_ADC", ("U1", "7"): "/ESC_CURRENT_ADC",
    ("U1", "8"): "/MOTOR_RR_RAW", ("U1", "9"): "/IMU_CS",
    ("U1", "10"): "/MOTOR_FL_RAW", ("U1", "11"): "/MOTOR_RL_RAW",
    ("U1", "12"): "/IMU_INT", ("U1", "24"): "/BOOT_STATUS_LED",
    ("U1", "26"): "/MOTOR_FR_RAW", ("U1", "27"): "/IBUS_RX",
    ("U1", "28"): "/GPS_TX", ("U1", "29"): "/SPI_SCK",
    ("U1", "30"): "/SPI_MOSI", ("U1", "31"): "/SPI_MISO",
    ("U1", "33"): "/I2C_SDA", ("U1", "36"): "/I2C_SCL",
    ("U1", "37"): "/GPS_RX",
    ("U2", "1"): "+3V3", ("U2", "8"): "+3V3",
    ("U2", "13"): "+3V3", ("U2", "9"): "/SPI_MISO",
    ("U2", "22"): "/IMU_CS", ("U2", "23"): "/SPI_SCK",
    ("U2", "24"): "/SPI_MOSI", ("U2", "12"): "/IMU_INT",
    ("U2", "11"): "GND", ("U2", "18"): "GND", ("U2", "20"): "GND",
    # Motor isolation, ESC-current filtering, and service/status indicators.
    ("R10", "1"): "/MOTOR_FL", ("R10", "2"): "/MOTOR_FL_RAW",
    ("R11", "1"): "/MOTOR_FR", ("R11", "2"): "/MOTOR_FR_RAW",
    ("R12", "1"): "/MOTOR_RL", ("R12", "2"): "/MOTOR_RL_RAW",
    ("R13", "1"): "/MOTOR_RR", ("R13", "2"): "/MOTOR_RR_RAW",
    ("R21", "1"): "/ESC_CURRENT_ADC", ("R21", "2"): "/ESC_CUR_RAW",
    ("R24", "1"): "/ESC_CURRENT_ADC", ("R24", "2"): "GND",
    ("R25", "1"): "/IBUS_RX", ("R25", "2"): "/IBUS_RAW",
    ("R26", "1"): "/IBUS_RX", ("R26", "2"): "GND",
    ("C19", "1"): "/ESC_CURRENT_ADC", ("C19", "2"): "GND",
    ("D3", "1"): "GND", ("D3", "2"): "Net-(D3-A)",
    ("R22", "1"): "Net-(D3-A)", ("R22", "2"): "+3V3",
    ("D4", "1"): "GND", ("D4", "2"): "Net-(D4-A)",
    ("R23", "1"): "Net-(D4-A)", ("R23", "2"): "/BOOT_STATUS_LED",
}

errors = []
for pin, wanted in expected.items():
    got = pin_net.get(pin, "<missing>")
    if got != wanted:
        errors.append(f"{pin[0]}.{pin[1]}: expected {wanted}, got {got}")

if errors:
    print("Critical netlist audit FAILED:")
    print("\n".join(f"- {e}" for e in errors))
    raise SystemExit(1)
print(f"Critical netlist audit passed ({len(expected)} pin assertions).")
