#include <Arduino.h>
#define ANALOG_PIN 4

int calculateVoltage(int analogValue);
void logValues(int analogValue, int voltage, int currentVoltage, int voltageDifferencePercentage);
int calculateVoltageDifferencePercentage(int voltage, int currentVoltage);

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, World!");
}
void loop()
{
  int analogValue = analogRead(ANALOG_PIN);
  int voltage = calculateVoltage(analogValue);
  int currentVoltage = analogReadMilliVolts(ANALOG_PIN);

  logValues(analogValue, voltage, currentVoltage, calculateVoltageDifferencePercentage(voltage, currentVoltage));

  delay(100);
}

int calculateVoltage(int analogValue)
{
  // Assuming a 3.3V reference voltage and a 12-bit ADC (0-4095)
  return (analogValue * 3300) / 4095; // Convert to millivolts
}

int calculateVoltageDifferencePercentage(int voltage, int currentVoltage)
{
  if (voltage == 0)
    return 0; // Avoid division by zero

  int difference = currentVoltage - voltage;
  return abs((difference * 100) / currentVoltage); // Return percentage difference
}

void logValues(int analogValue, int voltage, int currentVoltage, int voltageDifferencePercentage)
{
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.print(" mV");
  Serial.print(" | Current Voltage: ");
  Serial.print(currentVoltage);
  Serial.print(" mV");
  Serial.print(" | Voltage Difference: ");
  Serial.print(voltageDifferencePercentage);
  Serial.println(" %");
}
