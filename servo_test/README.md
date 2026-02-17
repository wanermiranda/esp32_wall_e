# Servo Test - ESP32 WROOM

A simple Arduino sketch to test controlling a servo motor using an ESP32 WROOM board with the `ESP32Servo` library.

## Hardware Requirements

- ESP32 WROOM development board
- Servo motor (e.g. SG90 or MG996R)
- Jumper wires

## Wiring

| Component   | ESP32 GPIO |
|-------------|------------|
| Servo signal | GPIO 26   |
| Output pin 1 | GPIO 18   |
| Output pin 2 | GPIO 25   |

## How It Works

1. **Setup** – The servo is attached to **GPIO 26** and moved to its initial position. GPIOs **18** and **25** are configured as digital outputs.
2. **Loop** – The sketch iterates through the two output pins. For each pin:
   - The pin is set **HIGH**.
   - The servo sweeps **180 steps** in the current direction (forward or reverse).
   - The pin is set **LOW**.
   - The sweep direction is inverted for the next pin.

The `moveServo()` helper moves the servo one degree at a time with a **10 ms** delay between steps, printing the current position and direction to the Serial monitor.

## Dependencies

- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) – Servo library for ESP32

Install it via the Arduino Library Manager or PlatformIO.

## Serial Monitor

Set the baud rate to **9600** to view servo position and pin state logs.

## Usage

1. Open `servo_test.ino` in the Arduino IDE (or PlatformIO).
2. Select your ESP32 board and the correct COM port.
3. Upload the sketch.
4. Open the Serial Monitor at 9600 baud to observe the output.
