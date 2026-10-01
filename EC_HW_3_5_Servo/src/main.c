#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

#define SERVO_GPIO 4
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_14_BIT

#define POT_CHANNEL ADC_CHANNEL_4

#define SERVO_MAX_DUTY (1u << 14)
#define SERVO_PERIOD_US 20000u

#define SERVO_MIN_US 400
#define SERVO_MAX_US 2600

#define POT_MIN_US 400
#define POT_MAX_US 2600

static void servo_init(void)
{
    ledc_timer_config_t t = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    ledc_channel_config_t c = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
}

static adc_oneshot_unit_handle_t adc1;

static void pot_init(void)
{

    adc_oneshot_unit_init_cfg_t ucfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&ucfg, &adc1);

    adc_oneshot_chan_cfg_t ccf = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc1, POT_CHANNEL, &ccf);
}

static void servo_set_us(uint32_t us)
{
    if (us > 2600)
        us = 2600;
    if (us < 400)
        us = 400;
    uint32_t duty = (uint32_t)((uint32_t)(us * SERVO_MAX_DUTY) / SERVO_PERIOD_US);

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

static void servo_set_angle(float deg)
{
    if (deg > 180.0f)
        deg = 180.0f;
    if (deg < 0.0f)
        deg = 0.0f;
    uint32_t us = SERVO_MIN_US + (uint32_t)((deg / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US));
    ESP_LOGI("check", " us = %d", us);

    servo_set_us(us);
}

void app_main(void)
{
    servo_init();

    servo_set_angle(90.0f);

    vTaskDelay(pdMS_TO_TICKS(1000));

    pot_init();

    while (1)
    {
        int raw = 0;

        adc_oneshot_read(adc1, POT_CHANNEL, &raw);

        uint32_t us = POT_MIN_US + ((uint32_t)raw * (POT_MAX_US - POT_MIN_US) / 4095);

        ESP_LOGI("check", " us = %d", us);
        servo_set_us(us);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
