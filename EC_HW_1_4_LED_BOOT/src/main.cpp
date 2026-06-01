#include <Arduino.h>

#define BTN_PIN 0
#define BLUE_LED_PIN 16
#define RED_LED_PIN 17
#define OUTER_BTN_PIN 18

enum class BlinkMode
{
  OFF,
  FAST,
  SLOW,
  ALWAYS_ON,
};

BlinkMode blinkMode = BlinkMode::OFF;

int lastBtn = HIGH, lastOuter = HIGH;
unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_MS = 30;

unsigned long lastToggle = 0;

unsigned long delayMs = 0;

// void setup()
// {
//   Serial.begin(115200);
//   delay(200);
//   Serial.printf("Total Heap: %d\n", ESP.getHeapSize());
//   Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());
//   Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
//   Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
// }

void blinkLeds()
{
  if (blinkMode == BlinkMode::OFF)
  {
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    return;
  }

  if (blinkMode == BlinkMode::ALWAYS_ON)
  {
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    return;
  }

  if (blinkMode == BlinkMode::FAST || blinkMode == BlinkMode::SLOW)
  {
    if (blinkMode == BlinkMode::FAST)
    {
      delayMs = 100;
    }
    else if (blinkMode == BlinkMode::SLOW)
    {
      delayMs = 500;
    }

    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    delay(delayMs);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(delayMs);
  }
}

void setup()
{
  blinkMode = BlinkMode::OFF;
  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(OUTER_BTN_PIN, INPUT);
}

void loop()
{
  int btn = digitalRead(BTN_PIN);
  int outer = digitalRead(OUTER_BTN_PIN);

  Serial.printf("BTN: %d, OUTER: %d\n", btn, outer);

  bool btnPressed = (btn == LOW);
  bool outerPressed = (outer == HIGH);

  bool btnEdge = (lastBtn == HIGH && btn == LOW);
  bool outerEdge = (lastOuter == LOW && outer == HIGH);

  if (millis() - lastDebounce > DEBOUNCE_MS)
  {
    if (btnPressed && outerPressed)
    {
      blinkMode = BlinkMode::ALWAYS_ON;
      lastDebounce = millis();
    }
    else if (btnEdge)
    {
      blinkMode = BlinkMode::SLOW;
      lastDebounce = millis();
    }
    else if (outerEdge)
    {
      blinkMode = BlinkMode::FAST;
      lastDebounce = millis();
    }
  }

  Serial.printf("Blink Mode: %d\n", static_cast<int>(blinkMode));

  lastBtn = btn;
  lastOuter = outer;

  blinkLeds();
}
