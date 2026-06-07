#include "lcd.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

bool feedbackActive = false;
unsigned long feedbackStartTime = 0;
const unsigned long FEEDBACK_DURATION = 1500; // milliseconds

void setupLCD()
{
    lcd.begin(16, 2);
    delay(500);     // Give LCD time to initialize
    lcd.display();  // Turn on display
    lcd.noCursor(); // Hide cursor
    lcd.noBlink();  // No blinking
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter code:");
}

void updateLCD(int currentCode[], int currentCodeIndex)
{
    lcd.setCursor(0, 1);
    lcd.print("                "); // clear line 2
    lcd.setCursor(0, 1);
    for (int i = 0; i < currentCodeIndex; i++)
    {
        lcd.print(currentCode[i] == A ? "A " : "B ");
    }
}

void startFeedback(bool success)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    if (success)
    {
        lcd.print("Code correct!");
    }
    else
    {
        lcd.print("Code incorrect!");
    }
    feedbackActive = true;
    feedbackStartTime = millis();
}

void showDoublePress()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Both pressed!");
    feedbackActive = true;
    feedbackStartTime = millis();
}

bool updateFeedback()
{
    if (!feedbackActive)
        return false;

    if (millis() - feedbackStartTime >= FEEDBACK_DURATION)
    {
        feedbackActive = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter code:");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        return true; // Feedback complete
    }
    return false; // Feedback still showing
}
