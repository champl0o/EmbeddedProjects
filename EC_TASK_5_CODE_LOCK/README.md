# Code Lock System

A digital lock system with a 4-button code entry, LED feedback, and LCD display.

## Hardware

- **ESP32-S3 DevKitC-1** (N16R8)
- **2x Buttons** with built-in LED (VCC, GPIO, GND)
- **1602A LCD Display** (16-pin)
- **3x LEDs** (red, green, yellow)
- **Resistors:** 220Ω (for LED backlight)

## Pinout

### Buttons

- Button A: GPIO 18
- Button B: GPIO 4

### LEDs

- Red: GPIO 16
- Green: GPIO 15
- Yellow: GPIO 17

### LCD Display (4-bit mode)

| LCD Pin       | Connect to              |
| ------------- | ----------------------- |
| 1 (VSS)       | GND                     |
| 2 (VDD)       | 3.3V                    |
| 3 (V0)        | Potentiometer wiper     |
| 4 (RS)        | GPIO 5                  |
| 5 (R/W)       | GND                     |
| 6 (E)         | GPIO 6                  |
| 11-14 (D4-D7) | GPIO 7,8,9,10           |
| 15 (LED+)     | 3.3V via 100Ω resistor |
| 16 (LED-)     | GND                     |

## Default Code

`A → B → B → A`

## Usage

1. Power on the system
2. LCD shows "Enter code:"
3. Press buttons to enter code
4. LCD displays entered sequence (A/B)
5. After 4 presses:
   - **Green blink + "Code correct!"** → unlocked
   - **Red blink + "Code incorrect!"** → try again
6. Both buttons pressed → "Both pressed!" message

## Features

- Non-blocking feedback display (buttons responsive during messages)
- Software debounce (200ms stable-state detection)
- Adjustable contrast via potentiometer

## Build & Upload

```bash
platformio run
platformio run --target upload
platformio device monitor
```
