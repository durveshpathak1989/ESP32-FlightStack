import fs from "node:fs";

const path = new URL("../ESP32_FlightController.sch", import.meta.url);
let source = fs.readFileSync(path, "utf8");

if (source.includes("USB-C PROGRAMMING AND PROTECTED POWER MUX")) {
  console.log("USB/power section already integrated");
  process.exit(0);
}

// The TPS5430 output is a mux input, not the system-wide +5 V rail.
source = source.replace(
`$Comp
L power:+5V #PWR0161
U 1 1 60000065
P 4100 1700
F 0 "#PWR0161" H 4100 1550 50 0001 C CNN
F 1 "+5V" H 4115 1873 50 0000 C CNN
	1    4100 1700
	1 0 0 -1
$EndComp
Wire Wire Line
	4100 1700 4100 1900`,
`Text Label 4100 1700 1 50 ~ 0
+5V_BUCK
Wire Wire Line
	4100 1700 4100 1900`);

const marker = "Text Notes 10000 8200 0 80 ~ 16\nASSEMBLY / BRING-UP NOTES";
const usb = `Text Notes 10000 1350 0 80 ~ 16
USB-C PROGRAMMING AND PROTECTED POWER MUX
Text Notes 10000 1500 0 45 ~ 0
USB VBUS and battery-derived 5 V are isolated by U7; simultaneous connection is permitted.
$Comp
L Connector:USB_C_Receptacle_USB2.0_16P J9
U 1 1 60000100
P 10600 2600
F 0 "J9" H 10707 3467 50 0000 C CNN
F 1 "USB_C_PROGRAM" H 10707 3376 50 0000 C CNN
F 2 "Connector_USB:USB_C_Receptacle_GCT_USB4105-xx-A_16P_TopMnt_Horizontal" H 10750 2600 50 0001 C CNN
	1    10600 2600
	-1 0 0 1
$EndComp
Text Label 11200 2000 0 50 ~ 0
USB_VBUS_RAW
Text Label 11200 2100 0 50 ~ 0
USB_CC1
Text Label 11200 2200 0 50 ~ 0
USB_CC2
Text Label 11200 2500 0 50 ~ 0
USB_CONN_DM
Text Label 11200 2600 0 50 ~ 0
USB_CONN_DP
NoConn ~ 11200 3000
NoConn ~ 11200 3100
$Comp
L power:GND #PWR0200
U 1 1 60000101
P 10600 3600
F 0 "#PWR0200" H 10600 3350 50 0001 C CNN
F 1 "GND" H 10605 3427 50 0000 C CNN
	1    10600 3600
	1 0 0 -1
$EndComp
$Comp
L Device:R R15
U 1 1 60000102
P 11500 2250
F 0 "R15" H 11570 2296 50 0000 L CNN
F 1 "5.1k" H 11570 2205 50 0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 11430 2250 50 0001 C CNN
	1    11500 2250
	1 0 0 -1
$EndComp
Text Label 11500 2050 1 50 ~ 0
USB_CC1
$Comp
L Device:R R16
U 1 1 60000103
P 11900 2250
F 0 "R16" H 11970 2296 50 0000 L CNN
F 1 "5.1k" H 11970 2205 50 0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 11830 2250 50 0001 C CNN
	1    11900 2250
	1 0 0 -1
$EndComp
Text Label 11900 2050 1 50 ~ 0
USB_CC2
$Comp
L power:GND #PWR0201
U 1 1 60000104
P 11700 2500
F 0 "#PWR0201" H 11700 2250 50 0001 C CNN
F 1 "GND" H 11705 2327 50 0000 C CNN
	1    11700 2500
	1 0 0 -1
$EndComp
Wire Wire Line
	11500 2400 11500 2450
Wire Wire Line
	11500 2450 11900 2450
Wire Wire Line
	11900 2450 11900 2400
Wire Wire Line
	11700 2450 11700 2500
$Comp
L Power_Protection:USBLC6-2SC6 U5
U 1 1 60000105
P 12200 2850
F 0 "U5" H 12200 3317 50 0000 C CNN
F 1 "USBLC6-2SC6" H 12200 3226 50 0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 12200 2350 50 0001 C CNN
	1    12200 2850
	1 0 0 -1
$EndComp
Text Label 12000 2850 2 50 ~ 0
USB_CONN_DP
Text Label 12000 2950 2 50 ~ 0
USB_CONN_DM
Text Label 12400 2850 0 50 ~ 0
USB_DP
Text Label 12400 2950 0 50 ~ 0
USB_DM
Text Label 12200 2650 1 50 ~ 0
USB_VBUS
$Comp
L power:GND #PWR0202
U 1 1 60000106
P 12200 3250
F 0 "#PWR0202" H 12200 3000 50 0001 C CNN
F 1 "GND" H 12205 3077 50 0000 C CNN
	1    12200 3250
	1 0 0 -1
$EndComp
$Comp
L Interface_USB:CP2102N-Axx-xQFN24 U6
U 1 1 60000107
P 13900 2850
F 0 "U6" H 13900 4031 50 0000 C CNN
F 1 "CP2102N-A02-GQFN24" H 13900 3940 50 0000 C CNN
F 2 "Package_DFN_QFN:QFN-24-1EP_4x4mm_P0.5mm_EP2.7x2.7mm" H 14350 1650 50 0001 C CNN
	1    13900 2850
	1 0 0 -1
$EndComp
Text Label 13400 2650 2 50 ~ 0
USB_VBUS
Text Label 13400 2750 2 50 ~ 0
USB_DP
Text Label 13400 2850 2 50 ~ 0
USB_DM
Text Label 14400 1950 0 50 ~ 0
UART0_RX
Text Label 14400 2050 0 50 ~ 0
UART0_TX
Text Label 14400 2450 0 50 ~ 0
AUTO_DTR
Text Label 14400 2350 0 50 ~ 0
AUTO_RTS
$Comp
L ESP32_FlightController-cache:TPS2113A U7
U 1 1 60000108
P 11700 4050
F 0 "U7" H 11700 4465 50 0000 C CNN
F 1 "TPS2113APW" H 11700 4374 50 0000 C CNN
F 2 "Package_SO:TSSOP-8_3x3mm_P0.65mm" H 11700 3600 50 0001 C CNN
	1    11700 4050
	1 0 0 -1
$EndComp
Text Label 11200 3850 2 50 ~ 0
+5V_BUCK
Text Label 11200 3950 2 50 ~ 0
USB_VBUS
Text Label 12200 4050 0 50 ~ 0
+5V
Text Label 11200 4050 2 50 ~ 0
+5V_BUCK
Text Label 11200 4150 2 50 ~ 0
+5V_BUCK
$Comp
L Device:R R17
U 1 1 60000109
P 11700 4650
F 0 "R17" H 11770 4696 50 0000 L CNN
F 1 "499R (1A LIMIT)" H 11770 4605 50 0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 11630 4650 50 0001 C CNN
	1    11700 4650
	1 0 0 -1
$EndComp
$Comp
L power:GND #PWR0203
U 1 1 6000010A
P 11550 4950
F 0 "#PWR0203" H 11550 4700 50 0001 C CNN
F 1 "GND" H 11555 4777 50 0000 C CNN
	1    11550 4950
	1 0 0 -1
$EndComp
Wire Wire Line
	11700 4450 11700 4500
Wire Wire Line
	11700 4800 11700 4900
Wire Wire Line
	11550 4550 11550 4950
Text Notes 10000 5200 0 45 ~ 0
U7 IN1 = battery buck, IN2 = protected USB. Reverse/cross conduction are blocked internally.
`;

if (!source.includes(marker)) throw new Error("assembly marker not found");
source = source.replace(marker, usb + marker);
source = source.replace('Rev "1.0"', 'Rev "1.1"');
source = source.replace('Comment3 "Onboard 3S-to-5V buck and 3.3V LDO"', 'Comment3 "Protected USB/battery mux, 3S-to-5V buck and 3.3V LDO"');
fs.writeFileSync(path, source);
console.log("Integrated USB-C and protected dual-source power section");
