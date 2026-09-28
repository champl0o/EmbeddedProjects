#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_PIN 18
#define LED_PIN 17
#define POT_PIN 4

#define POT_ADC_CHANNEL ADC_CHANNEL_3

#define LED_LEDC_CHANNEL LEDC_CHANNEL_0
#define MOTOR_LEDC_CHANNEL LEDC_CHANNEL_1

#define LED_LEDC_TIMER LEDC_TIMER_0
#define MOTOR_LEDC_TIMER LEDC_TIMER_1

#define LED_FREQ 1000
#define MOTOR_FREQ 19000

#define ADC_MAX_VALUE 4095
#define PWM_MAX_DUTY 4095

static adc_oneshot_unit_handle_t adc1_handle;

static void init_adc()
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc1_handle, POT_ADC_CHANNEL, &channel_config);
}

static void init_pwm(int frequency, int channel, int timer, int gpio_num)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = timer,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = frequency,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = gpio_num,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .timer_sel = timer,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);
}

void app_main()
{
    init_adc();
    init_pwm(LED_FREQ, LED_LEDC_CHANNEL, LED_LEDC_TIMER, LED_PIN);
    init_pwm(MOTOR_FREQ, MOTOR_LEDC_CHANNEL, MOTOR_LEDC_TIMER, MOTOR_PIN);

    while (1)
    {
        int adc_reading = 0;
        adc_oneshot_read(adc1_handle, POT_ADC_CHANNEL, &adc_reading);

        uint32_t duty_cycle = (adc_reading * PWM_MAX_DUTY) / ADC_MAX_VALUE;

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_LEDC_CHANNEL, duty_cycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_LEDC_CHANNEL);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL, duty_cycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
