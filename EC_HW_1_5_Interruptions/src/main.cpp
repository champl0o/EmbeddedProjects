#include <Arduino.h>

#define BUTTON_EXTERNAL 17

volatile uint16_t counter = 0;
volatile bool pressed = false;

void IRAM_ATTR reaction_left()
{
  counter++;      // короткий ISR — без Serial/String усередині переривання
  pressed = true; // лише прапорець; увесь друк робимо в loop()
}

void setup()
{
  pinMode(BUTTON_EXTERNAL, INPUT); // ТЕСТ: внутрішня підтяжка ~45 кОм
  Serial.begin(115200);
  delay(200);
  Serial.println("Press the button to see the count...");
  attachInterrupt(digitalPinToInterrupt(BUTTON_EXTERNAL), reaction_left, FALLING);
}

void loop()
{

  if (pressed)
  {
    pressed = false;
    Serial.println("Button Pressed! Count: " + String(counter)); // друк тут — безпечно
  }
}
