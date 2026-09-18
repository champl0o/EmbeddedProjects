#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#define RELAY_PIN 4

#define WORK_TIME (1UL * 1000ULL)
#define PERIOD_TIME (5UL * 1000ULL)
#define IDLE_TIME (PERIOD_TIME - WORK_TIME)

static const char *TAG = "relay";

typedef enum
{
    RELAY_OFF,
    RELAY_ON
} relay_state_t;

typedef struct
{
    volatile relay_state_t state;
} relay_t;

relay_t relay;

static esp_timer_handle_t periodic_timer;
static esp_timer_handle_t one_time_timer;

void set_relay_on(void *arg);
void set_relay_off(void *arg);
void one_time_timer_init(void);
void periodic_timer_init(void);

void set_relay_on(void *arg)
{
    if (relay.state == RELAY_ON)
    {
        ESP_LOGW(TAG, "Relay is already ON (t=%lld us)", esp_timer_get_time());
        return;
    }

    relay.state = RELAY_ON;
    gpio_set_level(RELAY_PIN, relay.state == RELAY_ON ? 1 : 0);
    ESP_LOGI(TAG, "ON  (t=%lld us)", esp_timer_get_time());

    esp_err_t err = esp_timer_start_once(one_time_timer, WORK_TIME * 1000);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "start_once: %s", esp_err_to_name(err));
        set_relay_off(NULL);
    }
}

void set_relay_off(void *arg)
{
    if (relay.state == RELAY_OFF)
    {
        ESP_LOGW(TAG, "Relay is already OFF (t=%lld us)", esp_timer_get_time());
        return;
    }

    relay.state = RELAY_OFF;
    gpio_set_level(RELAY_PIN, relay.state == RELAY_ON ? 1 : 0);
    ESP_LOGI(TAG, "OFF (t=%lld us)", esp_timer_get_time());
}

void gpio_init()
{
    gpio_reset_pin(RELAY_PIN);
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "GPIO initialized (t=%lld us)", esp_timer_get_time());
}
void periodic_timer_init()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &set_relay_on,
        .name = "relay_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, PERIOD_TIME * 1000));
    ESP_LOGI(TAG, "Periodic timer initialized (t=%lld us)", esp_timer_get_time());
}

void one_time_timer_init()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &set_relay_off,
        .name = "one_time_relay_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &one_time_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(one_time_timer, WORK_TIME * 1000));
    ESP_LOGI(TAG, "One-time timer initialized (t=%lld us)", esp_timer_get_time());
}

void setup_watchdog(void)
{
    // TWDT is already started at boot (CONFIG_ESP_TASK_WDT_INIT), so only subscribe this task
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_LOGI(TAG, "Watchdog initialized (t=%lld us)", esp_timer_get_time());
}

void check_reboot_reason(void)
{
    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI(TAG, "Reboot reason: %d (t=%lld us)", reason, esp_timer_get_time());
}

void app_main()
{
    check_reboot_reason();

    gpio_init();

    relay.state = RELAY_OFF;

    periodic_timer_init();
    one_time_timer_init();
    set_relay_off(NULL);

    setup_watchdog();
    while (1)
    {
        esp_task_wdt_reset(); // Reset the watchdog timer
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
