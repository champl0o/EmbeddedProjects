#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#define BUZZER_GPIO 16
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define BUZ_TIMER LEDC_TIMER_1
#define BUZ_CHANNEL LEDC_CHANNEL_1
#define BUZ_DUTY_RES LEDC_TIMER_10_BIT
#define BUZ_DUTY_50 512

typedef struct
{
    uint16_t freq;
    uint16_t ms;
} note_t;

static const note_t melody_open[] = {
    {523, 120},
    {659, 120},
    {784, 120},
    {1047, 300}, // C5 E5 G5 C6
};

void servo_init(void);
static void buzzer_init(void)
{
    ledc_timer_config_t t = {
        .speed_mode = LEDC_MODE,
        .timer_num = BUZ_TIMER,
        .duty_resolution = BUZ_DUTY_RES,
        .freq_hz = 2700,
        .clk_cfg = LEDC_USE_APB_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    ledc_channel_config_t c = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = BUZ_CHANNEL,
        .timer_sel = BUZ_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
}

static void tone(uint16_t freq, uint16_t ms)
{
    if (freq == 0)
    {
        ledc_set_duty(LEDC_MODE, BUZ_CHANNEL, 0);
    }
    else
    {
        ledc_set_freq(LEDC_MODE, BUZ_TIMER, freq);
        ledc_set_duty(LEDC_MODE, BUZ_CHANNEL, BUZ_DUTY_50);
    }
    ledc_update_duty(LEDC_MODE, BUZ_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void buzzer_play_melody()
{
    for (size_t i = 0; i < sizeof(melody_open) / sizeof(melody_open[0]); i++)
    {
        tone(melody_open[i].freq, melody_open[i].ms);
    }

    tone(0, 0); // Turn off the buzzer after playing the melody
}

#endif // ACTUATORS_H
