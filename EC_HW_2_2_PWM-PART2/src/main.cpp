#include <Arduino.h>

#define POT_PIN 15
#define PWM_PIN 16

#define MAX_ADC 4095

#define PWM_PERIOD_MS 255

uint32_t potValue = 0;

uint32_t lastReadTime = 0;
uint32_t highDuration = 0;
uint32_t lowDuration = 0;

uint32_t highStartTime = 0;
uint32_t lowStartTime = 0;

uint32_t lastPrintTime = 0;

float calculateCoeficient(int potValue)
{
  return (float)potValue / (float)MAX_ADC;
}

void setup()
{
  pinMode(POT_PIN, INPUT);
  pinMode(PWM_PIN, OUTPUT);
  Serial.begin(115200);

  digitalWrite(PWM_PIN, LOW);
}

void loop()
{
  uint32_t currentTime = millis();
  if (currentTime - lastReadTime >= 50)
  {
    potValue = analogRead(POT_PIN);

    lastReadTime = currentTime;
  }

  float k = calculateCoeficient(potValue);
  highDuration = (uint32_t)(k * PWM_PERIOD_MS);
  if (highDuration == 0)
  {
    highDuration = 1;
  }
  else if (highDuration == 255)
  {
    highDuration = 254; // Ensure highDuration is at least 1 ms
  }
  lowDuration = (uint32_t)(PWM_PERIOD_MS - highDuration);
  if (lowDuration == 0)
  {
    lowDuration = 1; // Ensure lowDuration is at least 1 ms
  }
  else if (lowDuration == 255)
  {
    lowDuration = 254; // Ensure lowDuration is at least 1 ms
  }

  if (digitalRead(PWM_PIN) == HIGH && currentTime - highStartTime >= highDuration)
  {
    digitalWrite(PWM_PIN, LOW);
    lowStartTime = currentTime;
  }
  else if (digitalRead(PWM_PIN) == LOW && currentTime - lowStartTime >= lowDuration)
  {
    digitalWrite(PWM_PIN, HIGH);
    highStartTime = currentTime;
  }

  if (currentTime - lastPrintTime >= 1000)
  {
    Serial.print("Potentiometer Value: ");
    Serial.print(potValue);
    Serial.print(", High Duration: ");
    Serial.print(highDuration);
    Serial.print(", Low Duration: ");
    Serial.println(lowDuration);

    lastPrintTime = currentTime;
  }
}
