#include "police_display.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

// ====== OLED ======
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// ====== screen geometry ======
namespace Screen
{
  constexpr int WIDTH = 128;
  constexpr int HEADER_X = 0, HEADER_Y = 7;
}

namespace Road
{
  constexpr int Y = 60;
  constexpr int MARK_Y = 58, MARK_LEN = 10, MARK_GAP = 20;
  constexpr uint32_t MARK_SCROLL_MS = 30;
}

namespace CarSprite
{
  // body / cabin
  constexpr int BODY_X = 2,  BODY_Y = 44, BODY_W = 36, BODY_H = 12;
  constexpr int CABIN_X = 10, CABIN_Y = 36, CABIN_W = 20, CABIN_H = 8;
  // windows
  constexpr int WIN_Y = 38, WIN_W = 7, WIN_H = 5;
  constexpr int WIN_L_X = 12, WIN_R_X = 21;
  // wheels (outer disc + hub)
  constexpr int WHEEL_Y = 57, WHEEL_R = 3, HUB_R = 1;
  constexpr int WHEEL_L_X = 9, WHEEL_R_X = 31;
  // light bar
  constexpr int LIGHT_Y = 32, LIGHT_W = 5, LIGHT_H = 4;
  constexpr int LIGHT_L_X = 14, LIGHT_R_X = 21;
  // POLICE label
  constexpr int LABEL_X = 4, LABEL_Y = 53;
  // rays above the left / right beacon
  constexpr int RAY_TOP_Y = 26, RAY_TOP_LEN = 4;
  constexpr int RAY_SIDE_Y = 28, RAY_SIDE_LEN = 3;
  constexpr int RAY_L_CENTER_X = 16, RAY_L_LEFT_X = 12, RAY_L_RIGHT_X = 20;
  constexpr int RAY_R_CENTER_X = 23, RAY_R_LEFT_X = 19, RAY_R_RIGHT_X = 27;
}

void displayBegin()
{
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  display.begin();
}

void drawPoliceCar(int x, bool redLight, const char *patternName)
{
  using namespace CarSprite;
  display.clearBuffer();

  // road
  display.drawHLine(0, Road::Y, Screen::WIDTH);
  for (int i = (millis() / Road::MARK_SCROLL_MS) % Road::MARK_GAP; i < Screen::WIDTH; i += Road::MARK_GAP)
  {
    display.drawHLine(i, Road::MARK_Y, Road::MARK_LEN); // markings scroll backward
  }

  // body and cabin
  display.drawBox(x + BODY_X, BODY_Y, BODY_W, BODY_H);
  display.drawBox(x + CABIN_X, CABIN_Y, CABIN_W, CABIN_H);

  // windows (cut out in black)
  display.setDrawColor(0);
  display.drawBox(x + WIN_L_X, WIN_Y, WIN_W, WIN_H);
  display.drawBox(x + WIN_R_X, WIN_Y, WIN_W, WIN_H);
  display.setDrawColor(1);

  // wheels
  display.drawDisc(x + WHEEL_L_X, WHEEL_Y, WHEEL_R);
  display.drawDisc(x + WHEEL_R_X, WHEEL_Y, WHEEL_R);
  display.setDrawColor(0);
  display.drawDisc(x + WHEEL_L_X, WHEEL_Y, HUB_R);
  display.drawDisc(x + WHEEL_R_X, WHEEL_Y, HUB_R);
  display.setDrawColor(1);

  // roof light bar
  if (redLight)
  {
    display.drawBox(x + LIGHT_L_X, LIGHT_Y, LIGHT_W, LIGHT_H);
    display.drawFrame(x + LIGHT_R_X, LIGHT_Y, LIGHT_W, LIGHT_H);
  }
  else
  {
    display.drawFrame(x + LIGHT_L_X, LIGHT_Y, LIGHT_W, LIGHT_H);
    display.drawBox(x + LIGHT_R_X, LIGHT_Y, LIGHT_W, LIGHT_H);
  }

  // POLICE label on the side (inverted)
  display.setFont(u8g2_font_5x7_tr);
  display.setDrawColor(0);
  display.drawStr(x + LABEL_X, LABEL_Y, "POLICE");
  display.setDrawColor(1);

  // beacon rays
  if (redLight)
  {
    display.drawVLine(x + RAY_L_CENTER_X, RAY_TOP_Y,  RAY_TOP_LEN);
    display.drawVLine(x + RAY_L_LEFT_X,   RAY_SIDE_Y, RAY_SIDE_LEN);
    display.drawVLine(x + RAY_L_RIGHT_X,  RAY_SIDE_Y, RAY_SIDE_LEN);
  }
  else
  {
    display.drawVLine(x + RAY_R_CENTER_X, RAY_TOP_Y,  RAY_TOP_LEN);
    display.drawVLine(x + RAY_R_LEFT_X,   RAY_SIDE_Y, RAY_SIDE_LEN);
    display.drawVLine(x + RAY_R_RIGHT_X,  RAY_SIDE_Y, RAY_SIDE_LEN);
  }

  // current pattern name at the top
  display.setFont(u8g2_font_5x7_tr);
  display.drawStr(Screen::HEADER_X, Screen::HEADER_Y, patternName);

  display.sendBuffer();
}
