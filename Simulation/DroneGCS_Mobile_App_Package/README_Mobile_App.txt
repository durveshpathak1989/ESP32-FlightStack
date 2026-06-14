# Drone GCS Mobile App Package

Files included:
- `DroneGCS_Mobile_App.html` — mobile-first GCS dashboard/PID tuning app.
- `TestScreen_Mobile_App.html` — mobile-first test screen.
- `manifest.webmanifest`, `sw.js`, icons — installable PWA support.

How to use:
1. Put all files in the same folder.
2. Serve the folder using a local server, for example: `python3 -m http.server 8080`.
3. Open `http://localhost:8080/DroneGCS_Mobile_App.html` on your phone/computer.
4. Connect the phone to the drone Wi-Fi AP, then press Wi-Fi in the app.
5. Use browser menu → Add to Home Screen.

Important note:
Browsers require HTTPS or localhost for full service-worker PWA installation. Direct `file://` opening will still display the app, but install/offline caching may not work. If you host from the ESP32 at `http://192.168.4.1`, it may behave like a mobile web shortcut rather than a full PWA depending on browser.
