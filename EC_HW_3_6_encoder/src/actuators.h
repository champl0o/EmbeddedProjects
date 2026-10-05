#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "driver/ledc.h"

#define BUZZER_GPIO 16
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT

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

static uint16_t duty;

void servo_init();
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

static void tone(uint16_t freq, uint16_t ms)
{
    if (freq == 0 || ms == 0)
    {
        duty = 0;
        return;
    }
}

#endif // ACTUATORS_H
