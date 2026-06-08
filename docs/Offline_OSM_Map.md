# Offline OSM Map Workflow

This project can use a local `.osm` map file so the GPS map still works when the laptop is connected to the ESP32 access point and has no internet connection.

## Why this is needed

When the ground station uses online map tiles, the map may be blank while connected to `ESP32-DRONE`, because that access point usually does not provide internet. A local `.osm` file avoids that dependency.

## Recommended folder layout

```text
DroneGCS/
├── DroneGCS_Apple_OSM_offline.html
└── ColumbusMap.osm
```

Keep the `.osm` file in the same folder as the offline GCS HTML unless the HTML has been edited to point to a different file path.

## Run from a local web server

Do not double-click the HTML file directly from Windows Explorer. Browser security rules may block local file loading from `file://`.

From the GCS folder, run:

```bash
python -m http.server 8080
```

Then open:

```text
http://localhost:8080/DroneGCS_Apple_OSM_offline.html
```

With this setup:

```text
GCS HTML          -> loaded from laptop localhost
ColumbusMap.osm  -> loaded from laptop localhost
Telemetry         -> loaded from ESP32 at http://192.168.4.1/telemetry
Tune endpoint     -> http://192.168.4.1/tune
OTA endpoint      -> http://192.168.4.1/update
```

This allows the map to work while the laptop is connected to the ESP32 Wi-Fi network.

## Getting a small OSM file

Use a small exported area around the flight-test field. Avoid very large city or state extracts because browser parsing becomes slow.

Possible sources:

- OpenStreetMap export for a small bounding box
- BBBike extract service
- Geofabrik extract cropped down with Osmium or QGIS

## Notes

The offline map is intended for situational awareness and test visualization. It is not an autonomous navigation authority.
