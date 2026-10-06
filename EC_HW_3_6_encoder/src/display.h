#ifndef DISPLAY_H
#define DISPLAY_H

#include "esp_lcd_io_spi.h"
#include "esp_log.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"

#define DISPLAY_MOSI 13
#define DISPLAY_SCLK 12
#define DISPLAY_CS 17
#define DISPLAY_DC 15
#define DISPLAY_RST 18

// RGB565: 5 біт червоного, 6 зеленого, 5 синього
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define COLOR_BG RGB565(0, 0, 0)
#define COLOR_BAR RGB565(40, 40, 60)
#define COLOR_TEXT RGB565(255, 255, 255)
#define COLOR_DIM RGB565(90, 90, 90)
#define COLOR_ACTIVE RGB565(255, 200, 0)
#define COLOR_OK RGB565(0, 200, 80)
#define COLOR_ERR RGB565(230, 30, 30)

void display_init(void);
void display_attempt(int attempt, int max);
void display_digit(int pos, int digit);
void display_confirm(int pos, int digit);
void display_status(const char *text, uint16_t color);
void display_locked(void);
void fill_rect(int x, int y, int w, int h, uint16_t color);
void draw_char(int x, int y, char ch, int scale, uint16_t fg, uint16_t bg);
void draw_text(int x, int y, const char *s, int scale, uint16_t fg, uint16_t bg);

#endif // DISPLAY_H
