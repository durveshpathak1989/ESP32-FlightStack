import fs from "node:fs";

const path = new URL("../ESP32_FlightController.sch", import.meta.url);
let source = fs.readFileSync(path, "utf8");

if (source.includes("BLS-60A 7-PIN ESC INTERFACE")) {
  console.log("BLS-60A interface already integrated");
  process.exit(0);
}

source = source
  .replace('F 1 "3S_LIPO_IN"', 'F 1 "BATTERY_PIGTAIL"')
  .replace(
    'F 2 "Connector_JST:JST_GH_BM02B-GHS-TBT_1x02-1MP_P1.25mm_Vertical" H 1050 1850 50 0001 C CNN',
    'F 2 "Connector_Wire:SolderWire-1.5sqmm_1x02_P6mm_D1.7mm_OD3mm" H 1050 1850 50 0001 C CNN'
  )
  .replace("NoConn ~ 8400 4000", "Text Label 8400 4000 0 50 ~ 0\nBOOT_STATUS_LED")
  .replace("NoConn ~ 8400 6100", "Text Label 8400 6100 0 50 ~ 0\nESC_CURRENT_ADC")
  .replace('F 1 "I2C_SENSOR"', 'F 1 "QWIIC_I2C"')
  .replace(
    'F 2 "Connector_JST:JST_GH_BM04B-GHS-TBT_1x04-1MP_P1.25mm_Vertical" H 10300 3650 50 0001 C CNN',
    'F 2 "Connector_JST:JST_SH_BM04B-SRSS-TB_1x04-1MP_P1.00mm_Vertical" H 10300 3650 50 0001 C CNN'
  );

// Remove the four alternate two-pin ESC connectors.  The production assembly
// uses one keyed seven-position harness matching the verified BLS-60A order.
const escStart = source.indexOf("$Comp\nL Connector_Generic:Conn_01x02 J10");
const escEndMarker = "Text Notes 650 6500 0 50 ~ 0\nMotor map:";
const escEnd = source.indexOf(escEndMarker, escStart);
if (escStart < 0 || escEnd < 0) throw new Error("legacy ESC connector block not found");

