#include "patterns.h"

// 1. WIG-WAG: left pair / right pair alternate
static const uint8_t WIG_WAG[][CHANNELS] PROGMEM = {
    // R1   R2   B1   B2
    {255, 255, 0, 0},
    {0, 0, 255, 255},
};

// 2. RUNNING: chase wave across channels
static const uint8_t RUNNING[][CHANNELS] PROGMEM = {
    {255, 0, 0, 0},
    {0, 255, 0, 0},
    {0, 0, 255, 0},
    {0, 0, 0, 255},
};

// 3. QUAD-FLASH: 4 fast flashes per side
static const uint8_t QUAD_FLASH[][CHANNELS] PROGMEM = {
    {255, 255, 0, 0},
    {0, 0, 0, 0},
    {255, 255, 0, 0},
    {0, 0, 0, 0},
    {255, 255, 0, 0},
    {0, 0, 0, 0},
    {255, 255, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 255, 255},
    {0, 0, 0, 0},
    {0, 0, 255, 255},
    {0, 0, 0, 0},
    {0, 0, 255, 255},
    {0, 0, 0, 0},
    {0, 0, 255, 255},
    {0, 0, 0, 0},
};

// 4. FADE-ALTERNATE: smooth crossfade red <-> blue
static const uint8_t FADE_ALT[][CHANNELS] PROGMEM = {
    {255, 255, 0, 0},
    {200, 200, 60, 60},
    {140, 140, 120, 120},
    {80, 80, 180, 180},
    {0, 0, 255, 255},
    {80, 80, 180, 180},
    {140, 140, 120, 120},
    {200, 200, 60, 60},
};

Pattern patterns[] = {
    {WIG_WAG, 2, 400, "WIG-WAG"},
    {RUNNING, 4, 120, "RUNNING"},
    {QUAD_FLASH, 16, 70, "QUAD-FLASH"},
    {FADE_ALT, 8, 60, "FADE"},
};
const int PATTERN_COUNT = sizeof(patterns) / sizeof(patterns[0]);
