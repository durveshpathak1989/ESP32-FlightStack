import fs from "node:fs";

const path = new URL("../ESP32_FlightController.sch", import.meta.url);
const lines = fs.readFileSync(path, "utf8").split(/\r?\n/);
const start = lines.indexOf("USB-C PROGRAMMING AND PROTECTED POWER MUX") - 1;
const end = lines.indexOf("ASSEMBLY / BRING-UP NOTES") - 1;
if (start < 0 || end <= start) throw new Error("USB section markers not found");

function transform(line, dx, dy) {
  let m;
  if ((m = line.match(/^(P) (\d+) (\d+)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}`;
  if ((m = line.match(/^(F \d+ ".*?" [HV]) (\d+) (\d+)(.*)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}${m[4]}`;
  if ((m = line.match(/^(Text (?:Notes|Label)) (\d+) (\d+)(.*)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}${m[4]}`;
  if ((m = line.match(/^(NoConn ~|Connection ~) (\d+) (\d+)$/))) return `${m[1]} ${+m[2] + dx} ${+m[3] + dy}`;
  if ((m = line.match(/^(Wire Wire Line)$/))) return line;
  if ((m = line.match(/^(\s+)(\d+) (\d+) (\d+) (\d+)$/))) return `${m[1]}${+m[2] + dx} ${+m[3] + dy} ${+m[4] + dx} ${+m[5] + dy}`;
  if ((m = line.match(/^(\s+1\s+)(\d+) (\d+)$/))) return `${m[1]}${+m[2] + dx} ${+m[3] + dy}`;
  return line;
}

// The integration script initially places this block at x=10000.  On the
// first run move it to the lower-left; later runs only nudge a block that is
// already in the left column below the ESC section.
const header = lines[start].match(/^Text Notes (\d+) (\d+)/);
const alreadyLeft = header && +header[1] < 6000;
const dx = alreadyLeft ? 0 : -9300;
const dy = alreadyLeft ? 900 : 6100;
for (let i = start; i < end; i++) lines[i] = transform(lines[i], dx, dy);
fs.writeFileSync(path, lines.join("\n"));
console.log("Moved USB/power section into the lower-left schematic region");
