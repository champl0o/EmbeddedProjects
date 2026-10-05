#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "encoder.h"
#include "safe.h"

void listen_encoder(int *last_count, int64_t *last_us, int *sw_prev)
{
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &count));

    int64_t now_us = esp_timer_get_time();
    int delta = count - *last_count;
    int64_t dt_us = now_us - *last_us;

    float rpm = 0.0f;
    if (dt_us > 0)
    {
        rpm = ((float)delta / STEPS_PER_REV) * (60.0f * 1000000.0f / (float)dt_us);
    }

    float angle = (count % STEPS_PER_REV) * (360.0f / STEPS_PER_REV);
    if (angle < 0.0f)
    {
        angle += 360.0f;
    }

    if (delta != 0)
    {
        ESP_LOGI(TAG, "%c кроки=%6d  клац=%5d  кут=%6.1f  RPM=%7.1f",
                 count,
                 count / PULSES_PER_DETENT,
                 angle,
                 rpm);
    }

    *last_count = count;
    *last_us = now_us;

    int sw = gpio_get_level(ENC_SW);
    if (sw == 0 && *sw_prev == 1)
    {
        ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
        *last_count = 0;
        ESP_LOGW(TAG, "нуль встановлено");
        reset_code();
    }
    *sw_prev = sw;
}

void app_main(void)
{
    encoder_init();

    int last_count = 0;
    int64_t last_us = esp_timer_get_time();
    int sw_prev = 1;

    while (1)
    {
        listen_encoder(&last_count, &last_us, &sw_prev);

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
