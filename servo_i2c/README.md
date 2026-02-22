# Servo I2C — ESP32 WROOM + PCA9685 (I2C)

Servo pose-control example using an **I2C servo module (PCA9685)** with `Wire` and `Adafruit_PWMServoDriver`.

This sketch controls three logical servos (left arm, right arm, head) with incremental motion and simple pose sequencing.

## Uses I2C Module

Yes — this module uses PCA9685 over I2C.

- Library includes: `Wire.h`, `Adafruit_PWMServoDriver.h`
- Driver object: `Adafruit_PWMServoDriver pwm`

## Typical I2C Wiring (ESP32 → PCA9685)

| ESP32 GPIO | PCA9685 Pin | Function                      |
| ---------- | ----------- | ----------------------------- |
| 21         | SDA         | I2C data                      |
| 22         | SCL         | I2C clock                     |
| 3.3V/5V    | VCC         | Logic power (board-dependent) |
| GND        | GND         | Common ground                 |

Also connect servo power (`V+`) with an appropriate external supply and keep grounds common with ESP32.

## What the Sketch Does

- Initializes PCA9685 at 50 Hz
- Moves servos by logical ID (`1..16` channel mapping)
- Applies incremental angle transitions for smoother movement
- Runs pose sequence: center, arms up, center, arms down, head left/right

## Dependencies

- [Adafruit PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)
- ESP32 Arduino core

## Usage

1. Open `servo_i2c.ino`.
2. Select ESP32 board and COM port.
3. Install dependencies if needed.
4. Upload sketch and open Serial Monitor at `115200` baud.
