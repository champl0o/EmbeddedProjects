#include <Arduino.h>

#define LED_SLOW 1000
#define LED_MEDIUM 500
#define LED_FAST 200

#define LED_SLOW_PIN 15
#define LED_MEDIUM_PIN 16
#define LED_FAST_PIN 17

#define BLINK_DURATION 100

uint32_t slowBlinkedAt = 0;
uint32_t mediumBlinkedAt = 0;
uint32_t fastBlinkedAt = 0;

void setup()
{
  pinMode(LED_SLOW_PIN, OUTPUT);
  pinMode(LED_MEDIUM_PIN, OUTPUT);
  pinMode(LED_FAST_PIN, OUTPUT);
}

void blinkSlow()
{
  if (millis() - slowBlinkedAt >= LED_SLOW)
  {
    digitalWrite(LED_SLOW_PIN, !digitalRead(LED_SLOW_PIN));
    slowBlinkedAt = millis();
  }
  else if (digitalRead(LED_SLOW_PIN) == HIGH && millis() - slowBlinkedAt >= BLINK_DURATION)
  {
    digitalWrite(LED_SLOW_PIN, LOW);
  }
}

void blinkMedium()
{
  if (millis() - mediumBlinkedAt >= LED_MEDIUM)
  {
    digitalWrite(LED_MEDIUM_PIN, !digitalRead(LED_MEDIUM_PIN));
    mediumBlinkedAt = millis();
  }
  else if (digitalRead(LED_MEDIUM_PIN) == HIGH && millis() - mediumBlinkedAt >= BLINK_DURATION)
  {
    digitalWrite(LED_MEDIUM_PIN, LOW);
  }
}

void blinkFast()
{
  if (millis() - fastBlinkedAt >= LED_FAST)
  {
    digitalWrite(LED_FAST_PIN, !digitalRead(LED_FAST_PIN));
    fastBlinkedAt = millis();
  }
  else if (digitalRead(LED_FAST_PIN) == HIGH && millis() - fastBlinkedAt >= BLINK_DURATION)
  {
    digitalWrite(LED_FAST_PIN, LOW);
  }
}

void loop()
{
  blinkSlow();

  blinkMedium();

  blinkFast();
}
