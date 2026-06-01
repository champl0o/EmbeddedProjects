#pragma once
#include <Arduino.h>
#include "config.h"

struct Pattern
{
  const uint8_t (*frames)[CHANNELS]; // pointer to frame array
  uint8_t frameCount;
  uint16_t frameMs;
  const char *name;
};

extern Pattern patterns[];
extern const int PATTERN_COUNT;
