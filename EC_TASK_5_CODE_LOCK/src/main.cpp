#include <Arduino.h>
#include "config.h"
#include "logger.h"

void blinkLED(int pin, int times, int delayMs);
void handleButtonPress(int aPressedEdge, int bPressedEdge);
void verifyCode();

unsigned long lastDebounce = 0;

const int code[] = {A, B, B, A};
const int codeLength = sizeof(code) / sizeof(code[0]);

int currentCode[sizeof(code) / sizeof(code[0])] = {0};
int currentCodeIndex = 0;

int previousAButtonState = HIGH;
int previousBButtonState = HIGH;

void setup()
{
  Serial.begin(115200);

  configurePins();

  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);

  previousAButtonState = digitalRead(A_BUTTON_PIN);
  previousBButtonState = digitalRead(B_BUTTON_PIN);

  logStart();
}

void loop()
{
  if (millis() - lastDebounce > DEBOUNCE_DELAY_MS)
  {
    lastDebounce = millis();

    int aButtonState = digitalRead(A_BUTTON_PIN);
    int bButtonState = digitalRead(B_BUTTON_PIN);

    bool aPressedEdge = (aButtonState == LOW && previousAButtonState == HIGH);
    bool bPressedEdge = (bButtonState == LOW && previousBButtonState == HIGH);

    logButtonStates(aButtonState == LOW, bButtonState == LOW);

    handleButtonPress(aPressedEdge, bPressedEdge);

    previousAButtonState = aButtonState;
    previousBButtonState = bButtonState;

    logInsertedCode(currentCode, currentCodeIndex);

    verifyCode();
  }
}

void handleButtonPress(int aPressedEdge, int bPressedEdge)
{
  if (aPressedEdge && bPressedEdge)
  {
    logBothButtonsPressed();
  }
  else if (aPressedEdge)
  {
    if (currentCodeIndex < codeLength)
    {
      currentCode[currentCodeIndex] = A;
      currentCodeIndex++;
    }
  }
  else if (bPressedEdge)
  {
    if (currentCodeIndex < codeLength)
    {
      currentCode[currentCodeIndex] = B;
      currentCodeIndex++;
    }
  }
}

void verifyCode()
{
  bool codeCorrect = true;
  for (int i = 0; i < codeLength; i++)
  {
    if (currentCode[i] != code[i])
    {
      codeCorrect = false;
      break;
    }
  }

  if (codeCorrect)
  {
    digitalWrite(RED_PIN, LOW);

    blinkLED(GREEN_PIN, 3, 200); // Blink green LED 3 times with 200ms delay
  }
  else
  {
    digitalWrite(GREEN_PIN, LOW);

    blinkLED(RED_PIN, 3, 200); // Blink red LED 3 times with 200ms delay
  }

  currentCodeIndex = 0;
}

void blinkLED(int pin, int times, int delayMs)
{
  digitalWrite(YELLOW_PIN, LOW);

  for (int i = 0; i < times; i++)
  {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
    delay(delayMs);
  }

  digitalWrite(YELLOW_PIN, HIGH);
}
