#include <Arduino.h>
#include "config.h"
#include "patterns.h"
#include "led_driver.h"
#include "police_display.h"

// ====== state ======
static int currentPattern = 0;
static int currentFrame = 0;
static bool redOn = true;
static int carX = -40;

// ====== cooperative scheduler ======
struct Task
{
  uint32_t (*intervalMs)(); // dynamic interval (read each tick)
  uint32_t last;
  void (*run)(uint32_t now);
};

static uint32_t patternFrameInterval() { return patterns[currentPattern].frameMs; }
static uint32_t blinkInterval()        { return BLINK_MS; }
static uint32_t carFrameInterval()     { return CAR_FRAME_MS; }
static uint32_t patternCycleInterval() { return PATTERN_CYCLE_MS; }

static void tickPatternFrame(uint32_t)
{
  Pattern &p = patterns[currentPattern];
  uint8_t frame[CHANNELS];
  for (int i = 0; i < CHANNELS; i++)
  {
    frame[i] = pgm_read_byte(&p.frames[currentFrame][i]);
  }
  applyFrame(frame);
  currentFrame = (currentFrame + 1) % p.frameCount;
}

static void tickBlink(uint32_t)
{
  redOn = !redOn;
  neopixelWrite(LED_BUILTIN, redOn ? 120 : 0, 0, redOn ? 0 : 120);
}

static void tickCar(uint32_t)
{
  carX += 2;
  if (carX > 130)
    carX = -40;
  drawPoliceCar(carX, redOn, patterns[currentPattern].name);
}

static void tickPatternCycle(uint32_t)
{
  currentPattern = (currentPattern + 1) % PATTERN_COUNT;
  currentFrame = 0;
  Serial.print("pattern: ");
  Serial.println(patterns[currentPattern].name);
}

static Task tasks[] = {
    {patternFrameInterval, 0, tickPatternFrame},
    {blinkInterval,        0, tickBlink},
    {carFrameInterval,     0, tickCar},
    {patternCycleInterval, 0, tickPatternCycle},
};

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println("start");

  ledDriverBegin();
  pinMode(LED_BUILTIN, OUTPUT);
  displayBegin();

  Serial.println("ready");
}

void loop()
{
  uint32_t now = millis();
  for (auto &t : tasks)
  {
    if (now - t.last >= t.intervalMs())
    {
      t.last = now;
      t.run(now);
    }
  }
}
