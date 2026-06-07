#pragma once
#include "Arduino.h"

// Define pins
#define RED_PIN 16
#define GREEN_PIN 15
#define YELLOW_PIN 17

// define buttons
#define A_BUTTON_PIN 18
#define B_BUTTON_PIN 4

#define DEBOUNCE_DELAY_MS 200

// Define code

#define A 0
#define B 1

// LCD config
// LCD 1602A pins (4-bit mode)
#define LCD_RS 5
#define LCD_E 6
#define LCD_D4 7
#define LCD_D5 8
#define LCD_D6 9
#define LCD_D7 10

void configurePins();
