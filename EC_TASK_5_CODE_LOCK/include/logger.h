#pragma once

#include "Arduino.h"
#include "config.h"

void logStart();
void logBothButtonsPressed();
void logButtonStates(bool aState, bool bState);
void logInsertedCode(const int currentCode[], int currentCodeIndex);
