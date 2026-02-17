# Robot Main — ESP32 WROOM

Combined sketch: **joystick-controlled DC motors** (L298N) + **Wall-E Solar Charge Level gauge** (ST7735 TFT) running together on a single ESP32 WROOM.

## Hardware Requirements

- ESP32 WROOM development board
- L298N dual H-bridge motor driver
- 2× DC motors
- Analog joystick module (VRX, VRY, SW)
- ST7735 128×160 TFT display
- External power supply for the motors (7–12 V)
- Jumper wires

## Pin Map

| GPIO | Function             | Peripheral |
| ---- | -------------------- | ---------- |
| 2    | TFT DC / AO          | ST7735     |
| 4    | Joystick SW (button)  | Joystick   |
| 5    | TFT CS               | ST7735     |
| 13   | TFT SDA / MOSI       | ST7735     |
| 14   | ENA (Motor A PWM)    | L298N      |
| 15   | TFT RST              | ST7735     |
| 18   | TFT SCK              | ST7735     |
| 25   | IN3 (Motor B dir)    | L298N      |
| 26   | IN2 (Motor A dir)    | L298N      |
| 27   | IN1 (Motor A dir)    | L298N      |
| 32   | ENB (Motor B PWM)    | L298N      |
| 33   | IN4 (Motor B dir)    | L298N      |
| 34   | VRX (horizontal)     | Joystick   |
| 35   | VRY (vertical)       | Joystick   |

## Wiring

### ESP32 → L298N

| GPIO | L298N | Function            |
| ---- | ----- | ------------------- |
| 14   | ENA   | Motor A speed (PWM) |
| 27   | IN1   | Motor A direction   |
| 26   | IN2   | Motor A direction   |
| 32   | ENB   | Motor B speed (PWM) |
| 25   | IN3   | Motor B direction   |
| 33   | IN4   | Motor B direction   |
| GND  | GND   | Common ground       |

### ESP32 → Joystick

| GPIO | Joystick | Function                 |
| ---- | -------- | ------------------------ |
| 34   | VRX      | Horizontal axis (analog) |
| 35   | VRY      | Vertical axis (analog)   |
| 4    | SW       | Push-button (active LOW) |
| 3.3V | VCC      | Power                    |
| GND  | GND      | Ground                   |

### ESP32 → ST7735 TFT

| GPIO | TFT Pin | Function     |
| ---- | ------- | ------------ |
| 5    | CS      | Chip select  |
| 15   | RST     | Reset        |
| 2    | DC / AO | Data/Command |
| 13   | SDA     | MOSI (data)  |
| 18   | SCK     | Clock        |
| 3.3V | VCC     | Power        |
| GND  | GND     | Ground       |

## How It Works

### Motor Control

The joystick (VRX/VRY) feeds a **differential-drive mixer** that controls both motors with proportional speed. A dead-zone (15) filters drift. The SW button toggles motors on/off.

### TFT Display

The Wall-E Solar Charge Level gauge animates continuously — bars fill bottom-to-top and drain back down. The display updates independently of motor control using non-blocking `millis()` timing.

### Main Loop (~20 Hz)

Each iteration:

1. `button.loop()` — debounced SW button check
2. Read joystick → drive motors (if enabled)
3. `updateCharge()` — step the gauge animation if enough time has passed

## Dependencies

- [ezButton](https://github.com/ArduinoGetStarted/button) — Debounced button library
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
- ESP32 Arduino core (≥ 3.x)

## Usage

1. Open `robot_main.ino` in the Arduino IDE (or PlatformIO).
2. Select your ESP32 board and the correct COM port.
3. Upload the sketch.
4. Open the Serial Monitor at **115200** baud.
5. Leave the joystick centred during startup (calibration).
