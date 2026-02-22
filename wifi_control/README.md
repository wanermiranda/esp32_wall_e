# WiFi Control (ESP32 WROOM, Hotspot Mode)

Simple HTTP remote for ESP32 that serves a web page with directional controls for:

- Robot motion
- Head
- Left arm
- Right arm

This sketch does **not** drive hardware modules directly. It only logs compatible command intents to `Serial`.

## Project structure

- `wifi_control.ino` → HTTP routes and command mapping
- `wifi_ap.cpp/.h` → AP (hotspot) setup and SSID exposure
- `web_ui.cpp/.h` → Embedded HTML/CSS/JS UI page
- `preview.html` → Browser-only local preview of the UI

## Design decision: embedded HTML source module

For this project, the control page is embedded in firmware as C++ source (now in `web_ui.cpp`).

Reason:

- We are intentionally avoiding a SPIFFS-based split HTML approach because SPIFFS is no longer actively maintained.
- Keeping the UI embedded in source avoids extra filesystem upload/mount steps and keeps deployment simple for this hotspot-only controller.

If the UI grows later, we can migrate to a maintained filesystem approach (for example LittleFS) and serve `index.html` as a separate file.

## Hotspot setup

1. Upload `wifi_control.ino` to your ESP32.
2. Open Serial Monitor at `115200`.
3. Connect your phone/computer to Wi-Fi SSID `ESP32-Robot-Control` with password `robot1234`.
4. Open the AP IP shown in Serial Monitor (default is usually `192.168.4.1`).

No external Wi-Fi credentials are required in this mode.

## HTTP endpoints

- `GET /` → Control page
- `GET /cmd?target=<...>&action=<...>` → Sends command (and logs to Serial)
- `GET /status` → Last accepted command

## UI behavior

- Motion buttons use **press-and-hold** behavior (release sends `stop`).
- Motion includes diagonals (`↖ ↗ ↙ ↘`).
- Motion speed is controlled by slider (`60..255`, default `185`) and sent as `speed` query parameter.
- Left/Right arm cards are displayed as two columns, including on small screens.

## Command mapping (v2-compatible intent)

### Motion

- `motion + forward` → `MOTION FORWARD -> driveTank(185,185)`
- `motion + backward` → `MOTION BACKWARD -> driveTank(-185,-185)`
- `motion + left` → `MOTION LEFT -> driveTank(-165,165)`
- `motion + right` → `MOTION RIGHT -> driveTank(165,-165)`
- `motion + forward_left` → `MOTION FORWARD_LEFT -> driveTank(speed/2,speed)`
- `motion + forward_right` → `MOTION FORWARD_RIGHT -> driveTank(speed,speed/2)`
- `motion + backward_left` → `MOTION BACKWARD_LEFT -> driveTank(-speed/2,-speed)`
- `motion + backward_right` → `MOTION BACKWARD_RIGHT -> driveTank(-speed,-speed/2)`
- `motion + stop` → `MOTION STOP -> stopMotors()`

### Head

- `head + left` → `SERVO HEAD -> 0`
- `head + center` → `SERVO HEAD -> 90`
- `head + right` → `SERVO HEAD -> 180`

### Arms

- `left_arm + up` → `SERVO LEFT_ARM -> 120`
- `left_arm + center` → `SERVO LEFT_ARM -> 60`
- `left_arm + down` → `SERVO LEFT_ARM -> 0`
- `right_arm + up` → `SERVO RIGHT_ARM -> 120`
- `right_arm + center`→ `SERVO RIGHT_ARM -> 60`
- `right_arm + down` → `SERVO RIGHT_ARM -> 0`
