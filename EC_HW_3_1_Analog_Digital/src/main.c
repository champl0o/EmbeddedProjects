#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#define LED 16

#define LDR_GPIO 4
#define LDR_ADC_CHANNEL ADC_CHANNEL_3 /* GPIO4 -> ADC1_CH3 на ESP32-S3 */
#define NUM_READINGS 10
#define READING_DELAY 100

#define THRESHOLD_DARK 2200
#define THRESHOLD_LIGHT 1800

volatile int readings[NUM_READINGS] = {0};
volatile int readIndex = 0;
volatile long total = 0;
volatile long average = 0;

static esp_timer_handle_t ldr_timer;
static esp_timer_handle_t led_timer;
volatile bool update_flag = false;
volatile bool update_led_flag = false;

static bool led_on = false;

void set_update_flag(void *arg);
void set_update_led_flag(void *arg);
long calculate_average(void);

static void config_gpio()
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

static void init_timers()
{
    const esp_timer_create_args_t ldr_timer_args = {
        .callback = &set_update_flag,
        .name = "ldr_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&ldr_timer_args, &ldr_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(ldr_timer, READING_DELAY * 1000));

    const esp_timer_create_args_t led_timer_args = {
        .callback = &set_update_led_flag,
        .name = "led_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&led_timer_args, &led_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(led_timer, READING_DELAY * 1000));
}

static adc_oneshot_unit_handle_t adc_handle;

static void config_adc(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LDR_ADC_CHANNEL, &chan_cfg));
}

int read_ldr()
{
    int raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, LDR_ADC_CHANNEL, &raw));
    return raw;
}

void set_update_flag(void *arg)
{
    update_flag = true;
}

void set_update_led_flag(void *arg)
{
    update_led_flag = true;
}

void update_readings()
{
    total -= readings[readIndex];
    readings[readIndex] = read_ldr();
    total += readings[readIndex];
    readIndex = (readIndex + 1) % NUM_READINGS;
    average = calculate_average();
}

long calculate_average()
{
    return total / NUM_READINGS;
}

void update_led(void)
{
    if (!led_on && average > THRESHOLD_DARK)
    {
        led_on = true;
    }
    else if (led_on && average < THRESHOLD_LIGHT)
    {
        led_on = false;
    }
    gpio_set_level(LED, led_on);
}

void app_main()
{
    config_gpio();
    config_adc();
    init_timers();

    while (1)
    {
        if (update_flag)
        {
            update_readings();
            update_flag = false;
        }

        if (update_led_flag)
        {
            update_led();
            update_led_flag = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
