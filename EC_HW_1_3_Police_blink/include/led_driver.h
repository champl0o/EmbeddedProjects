#pragma once
#include <Arduino.h>

void ledDriverBegin();                       // ledcAttach on all channels
void applyFrame(const uint8_t *frame);       // frame[0..N-1]=R, frame[N..2N-1]=B
