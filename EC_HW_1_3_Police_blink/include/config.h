#pragma once
#include <Arduino.h>

// ====== pins ======
#define SDA_PIN 20
#define SCL_PIN 21

constexpr int RED_PINS[]  = {18, 13};
constexpr int BLUE_PINS[] = {17, 16};
constexpr int N = sizeof(RED_PINS) / sizeof(RED_PINS[0]); // = 2
constexpr int CHANNELS = 2 * N;                           // reds + blues

// ====== PWM config ======
constexpr int PWM_FREQ = 5000; // 5 kHz — flicker invisible
constexpr int PWM_RES  = 8;    // 8 bits → duty 0..255

// ====== timings ======
constexpr uint16_t BLINK_MS         = 400;
constexpr uint16_t CAR_FRAME_MS     = 30;   // ~33 fps
constexpr uint32_t PATTERN_CYCLE_MS = 6000; // switch pattern every 6 s
