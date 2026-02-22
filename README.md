# ESP32 Robot Modules and Examples

This repository contains a collection of modular sketches and simple examples for building a robot using an **ESP32 WROOM Dev Kit** and common robotics components.

It is organized as learning steps: from isolated component tests (motors, servos, display, Wi-Fi control) to integrated robot sketches.

## What This Repository Includes

- DC motor control examples (L298N + differential/tank drive)
- Servo control examples (direct and PCA9685-based)
- TFT display example (Wall-E style charge gauge)
- Wi-Fi access-point + browser control UI example
- Integrated robot applications (`robot_main_v1` and `robot_main_v2`)

## Folder Overview

- `dc_motors/` — basic DC motor control sketch.
- `control_dc_motors/` — DC motor control example using an L298N driver and differential-drive mixing.
- `servo_test/` — quick single-servo movement test using direct PWM (`ESP32Servo`, non-I2C).
- `servo_i2c/` — servo pose control using PCA9685 over I2C (`Wire` + `Adafruit_PWMServoDriver`).
- `display_tft/` — ST7735 TFT gauge rendering/animation example.
- `wifi_control/` — ESP32 AP + HTTP control page for robot commands.
- `robot_main_v1/` — integrated robot sketch (earlier version).
- `robot_main_v2/` — modular integrated robot sketch with autonomous/manual mode arbitration, web control, display, and servo modules.

## Typical Hardware Used

- ESP32 WROOM development board
- L298N dual H-bridge motor driver
- 2x DC motors (or geared motors)
- Servo motors (head/arms or other joints)
- PCA9685 PWM servo driver (I2C)
- ST7735 TFT display (SPI)
- External motor/servo power supply
- Jumper wires and common ground wiring

## Servo Modules and I2C Usage

- `servo_test/` → direct servo control from ESP32 PWM pin (`ESP32Servo`), does **not** use I2C.
- `servo_i2c/` → uses PCA9685 servo driver over I2C.
- `robot_main_v2/servo_ioc_module.*` → also uses PCA9685 over I2C for head/arm servos.

## Goal

Use these examples as building blocks to assemble your own robot incrementally:

1. Validate each subsystem independently (motors, servos, display, Wi-Fi).
2. Confirm wiring/power stability.
3. Integrate modules into a full robot behavior sketch.

## Notes

- Each folder is intended to be opened/uploaded as its own Arduino sketch.
- Check the README inside each module folder for pin maps and usage details where available.
