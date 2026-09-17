#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define RELAY_PIN 4

#define WORK_TIME (1UL * 1000ULL)
#define PERIOD_TIME (5UL * 1000ULL)
#define IDLE_TIME (PERIOD_TIME - WORK_TIME)

typedef enum
{
    RELAY_OFF,
    RELAY_ON
} relay_state_t;

typedef struct
{
    relay_state_t state;
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
    relay.state = RELAY_ON;
    gpio_set_level(RELAY_PIN, relay.state == RELAY_ON ? 1 : 0);
    ESP_ERROR_CHECK(esp_timer_start_once(one_time_timer, WORK_TIME * 1000));
}

void set_relay_off(void *arg)
{
    relay.state = RELAY_OFF;
    gpio_set_level(RELAY_PIN, relay.state == RELAY_ON ? 1 : 0);
}

void gpio_init()
{
    gpio_reset_pin(RELAY_PIN);
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);
}
void periodic_timer_init()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &set_relay_on,
        .name = "relay_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, PERIOD_TIME * 1000));
}

void one_time_timer_init()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &set_relay_off,
        .name = "one_time_relay_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &one_time_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(one_time_timer, WORK_TIME * 1000));
}

void app_main()
{
    gpio_init();

    relay.state = RELAY_OFF;

    periodic_timer_init();
    one_time_timer_init();
    set_relay_off(NULL);
}
