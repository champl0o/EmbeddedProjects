#pragma once
#include <Arduino.h>

enum class LedState
{
    OFF,
    ON
};

enum class LedColor : uint8_t
{
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    WHITE = 3
};

class Led
{
public:
    void init(int pin)
    {
        this->pin = pin;
        pinMode(pin, OUTPUT);
        state = LedState::OFF;
        color = LedColor::RED;
        lastBlinkTime = millis();
        rgbLedWrite(pin, 0, 0, 0);
    }

    void setState(LedState newState)
    {
        state = newState;
        if (state == LedState::ON)
        {
            updateRGBColor();
        }
        else
        {
            rgbLedWrite(pin, 0, 0, 0);
        }
    }

    LedState getState() const
    {
        return state;
    }

    void setColor(LedColor newColor)
    {
        color = newColor;
        if (state == LedState::ON)
        {
            updateRGBColor();
        }
    }

    LedColor getColor() const
    {
        return color;
    }

    void setLastBlinkTime(uint32_t time)
    {
        lastBlinkTime = time;
    }

    uint32_t getLastBlinkTime() const
    {
        return lastBlinkTime;
    }

private:
    uint8_t pin;
    LedState state;
    LedColor color;
    uint32_t lastBlinkTime;

    void updateRGBColor()
    {
        switch (color)
        {
        case LedColor::RED:
            rgbLedWrite(pin, 255, 0, 0);
            break;
        case LedColor::GREEN:
            rgbLedWrite(pin, 0, 255, 0);
            break;
        case LedColor::BLUE:
            rgbLedWrite(pin, 0, 0, 255);
            break;
        case LedColor::WHITE:
            rgbLedWrite(pin, 255, 255, 255);
            break;
        }
    }
};
