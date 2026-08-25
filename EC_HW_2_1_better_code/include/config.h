#pragma once

#include <Arduino.h>

class Config
{
public:
    static constexpr uint32_t SERIAL_BAUD = 115200;

    static constexpr int LED_PIN = BUILTIN_LED;
    static constexpr int BUTTON_PIN = 0;
    static constexpr uint32_t DEBOUNCE_DELAY = 50;

    // Blink timing
    static constexpr uint32_t BLINK_INTERVAL = 500; // ON/OFF every 500ms
};
