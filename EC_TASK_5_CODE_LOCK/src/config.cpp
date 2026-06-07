#include "config.h"

void configurePins()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);

    pinMode(A_BUTTON_PIN, INPUT_PULLUP);
    pinMode(B_BUTTON_PIN, INPUT_PULLUP);
}
