# Control DC Motors — ESP32 WROOM + L298N

An Arduino sketch that drives two DC motors through an **L298N** H-bridge using **analog joystick input** (VRX / VRY) with differential-drive mixing and proportional speed control.

## Hardware Requirements

- ESP32 WROOM development board
- L298N dual H-bridge motor driver module
- 2× DC motors
- Analog joystick module (VRX, VRY, SW)
- External power supply for the motors (7–12 V recommended)
- Jumper wires

## Wiring (ESP32 → L298N)

| ESP32 GPIO | L298N Pin | Function            |
| ---------- | --------- | ------------------- |
| 14         | ENA       | Motor A speed (PWM) |
| 27         | IN1       | Motor A direction   |
| 26         | IN2       | Motor A direction   |
| 32         | ENB       | Motor B speed (PWM) |
| 25         | IN3       | Motor B direction   |
| 33         | IN4       | Motor B direction   |
| GND        | GND       | Common ground       |

## Wiring (ESP32 → Joystick)

| ESP32 GPIO | Joystick Pin | Function                 |
| ---------- | ------------ | ------------------------ |
| 34         | VRX          | Horizontal axis (analog) |
| 35         | VRY          | Vertical axis (analog)   |
| 23         | SW           | Push-button (active LOW) |
| 3.3V       | +5V / VCC    | Power                    |
| GND        | GND          | Ground                   |

> **Note:** Remove the ENA/ENB jumpers on the L298N board so that speed is controlled via PWM.

## How It Works

### Joystick Axes

Each axis accepts a value from **−255** to **+255**:

| Axis | Negative    | Positive     |
| ---- | ----------- | ------------ |
| VRX  | LEFT (−255) | RIGHT (+255) |
| VRY  | BACK (−255) | FRONT (+255) |

### Differential-Drive Mixing

The core function `joystickControl(int vrx, int vry)` combines both axes into individual motor commands:

```
leftMotorSpeed  = VRY + VRX
rightMotorSpeed = VRY − VRX
```

The **sign** of each result determines direction (forward / reverse), and the **magnitude** sets the PWM duty cycle (speed). A configurable dead-zone (default **15**) filters out small joystick drift.

### Direction Combinations

| Input          | Behaviour                                    |
| -------------- | -------------------------------------------- |
| `( 0, +255)`   | **Front** — both motors full speed forward   |
| `( 0, -255)`   | **Back** — both motors full speed reverse    |
| `(-255,  0)`   | **Spin left** — left reverse, right forward  |
| `(+255,  0)`   | **Spin right** — left forward, right reverse |
| `(+128, +255)` | **Front-right curve** — right motor slower   |
| `(-128, +255)` | **Front-left curve** — left motor slower     |
| `(+128, -200)` | **Back-right curve**                         |
| `(-128, -200)` | **Back-left curve**                          |
| `( 0,  80)`    | **Gentle forward** — proportional low speed  |

### Speed Control

Speed is **proportional** to the joystick displacement. A half-tilt on the Y axis produces roughly half the PWM duty cycle, giving smooth acceleration and fine control.

### SW Button (Emergency Stop / Toggle)

Pressing the joystick button **toggles** motor output on/off. When disabled, both motors stop immediately. Press again to re-enable. Debouncing is handled by the **ezButton** library.

## API

```cpp
void readJoystick(int &vrx, int &vry);
void joystickControl(int vrx, int vry);
```

| Function            | Description                                                      |
| ------------------- | ---------------------------------------------------------------- |
| `readJoystick()`    | Reads ADC pins, maps 0-4095 → −255…+255, inverts VRY for forward |
| `joystickControl()` | Applies dead-zone, differential-drive mix, sets motor PWM        |

| Parameter | Range       | Description                    |
| --------- | ----------- | ------------------------------ |
| `vrx`     | −255 … +255 | Horizontal axis (left / right) |
| `vry`     | −255 … +255 | Vertical axis (back / front)   |

Helper functions:

```cpp
void setMotorA(bool forward, uint8_t speed);
void setMotorB(bool forward, uint8_t speed);
void stopMotors();
```

## Main Loop

The `loop()` reads the physical joystick at ~20 Hz, maps the analog values, and drives the motors in real time. The SW button toggles motor output on/off.

## Serial Monitor

Set the baud rate to **115200** to view axis values, motor directions, and speeds in real time.

## Dependencies

- [ezButton](https://github.com/ArduinoGetStarted/button) — Debounced button library
- ESP32 Arduino core (≥ 3.x for `ledcAttachChannel` API)

Install **ezButton** via the Arduino Library Manager or PlatformIO.

## Usage

1. Open `control_dc_motors.ino` in the Arduino IDE (or PlatformIO).
2. Select your ESP32 board and the correct COM port.
3. Upload the sketch.
4. Open the Serial Monitor at 115200 baud to observe motor commands.
