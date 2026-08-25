#include <Arduino.h>
#include "led.h"
#include "config.h"

Led led;
uint32_t lastButtonPressTime = 0;
bool buttonPreviousState = false;

void setup()
{
  Serial.begin(Config::SERIAL_BAUD);
  Serial.println("Hello, ESP32-S3!");

  pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
  led.init(Config::LED_PIN);

  Serial.print("LED pin: ");
  Serial.println(Config::LED_PIN);
  Serial.print("Button pin: ");
  Serial.println(Config::BUTTON_PIN);
  Serial.println("Press button to change color");
}

void loop()
{
  uint32_t now = millis();

  if (now - led.getLastBlinkTime() >= Config::BLINK_INTERVAL)
  {
    if (led.getState() == LedState::ON)
    {
      led.setState(LedState::OFF);
      Serial.println("LED OFF");
    }
    else
    {
      led.setState(LedState::ON);
      Serial.println("LED ON");
    }

    led.setLastBlinkTime(now);
  }

  bool buttonCurrentState = digitalRead(Config::BUTTON_PIN) == LOW;

  if (buttonCurrentState && !buttonPreviousState && (now - lastButtonPressTime >= Config::DEBOUNCE_DELAY))
  {
    Serial.println("Button pressed!");

    LedColor currentColor = led.getColor();
    LedColor nextColor;

    switch (currentColor)
    {
    case LedColor::RED:
      nextColor = LedColor::GREEN;
      Serial.println("Color: GREEN");
      break;
    case LedColor::GREEN:
      nextColor = LedColor::BLUE;
      Serial.println("Color: BLUE");
      break;
    case LedColor::BLUE:
      nextColor = LedColor::WHITE;
      Serial.println("Color: WHITE");
      break;
    case LedColor::WHITE:
      nextColor = LedColor::RED;
      Serial.println("Color: RED");
      break;
    }

    led.setColor(nextColor);
    lastButtonPressTime = now;
  }

  buttonPreviousState = buttonCurrentState;
}
