#pragma once
#include "Arduino.h"

// Define pins
#define RED_PIN 16
#define GREEN_PIN 15
#define YELLOW_PIN 17

// define buttons
#define A_BUTTON_PIN 18
#define B_BUTTON_PIN 4

#define DEBOUNCE_DELAY_MS 100

// Define code

#define A 0
#define B 1

void configurePins();
