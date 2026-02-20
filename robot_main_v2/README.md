# Robot Main v2 — ESP32 WROOM

Integrated robot sketch for ESP32 WROOM combining:

- DC motor control (L298N)
- Autonomous drive state machine
- TFT Wall-E style solar charge gauge (ST7735)
- Servo IOC control via PCA9685 (head + arms)
- Wi-Fi AP + web control UI (motion, servos, mode toggles)

## Hardware Requirements

- ESP32 WROOM development board
- L298N dual H-bridge motor driver
- 2× DC motors
- ST7735 128×160 TFT display
- PCA9685 servo driver (I2C)
- 3× servos (left arm, right arm, head)
- External power supply for motors/servos
- Jumper wires

## Pin Map

| GPIO | Function          | Peripheral |
| ---- | ----------------- | ---------- |
| 14   | ENA (Motor A PWM) | L298N      |
| 27   | IN1 (Motor A dir) | L298N      |
| 26   | IN2 (Motor A dir) | L298N      |
| 32   | ENB (Motor B PWM) | L298N      |
| 25   | IN3 (Motor B dir) | L298N      |
| 33   | IN4 (Motor B dir) | L298N      |
| 5    | TFT CS            | ST7735     |
| 15   | TFT RST           | ST7735     |
| 2    | TFT DC / AO       | ST7735     |
| 13   | TFT SDA / MOSI    | ST7735     |
| 18   | TFT SCK           | ST7735     |
| 21   | I2C SDA           | PCA9685    |
| 22   | I2C SCL           | PCA9685    |

## Project Structure

- `robot_main_v2.ino` → setup/loop + HTTP routing + command arbitration
- `motor_control.cpp/.h` → low-level motor control + tank drive
- `autonomous_drive.cpp/.h` → non-blocking autonomous sequence
- `display_gauge.cpp/.h` → Wall-E charge gauge rendering + animation
- `servo_ioc_module.cpp/.h` → PCA9685 servo control + auto-pose logic
- `wifi_ap.cpp/.h` → access point setup
- `web_ui.cpp/.h` → embedded HTML/CSS/JS control page
- `preview.html` → local browser-only UI preview
- `robot_constants.h` → centralized pins, timings, and constants

## Wi-Fi Control

On boot, the ESP32 creates an AP:

- SSID: `ESP32-Robot-Control`
- Password: `robot1234`
- Open from browser: typically `http://192.168.4.1`

### HTTP Endpoints

- `GET /` → control page
- `GET /cmd?target=<...>&action=<...>&speed=<0..255>` → execute command
- `GET /status` → returns current state summary

## Command Coherence (Manual vs Auto)

The sketch prevents control conflicts by explicit mode arbitration:

- Any valid `motion` command disables autonomous drive and directly controls motors.
- Any valid servo command (`head`, `left_arm`, `right_arm`) disables servo auto-pose and applies manual targets.
- System commands can re-enable/disable background behaviors:
  - `target=system&action=autonomous_on`
  - `target=system&action=autonomous_off`
  - `target=system&action=pose_on`
  - `target=system&action=pose_off`

## Web UI Controls

The web UI includes:

- 8-direction motion + stop (press/hold behavior)
- Speed slider (`60..255`)
- Head controls (left/center/right)
- Left and right arm controls (up/center/down)
- Mode buttons (Autonomous ON/OFF, Pose ON/OFF)
- Live status refresh (polls `/status` every second)

## Dependencies

- ESP32 Arduino core (3.x recommended)
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
- [Adafruit PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)

## Usage

1. Open `robot_main_v2.ino`.
2. Select your ESP32 board and COM port.
3. Install dependencies if needed.
4. Upload the sketch.
5. Open Serial Monitor at `115200` baud and note AP IP.
6. Connect device to AP and open the robot control page.

## Local UI Preview

To preview UI without hardware/backend:

- Open `preview.html` directly in your browser.
- This page uses local mock logic for interaction/state display.

## Troubleshooting

### AP / Connectivity

- If the AP is not visible, reset the ESP32 and check Serial Monitor (`115200`) for AP startup logs.
- If you connect to the AP but page does not load, open the exact IP printed in Serial (usually `192.168.4.1`).
- If command responses are slow, reduce distance/interference and avoid multiple clients polling heavily.

### Web Commands Not Moving Robot

- Ensure motion commands are sent from `/` UI and verify `[WEB CMD]` logs in Serial.
- If autonomous behavior resumes unexpectedly, use `Modes → Autonomous OFF` to force manual drive.
- Use `GET /status` to confirm `auto_drive=off` when manually controlling motion.

### Servo Behavior Issues

- If manual head/arm commands get overridden, set `Pose OFF` in the UI (`auto_pose=off`).
- If servos jitter or reset, power servos from a stable external supply and share GND with ESP32.
- Confirm PCA9685 I2C wiring: SDA=`GPIO21`, SCL=`GPIO22`.

### Motor Power / Driver Issues

- If motors do not move but commands are received, verify L298N motor supply and common ground.
- Check ENA/ENB PWM pins (`GPIO14`, `GPIO32`) and direction pins (`GPIO27/26/25/33`) against wiring.
- If one wheel spins opposite to expected direction, swap that motor’s output leads or adjust wiring.