const esc = `$Comp
L Connector_Generic:Conn_01x07 J14
U 1 1 60000200
P 2800 5350
F 0 "J14" H 2880 5392 50 0000 L CNN
F 1 "BLS60A_ESC_7PIN" H 2880 5301 50 0000 L CNN
F 2 "Connector_JST:JST_SH_BM07B-SRSS-TB_1x07-1MP_P1.00mm_Vertical" H 2800 5350 50 0001 C CNN
\t1    2800 5350
\t-1 0 0 1
$EndComp
Text Label 3000 5050 0 50 ~ 0
ESC_CUR_RAW
Text Label 3000 5150 0 50 ~ 0
MOTOR_FL
Text Label 3000 5250 0 50 ~ 0
MOTOR_RL
Text Label 3000 5350 0 50 ~ 0
MOTOR_FR
Text Label 3000 5450 0 50 ~ 0
MOTOR_RR
Text Label 3000 5550 0 50 ~ 0
VBAT_RAW
$Comp
L power:GND #PWR0220
U 1 1 60000201
P 3200 5750
F 0 "#PWR0220" H 3200 5500 50 0001 C CNN
F 1 "GND" H 3205 5577 50 0000 C CNN
\t1    3200 5750
\t1 0 0 -1
$EndComp
Wire Wire Line
\t3000 5650 3200 5650
Wire Wire Line
\t3200 5650 3200 5750
$Comp
L Device:R R21
U 1 1 60000202
P 4000 5050
F 0 "R21" V 3793 5050 50 0000 C CNN
F 1 "1k" V 3884 5050 50 0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3930 5050 50 0001 C CNN
\t1    4000 5050
\t0 1 1 0
$EndComp
Text Label 3750 5050 2 50 ~ 0
ESC_CUR_RAW
Text Label 4250 5050 0 50 ~ 0
ESC_CURRENT_ADC
Wire Wire Line
\t3750 5050 3850 5050
Wire Wire Line
\t4150 5050 4250 5050
$Comp
L Device:C C19
U 1 1 60000203
P 4250 5350
F 0 "C19" H 4365 5396 50 0000 L CNN
F 1 "100n" H 4365 5305 50 0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4288 5200 50 0001 C CNN
\t1    4250 5350
\t1 0 0 -1
$EndComp
Wire Wire Line
\t4250 5050 4250 5200
$Comp
L power:GND #PWR0221
U 1 1 60000204
P 4250 5600
F 0 "#PWR0221" H 4250 5350 50 0001 C CNN
F 1 "GND" H 4255 5427 50 0000 C CNN
\t1    4250 5600
\t1 0 0 -1
$EndComp
Wire Wire Line
\t4250 5500 4250 5600
Text Notes 2450 4600 0 80 ~ 16
BLS-60A 7-PIN ESC INTERFACE
Text Notes 2450 4750 0 45 ~ 0
J14 pin order at FC: 1 CUR, 2 M4/FL, 3 M3/RL, 4 M2/FR, 5 M1/RR, 6 BAT, 7 GND.
Text Notes 2450 5950 0 45 ~ 0
BAT is a low-current FC supply/sense feed. XT60 and motor current terminate on the ESC, never through this PCB.
$Comp
L Device:R R22
U 1 1 60000210
P 5250 5050
F 0 "R22" V 5043 5050 50 0000 C CNN
F 1 "2.2k" V 5134 5050 50 0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5180 5050 50 0001 C CNN
\t1    5250 5050
\t0 1 1 0
$EndComp
$Comp
L Device:LED D3
U 1 1 60000211
P 5650 5050
F 0 "D3" H 5643 4795 50 0000 C CNN
F 1 "GREEN_POWER" H 5643 4886 50 0000 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 5650 5050 50 0001 C CNN
\t1    5650 5050
\t-1 0 0 1
$EndComp
$Comp
L power:+3V3 #PWR0222
U 1 1 60000212
P 5000 4950
F 0 "#PWR0222" H 5000 4800 50 0001 C CNN
F 1 "+3V3" H 5015 5123 50 0000 C CNN
\t1    5000 4950
\t1 0 0 -1
$EndComp
Wire Wire Line
\t5000 4950 5000 5050
Wire Wire Line
\t5000 5050 5100 5050
Wire Wire Line
\t5400 5050 5500 5050
$Comp
L power:GND #PWR0223
U 1 1 60000213
P 5900 5150
F 0 "#PWR0223" H 5900 4900 50 0001 C CNN
F 1 "GND" H 5905 4977 50 0000 C CNN
\t1    5900 5150
\t1 0 0 -1
$EndComp
Wire Wire Line
\t5800 5050 5900 5050
Wire Wire Line
\t5900 5050 5900 5150
$Comp
L Device:R R23
U 1 1 60000214
P 5250 5550
F 0 "R23" V 5043 5550 50 0000 C CNN
F 1 "2.2k" V 5134 5550 50 0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5180 5550 50 0001 C CNN
\t1    5250 5550
\t0 1 1 0
$EndComp
$Comp
L Device:LED D4
U 1 1 60000215
P 5650 5550
F 0 "D4" H 5643 5295 50 0000 C CNN
F 1 "BLUE_STATUS" H 5643 5386 50 0000 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 5650 5550 50 0001 C CNN
\t1    5650 5550
\t-1 0 0 1
$EndComp
Text Label 5000 5550 2 50 ~ 0
BOOT_STATUS_LED
Wire Wire Line
\t5000 5550 5100 5550
Wire Wire Line
\t5400 5550 5500 5550
$Comp
L power:GND #PWR0224
U 1 1 60000216
P 5900 5650
F 0 "#PWR0224" H 5900 5400 50 0001 C CNN
F 1 "GND" H 5905 5477 50 0000 C CNN
\t1    5900 5650
\t1 0 0 -1
$EndComp
Wire Wire Line
\t5800 5550 5900 5550
Wire Wire Line
\t5900 5550 5900 5650
Text Notes 4900 4750 0 80 ~ 16
BOARD INDICATORS
Text Notes 4900 5850 0 45 ~ 0
D3 = 3V3 power. D4 = GPIO2 boot/status; GPIO2 remains low-compatible during reset/download.
`;

source = source.slice(0, escStart) + esc + source.slice(escEnd);
source = source.replace('Rev "1.1"', 'Rev "1.2"');
source = source.replace(
  'Comment3 "Protected USB/battery mux, 3S-to-5V buck and 3.3V LDO"',
  'Comment3 "BLS-60A 7-pin ESC, protected USB/battery mux, onboard power"'
);

fs.writeFileSync(path, source);
console.log("Integrated BLS-60A harness, two-point battery pigtail, and current filter");
