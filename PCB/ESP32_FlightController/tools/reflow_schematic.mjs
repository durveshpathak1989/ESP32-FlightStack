import fs from "node:fs";

const path = new URL("../ESP32_FlightController.sch", import.meta.url);
const lines = fs.readFileSync(path, "utf8").split(/\r?\n/);

function transform(line, dx, dy) {
  let m;
  if ((m = line.match(/^(P) (\d+) (\d+)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}`;
  if ((m = line.match(/^(F \d+ ".*?" [HV]) (\d+) (\d+)(.*)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}${m[4]}`;
  if ((m = line.match(/^(Text (?:Notes|Label)) (\d+) (\d+)(.*)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}${m[4]}`;
  if ((m = line.match(/^(NoConn ~|Connection ~) (\d+) (\d+)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}`;
  if ((m = line.match(/^(\s+)(\d+) (\d+) (\d+) (\d+)$/))) return `${m[1]}${+m[2] + dx} ${+m[3] + dy} ${+m[4] + dx} ${+m[5] + dy}`;
  if ((m = line.match(/^(\s+1\s+)(\d+) (\d+)$/))) return `${m[1]}${+m[2] + dx} ${+m[3] + dy}`;
  return line;
}

const idxU1 = lines.findIndex((l) => l === "L RF_Module:ESP32-WROOM-32E U1") - 1;
const idxPower = lines.findIndex((l) => l === "POWER AND 3S BATTERY MONITOR") - 1;
const idxImu = lines.findIndex((l) => l === "INTEGRATED MPU-9250 IMU (SPI)") - 1;
const idxEsc = lines.findIndex((l) => l === "FOUR PWM ESC OUTPUTS — SIGNAL AND GROUND ONLY") - 1;
const idxAssembly = lines.findIndex((l) => l === "ASSEMBLY / BRING-UP NOTES") - 1;
const idxLdo = lines.findIndex((l) => l === "L Regulator_Linear:AP2112K-3.3 U4") - 1;
const idxReset = lines.findIndex((l) => l === "L Device:R R8") - 1;
const idxEnd = lines.findIndex((l) => l === "$EndSCHEMATC");

if ([idxU1, idxPower, idxImu, idxEsc, idxAssembly, idxLdo, idxReset, idxEnd].some((n) => n < 0)) {
  throw new Error("Unable to locate all schematic section markers");
}

lines[4] = "$Descr A3 16535 11693";
for (let i = idxU1; i < idxPower; i++) lines[i] = transform(lines[i], 3500, 1500);
for (let i = idxImu; i < idxEsc; i++) lines[i] = transform(lines[i], 3500, 500);
for (let i = idxAssembly; i < idxLdo; i++) lines[i] = transform(lines[i], 3500, 2500);
for (let i = idxLdo; i < idxReset; i++) lines[i] = transform(lines[i], 1200, 500);
for (let i = idxReset; i < idxEnd; i++) lines[i] = transform(lines[i], 2000, 3500);

fs.writeFileSync(path, lines.join("\n"));
