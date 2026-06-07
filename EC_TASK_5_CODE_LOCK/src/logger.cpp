#include "logger.h"

void logStart()
{
    Serial.println("System ready. Waiting for input...");
}

void logBothButtonsPressed()
{
    Serial.println("Both buttons pressed together - input ignored");
}

void logButtonStates(bool aState, bool bState)
{
    Serial.print("A Button: ");
    Serial.print(aState ? "Pressed" : "Released");
    Serial.print(" | B Button: ");
    Serial.println(bState ? "Pressed" : "Released");
}

void logInsertedCode(const int currentCode[], int currentCodeIndex)
{
    Serial.print("Inserted Code: ");
    for (int i = 0; i < currentCodeIndex; i++)
    {
        Serial.print(currentCode[i] == A ? "A" : "B");
        Serial.print(" ");
    }
    Serial.println();
}
