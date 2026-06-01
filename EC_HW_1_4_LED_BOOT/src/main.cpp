#include <Arduino.h>

#define BTN_PIN 0
#define BLUE_LED_PIN 16
#define RED_LED_PIN 17
#define OUTER_BTN_PIN 18

enum class BlinkMode
{
  OFF,
  FAST,
  SLOW
};

BlinkMode blinkMode = BlinkMode::OFF;

int lastBtn = HIGH, lastOuter = HIGH;
unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_MS = 30;

unsigned long lastToggle = 0;
bool phase = false;

// void setup()
// {
//   Serial.begin(115200);
//   delay(200);
//   Serial.printf("Total Heap: %d\n", ESP.getHeapSize());
//   Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());
//   Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
//   Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
// }

void blinkLeds(int delayMs)
{
  if (delayMs == 0)
  {
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    return;
  }

  digitalWrite(BLUE_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  delay(delayMs);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(delayMs);
}

void setup()
{
  blinkMode = BlinkMode::OFF;
  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(OUTER_BTN_PIN, INPUT_PULLUP);
}

void loop()
{
  int btn = digitalRead(BTN_PIN);
  int outer = digitalRead(OUTER_BTN_PIN);

  if (millis() - lastDebounce > DEBOUNCE_MS)
  {
    if (lastBtn == HIGH && btn == LOW) // BOOT just pressed
    {
      blinkMode = BlinkMode::SLOW;
      lastDebounce = millis();
    }
    else if (lastOuter == HIGH && outer == LOW) // outer just pressed
    {
      blinkMode = BlinkMode::FAST;
      lastDebounce = millis();
    }
  }

  lastBtn = btn;
  lastOuter = outer;

  int delayMs = 0;
  switch (blinkMode)
  {
  case BlinkMode::OFF:
    delayMs = 0;
    break;
  case BlinkMode::FAST:
    delayMs = 100;
    break;
  case BlinkMode::SLOW:
    delayMs = 500;
    break;
  }

  blinkLeds(delayMs);
}
