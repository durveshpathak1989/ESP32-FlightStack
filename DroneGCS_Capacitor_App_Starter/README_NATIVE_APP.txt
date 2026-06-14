# Native App Starter for Drone GCS

This is a Capacitor wrapper starter. It packages the mobile GCS files as a native Android/iOS app shell.

Commands:
1. Install Node.js.
2. In this folder: `npm install`
3. Android: `npm run android`
4. iOS on Mac with Xcode: `npm run ios`

Notes:
- `server.cleartext=true` and `allowNavigation` are set so the app can call the ESP32 at `http://192.168.4.1`.
- You still need Android Studio or Xcode to build/sign/install the native app.
