#include <Arduino.h>

#define PHOTORESISTOR_PIN 4
#define BUTTON_PIN 16
#define LED_PIN 17
#define DEBOUNCE_DELAY 50

#define LIGHT_THRESHOLD 2500

enum class LedState : uint8_t
{
  Off,
  OnDuty,
  Blinking
};

LedState state = LedState::Off;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

int readAdcValue();
int readButtonState();
void updateState();
void applyLedState();

void setup()
{
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetPinAttenuation(PHOTORESISTOR_PIN, ADC_11db);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
}

void loop()
{
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
  {
    readButtonState();
  }

  applyLedState();
}

int readAdcValue()
{
  int adcValue = analogRead(PHOTORESISTOR_PIN);

  Serial.print("ADC Value: ");
  Serial.println(adcValue);

  return adcValue;
}

int readButtonState()
{
  int buttonState = digitalRead(BUTTON_PIN);
  lastDebounceTime = millis();
  if (buttonState == HIGH && !buttonPressed)
  {
    buttonPressed = true;
    updateState();
    Serial.println("Pressed");
  }
  else if (buttonState == LOW && buttonPressed)
  {
    buttonPressed = false;
    Serial.println("Not Pressed");
  }

  return buttonState;
}

void updateState()
{
  switch (state)
  {
  case LedState::Off:
    state = LedState::OnDuty;
    Serial.println("LED State: OnDuty");
    break;
  case LedState::OnDuty:
  case LedState::Blinking:
    state = LedState::Off;
    Serial.println("LED State: Off");
    break;
  }
}

void applyLedState()
{
  switch (state)
  {
  case LedState::Off:
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED State: Off");
    break;
  case LedState::OnDuty:
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED State: OnDuty");
    if (readAdcValue() < LIGHT_THRESHOLD)
    {
      state = LedState::Blinking;
      Serial.println("LED State: OnDuty - LED ON");
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED State: OnDuty - LED OFF");
    }
    break;
  case LedState::Blinking:
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED State: Blinking - LED ON");
    break;
  }
}
