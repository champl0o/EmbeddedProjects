#include "display.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "freertos/semphr.h"
#include "font5x7.h"
#include "safe.h"

#define LCD_W 320
#define LCD_H 240
#define STRIP_LINES 20

#define BAR_H 40
#define TEXT_SCALE 3  // 5x7 → 18x24 пікселі на символ разом із проміжком
#define DIGIT_SCALE 8 // 5x7 → 48x64
#define DIGIT_W (6 * DIGIT_SCALE)
#define DIGIT_GAP 24
#define DIGITS_X0 ((LCD_W - CODE_LEN * DIGIT_W - (CODE_LEN - 1) * DIGIT_GAP) / 2)
#define DIGITS_Y 80
#define STATUS_Y 180
#define STATUS_H 40

esp_lcd_panel_handle_t panel_handle = NULL;

static uint16_t strip[LCD_W * STRIP_LINES];
static SemaphoreHandle_t s_done;

static bool on_trans_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *ctx)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_done, &woken);
    return woken == pdTRUE;
}

static void blit(int x, int y, int w, int h)
{
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + w, y + h, strip));
    xSemaphoreTake(s_done, portMAX_DELAY);
}

void display_init(void)
{
    s_done = xSemaphoreCreateBinary();

    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = DISPLAY_MOSI,
        .sclk_io_num = DISPLAY_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 320 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t panel_io = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DISPLAY_DC,
        .cs_gpio_num = DISPLAY_CS,
        .pclk_hz = 20 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_trans_done,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &panel_io));

    ESP_LOGI("DISPLAY", "Display initialized successfully");

    esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = DISPLAY_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true); // IPS-панелі ST7789 потребують інверсії
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    fill_rect(0, 0, LCD_W, LCD_H, COLOR_BG);
}

void fill_rect(int x, int y, int w, int h, uint16_t color)
{
    int rows = (LCD_W * STRIP_LINES) / w;
    if (rows > h)
    {
        rows = h;
    }
    for (int i = 0; i < w * rows; i++)
    {
        strip[i] = color;
    }
    for (int yy = y; yy < y + h; yy += rows)
    {
        int n = (y + h - yy < rows) ? (y + h - yy) : rows;
        blit(x, yy, w, n);
    }
}

void draw_char(int x, int y, char ch, int scale, uint16_t fg, uint16_t bg)
{
    const uint8_t *g = font_glyph(ch);
    int w = 6 * scale;
    int h = 8 * scale;

    for (int py = 0; py < h; py++)
    {
        int row = py / scale;
        for (int px = 0; px < w; px++)
        {
            int col = px / scale;
            bool on = col < 5 && row < 7 && ((g[col] >> row) & 1);
            strip[py * w + px] = on ? fg : bg;
        }
    }
    blit(x, y, w, h);
}

void draw_text(int x, int y, const char *s, int scale, uint16_t fg, uint16_t bg)
{
    for (; *s; s++, x += 6 * scale)
    {
        draw_char(x, y, *s, scale, fg, bg);
    }
}

static void draw_text_centered(int y, const char *s, int scale, uint16_t fg, uint16_t bg)
{
    int w = (int)strlen(s) * 6 * scale;
    draw_text((LCD_W - w) / 2, y, s, scale, fg, bg);
}

static int digit_x(int pos)
{
    return DIGITS_X0 + pos * (DIGIT_W + DIGIT_GAP);
}

void display_attempt(int attempt, int max)
{
    char line[16];
    snprintf(line, sizeof(line), "TRY %d/%d", attempt, max);

    fill_rect(0, 0, LCD_W, LCD_H, COLOR_BG);
    fill_rect(0, 0, LCD_W, BAR_H, COLOR_BAR);
    draw_text_centered((BAR_H - 8 * TEXT_SCALE) / 2 + TEXT_SCALE, line, TEXT_SCALE, COLOR_TEXT, COLOR_BAR);

    for (int i = 0; i < CODE_LEN; i++)
    {
        draw_char(digit_x(i), DIGITS_Y, '_', DIGIT_SCALE, COLOR_DIM, COLOR_BG);
    }
    display_status("TURN TO ENTER", COLOR_DIM);
}

void display_digit(int pos, int digit)
{
    draw_char(digit_x(pos), DIGITS_Y, (char)('0' + digit), DIGIT_SCALE, COLOR_ACTIVE, COLOR_BG);
}

void display_confirm(int pos, int digit)
{
    draw_char(digit_x(pos), DIGITS_Y, (char)('0' + digit), DIGIT_SCALE, COLOR_TEXT, COLOR_BG);
}

void display_status(const char *text, uint16_t color)
{
    fill_rect(0, STATUS_Y, LCD_W, STATUS_H, COLOR_BG);
    draw_text_centered(STATUS_Y + (STATUS_H - 8 * TEXT_SCALE) / 2, text, TEXT_SCALE, color, COLOR_BG);
}

void display_locked(void)
{
    fill_rect(0, 0, LCD_W, LCD_H, COLOR_ERR);
    draw_text_centered(70, "LOCKED", 6, COLOR_TEXT, COLOR_ERR);
    draw_text_centered(160, "REBOOT TO RETRY", 2, COLOR_TEXT, COLOR_ERR);
}
