#pragma once

#include "config.h"
#include "LiquidCrystal.h"

void setupLCD();
void updateLCD(int currentCode[], int currentCodeIndex);
void startFeedback(bool success);
void showDoublePress();
bool updateFeedback();
