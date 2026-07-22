import fs from "node:fs";
import { spawnSync } from "node:child_process";

const source = fs.readFileSync(new URL("../ESP32_FlightController.sch", import.meta.url), "utf8").split(/\r?\n/);
const endDescr = source.indexOf("$EndDescr");
const header = source.slice(0, endDescr + 1);
const body = source.slice(endDescr + 1, -2);
const items = [];

for (let i = 0; i < body.length;) {
  if (body[i] === "$Comp") {
    const end = body.indexOf("$EndComp", i);
    items.push(body.slice(i, end + 1));
    i = end + 1;
  } else if (/^(Text Notes|Text Label|Wire Wire Line)/.test(body[i])) {
    items.push(body.slice(i, i + 2));
    i += 2;
  } else {
    items.push([body[i]]);
    i += 1;
  }
}

for (let n = 0; n <= items.length; n++) {
  const testPath = "/tmp/esp32_fc_parse_test.sch";
  fs.writeFileSync(testPath, [...header, ...items.slice(0, n).flat(), "$EndSCHEMATC", ""].join("\n"));
  const result = spawnSync(
    "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli",
    ["sch", "export", "pdf", "--output", "/tmp/esp32_fc_parse_test.pdf", testPath],
    { encoding: "utf8" },
  );
  if (result.status !== 0) {
    console.log(JSON.stringify({ n, nearby: items.slice(Math.max(0, n - 5), n + 1), status: result.status, stdout: result.stdout, stderr: result.stderr }, null, 2));
    process.exit(1);
  }
}

console.log(`All ${items.length} items parsed.`);
