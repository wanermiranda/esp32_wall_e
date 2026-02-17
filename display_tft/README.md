# Wall-E Solar Charge Level Gauge

A recreation of the iconic chest display from Disney/Pixar's **Wall-E** robot, built for an **ESP32 WROOM** with an **ST7735 TFT** screen.

Black background with yellowish-green foreground — just like the old phosphor CRT look from the movie.

## Screen Layout

Portrait mode, 128×160 pixels:

```
┌──────────────────────────┐
│   SOLAR CHARGE LEVEL     │  ← title, full width
│──────────────────────────│
│  ☀   │ ████████████████  │  ← sun (left col) │ bars (right col)
│      │ ████████████████  │
│      │ ████████████████  │
│      │ ░░░░░░░░░░░░░░░░  │  (12 bars total)
│      │ ░░░░░░░░░░░░░░░░  │
│      │ ░░░░░░░░░░░░░░░░  │
│      │ ...               │
│      │ ░░░░░░░░░░░░░░░░  │
└──────────────────────────┘
```

## Wiring (ESP32 → ST7735)

| ESP32 Pin | TFT Pin |
| --------- | ------- |
| 5         | CS      |
| 15        | RST     |
| 32        | DC / AO |
| 23        | SDA     |
| 18        | SCK     |
| 3.3V      | VCC     |
| GND       | GND     |

## Dependencies

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
