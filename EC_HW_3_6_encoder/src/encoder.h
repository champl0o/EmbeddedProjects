#ifndef ENCODER_H
#define ENCODER_H

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "safe.h"

#define ENC_A GPIO_NUM_9
#define ENC_B GPIO_NUM_10
#define ENC_SW GPIO_NUM_11

#define PCNT_HIGH_LIMIT 1000
#define PCNT_LOW_LIMIT -1000
#define PULSES_PER_DETENT 4
#define DETENTS_PER_REV 20
#define STEPS_PER_REV (PULSES_PER_DETENT * DETENTS_PER_REV)
#define BTN_LOCKOUT_US (300 * 1000)

#define LOOP_PERIOD_MS 10

static const char *TAG = "ENC";

static pcnt_unit_handle_t pcnt_unit = NULL;

static void encoder_init(void)
{
    pcnt_unit_config_t unit_cfg = {
        .high_limit = PCNT_HIGH_LIMIT,
        .low_limit = PCNT_LOW_LIMIT,
        .flags.accum_count = true,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_cfg, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_cfg = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_cfg));

    pcnt_chan_config_t chan_a_cfg = {
        .edge_gpio_num = ENC_A,
        .level_gpio_num = ENC_B,
    };
    pcnt_channel_handle_t chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_cfg, &chan_a));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
                                                 PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
                                                  PCNT_CHANNEL_LEVEL_ACTION_KEEP,      /* B = HIGH    */
                                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE)); /* B = LOW   */

    pcnt_chan_config_t chan_b_cfg = {
        .edge_gpio_num = ENC_B,
        .level_gpio_num = ENC_A,
    };
    pcnt_channel_handle_t chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_cfg, &chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
                                                 PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
                                                  PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_LOW_LIMIT));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    gpio_config_t sw_cfg = {
        .pin_bit_mask = (1ULL << ENC_SW),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&sw_cfg));

    ESP_LOGI(TAG, "PCNT X4 готовий. A=%d B=%d SW=%d, %d кроків на оберт",
             ENC_A, ENC_B, ENC_SW, STEPS_PER_REV);
}

static void poll_encoder(void)
{
    static int last_count = 0;
    static int acc = 0;

    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &count));
    int delta = count - last_count;
    acc += delta;
    last_count = count;

    while (acc >= PULSES_PER_DETENT)
    {
        acc -= PULSES_PER_DETENT;
        on_tick(+1);
    }
    while (acc <= -PULSES_PER_DETENT)
    {
        acc += PULSES_PER_DETENT;
        on_tick(-1);
    }
}

static void poll_button(void)
{
    static int last_state = 1;
    static int64_t last_press_us = 0;

    int now = gpio_get_level(ENC_SW);
    int64_t t = esp_timer_get_time();

    if (last_state == 1 && now == 0 && (t - last_press_us) > BTN_LOCKOUT_US)
    {
        last_press_us = t;
        on_button();
    }
    last_state = now;
}

#endif // ENCODER_H
