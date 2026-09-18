#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define YELLOW_LED 17
#define RED_LED 16
#define GREEN_LED 18

#define BUTTON_PIN 0

#define GREEN_TIME 3000
#define GREEN_BLINK_TIME 2000
#define YELLOW_TIME 2000
#define RED_TIME 3000
#define RED_YELLOW_TIME 2000

#define BLINK_PERIOD_MS 500
#define BUTTON_DEBOUNCE_TIME 200
#define BUTTON_POLL_MS 10

static const char *TAG = "traffic_light";

typedef enum
{
    GREEN,
    GREEN_BLINKING,
    YELLOW,
    YELLOW_BLINKING,
    RED,
    RED_AND_YELLOW
} led_t;

typedef struct
{
    uint8_t red, yellow, green;
    bool blink;
    uint32_t duration_ms; // 0 = stay until the button is pressed
    led_t next;
} state_cfg_t;

static const state_cfg_t states[] = {
    [GREEN] = {0, 0, 1, false, GREEN_TIME, GREEN_BLINKING},
    [GREEN_BLINKING] = {0, 0, 1, true, GREEN_BLINK_TIME, YELLOW},
    [YELLOW] = {0, 1, 0, false, YELLOW_TIME, RED},
    [RED] = {1, 0, 0, false, RED_TIME, RED_AND_YELLOW},
    [RED_AND_YELLOW] = {1, 1, 0, false, RED_YELLOW_TIME, GREEN},
    [YELLOW_BLINKING] = {0, 1, 0, true, 0, YELLOW_BLINKING},
};

static const char *state_names[] = {
    [GREEN] = "GREEN",
    [GREEN_BLINKING] = "GREEN_BLINKING",
    [YELLOW] = "YELLOW",
    [YELLOW_BLINKING] = "YELLOW_BLINKING",
    [RED] = "RED",
    [RED_AND_YELLOW] = "RED_AND_YELLOW",
};

static esp_timer_handle_t state_timer; // one-shot: how long the current state lasts
static esp_timer_handle_t blink_timer; // periodic: only armed in blinking states

static volatile led_t current_state = RED;
static volatile bool button_pressed = false;
static volatile bool state_expired = false;
static volatile bool blink_tick = false;
static bool blink_on = false;

void enter_state(led_t state);
void init_button(void);
void gpio_init(void);
void timer_init(void);
void state_timer_callback(void *arg);
void blink_timer_callback(void *arg);

static void stop_timer(esp_timer_handle_t timer)
{
    (void)esp_timer_stop(timer);
}

static void apply_lamps(const state_cfg_t *cfg, bool on)
{
    gpio_set_level(RED_LED, cfg->red && on);
    gpio_set_level(YELLOW_LED, cfg->yellow && on);
    gpio_set_level(GREEN_LED, cfg->green && on);
}

void enter_state(led_t state)
{
    if (state == GREEN && current_state != RED_AND_YELLOW)
    {
        ESP_LOGE(TAG, "illegal transition %s -> GREEN", state_names[current_state]);
    }

    stop_timer(state_timer);
    stop_timer(blink_timer);
    state_expired = false;
    blink_tick = false;

    const state_cfg_t *cfg = &states[state];
    current_state = state;

    blink_on = true;
    apply_lamps(cfg, blink_on);

    ESP_LOGI(TAG, "%-15s (t=%lld us)", state_names[state], esp_timer_get_time());

    if (cfg->blink)
    {
        ESP_ERROR_CHECK(esp_timer_start_periodic(blink_timer, BLINK_PERIOD_MS * 1000ULL));
    }

    if (cfg->duration_ms)
    {
        ESP_ERROR_CHECK(esp_timer_start_once(state_timer, cfg->duration_ms * 1000ULL));
    }
}

void state_timer_callback(void *arg)
{
    state_expired = true;
}

void blink_timer_callback(void *arg)
{
    blink_tick = true;
}

void gpio_init(void)
{
    gpio_config_t io_conf = {0};
    // Configure yellow LED
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << YELLOW_LED);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure red LED
    io_conf.pin_bit_mask = (1ULL << RED_LED);
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure green LED
    io_conf.pin_bit_mask = (1ULL << GREEN_LED);
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

void timer_init(void)
{
    const esp_timer_create_args_t state_timer_args = {
        .callback = state_timer_callback,
        .name = "state_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&state_timer_args, &state_timer));

    const esp_timer_create_args_t blink_timer_args = {
        .callback = blink_timer_callback,
        .name = "blink_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&blink_timer_args, &blink_timer));
}

static void IRAM_ATTR handle_button(void *arg)
{
    static volatile int64_t last_press_time = 0;

    int64_t current_time = esp_timer_get_time() / 1000;
    if (current_time - last_press_time > BUTTON_DEBOUNCE_TIME)
    {
        last_press_time = current_time;
        button_pressed = true;
    }
}

void init_button(void)
{
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, handle_button, NULL));
}

void app_main(void)
{
    gpio_init();
    timer_init();
    init_button();

    enter_state(RED);

    while (1)
    {
        if (button_pressed)
        {
            button_pressed = false;
            enter_state(current_state == YELLOW_BLINKING ? RED : YELLOW_BLINKING);
        }
        else if (state_expired)
        {
            state_expired = false;
            enter_state(states[current_state].next);
        }
        else if (blink_tick)
        {
            blink_tick = false;
            blink_on = !blink_on;
            apply_lamps(&states[current_state], blink_on);
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}
