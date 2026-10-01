#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define BUZZER_GPIO 16
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT

#define BUTTON_DWN_GPIO 15
#define BUTTON_UP_GPIO 17
#define BUTTON_MID_GPIO 18
#define BUTTON_GPIO_MASK ((1ULL << BUTTON_DWN_GPIO) | (1ULL << BUTTON_UP_GPIO) | (1ULL << BUTTON_MID_GPIO))

#define R 0

#define C4 262
#define CS4 277
#define D4 294
#define DS4 311
#define E4 330
#define F4 349
#define FS4 370
#define G4 392
#define GS4 415
#define A4 440 /* еталон міжнародного настроювання */
#define AS4 466
#define B4 494

typedef struct
{
    uint16_t freq;
    uint16_t duration;
} note_t;

typedef struct
{
    uint16_t button;
    uint16_t note;
} table_t;

const static table_t table[] = {
    {BUTTON_DWN_GPIO, C4},
    {BUTTON_UP_GPIO, D4},
    {BUTTON_MID_GPIO, E4},
};

static void buzzer_init(void)
{
    ledc_timer_config_t t = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = 2700,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    ledc_channel_config_t c = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
}

static void buttons_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = BUTTON_GPIO_MASK,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);
}

static void tone_on(uint16_t freq)
{
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 512);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void tone_off(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void play_note(uint16_t freq)
{
    static uint16_t previous_freq = R;

    if (previous_freq == freq)
    {
        return;
    }
    previous_freq = freq;

    if (freq == R)
    {
        tone_off();
    }
    else
    {
        tone_on(freq);
    }
}

static void check_buttons(void)
{
    uint16_t current_freq = R;
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++)
    {
        if (gpio_get_level(table[i].button) == 0)
        {
            current_freq = table[i].note;
        }
    }
    play_note(current_freq);
}

void app_main(void)
{
    buzzer_init();
    buttons_init();

    while (1)
    {
        check_buttons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
