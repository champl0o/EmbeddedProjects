#include <Arduino.h>

#define RELAY_PIN 4
#define RELAY_CONTACT_PIN 5
#define MEASUREMENTS 10
#define RELAY_OFF_PAUSE_MS 1000
#define CONTACT_TIMEOUT_MS 500

enum State
{
  START_MEASUREMENT,
  WAIT_CONTACT,
  WAIT_NEXT,
  DONE
};

volatile bool waitingForContact = false;
volatile bool contactTriggered = false;
volatile uint32_t relayContactTime = 0;
volatile uint32_t relayContactTimeUs = 0;

State state = START_MEASUREMENT;
uint8_t measurementCount = 0;
uint32_t totalRelayTime = 0;
uint32_t totalRelayTimeUs = 0;
uint32_t relayStartTime = 0;
uint32_t relayStartTimeUs = 0;
uint32_t stateStartTime = 0;

void IRAM_ATTR relay_contact_interrupt()
{
  if (!waitingForContact || contactTriggered)
  {
    return;
  }

  relayContactTime = millis();
  relayContactTimeUs = micros();
  contactTriggered = true;
  waitingForContact = false;
}

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(RELAY_CONTACT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RELAY_CONTACT_PIN), relay_contact_interrupt, FALLING);

  Serial.println();
  Serial.println("Relay contact timing test");
  Serial.println("RELAY_PIN: GPIO4, CONTACT_PIN: GPIO5");
  Serial.println("Contact wiring: GPIO5 -> NO, GND -> COM");
  Serial.println("----------------------------------------");
}

void loop()
{
  uint32_t now = millis();

  if (state == START_MEASUREMENT)
  {
    if (digitalRead(RELAY_CONTACT_PIN) == LOW)
    {
      Serial.println("Waiting: contact is still CLOSED");
      stateStartTime = now;
      state = WAIT_NEXT;
      return;
    }

    contactTriggered = false;
    waitingForContact = true;
    relayStartTime = millis();
    relayStartTimeUs = micros();

    digitalWrite(RELAY_PIN, HIGH);

    Serial.print("#");
    Serial.print(measurementCount + 1);
    Serial.println(" relay ON, waiting for contact...");

    state = WAIT_CONTACT;
    return;
  }

  if (state == WAIT_CONTACT && contactTriggered)
  {
    noInterrupts();
    uint32_t capturedRelayContactTime = relayContactTime;
    uint32_t capturedRelayContactTimeUs = relayContactTimeUs;
    contactTriggered = false;
    interrupts();

    waitingForContact = false;
    digitalWrite(RELAY_PIN, LOW);

    uint32_t relayTime = capturedRelayContactTime - relayStartTime;
    uint32_t relayTimeUs = capturedRelayContactTimeUs - relayStartTimeUs;

    measurementCount++;
    totalRelayTime += relayTime;
    totalRelayTimeUs += relayTimeUs;

    Serial.print("#");
    Serial.print(measurementCount);
    Serial.print(" contact CLOSED after ");
    Serial.print(relayTime);
    Serial.print(" ms (");
    Serial.print(relayTimeUs);
    Serial.println(" us)");

    stateStartTime = now;
    if (measurementCount >= MEASUREMENTS)
    {
      Serial.println("----------------------------------------");
      Serial.print("Average for ");
      Serial.print(measurementCount);
      Serial.print(" measurements: ");
      Serial.print(static_cast<float>(totalRelayTime) / measurementCount);
      Serial.print(" ms (");
      Serial.print(static_cast<float>(totalRelayTimeUs) / measurementCount);
      Serial.println(" us)");
      Serial.println("Done");
      state = DONE;
    }
    else
    {
      state = WAIT_NEXT;
    }
    return;
  }

  if (state == WAIT_CONTACT && now - relayStartTime >= CONTACT_TIMEOUT_MS)
  {
    waitingForContact = false;
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Timeout: contact did not close");

    stateStartTime = now;
    state = WAIT_NEXT;
    return;
  }

  if (state == WAIT_NEXT && now - stateStartTime >= RELAY_OFF_PAUSE_MS)
  {
    state = START_MEASUREMENT;
    return;
  }
}
