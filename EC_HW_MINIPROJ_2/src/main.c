#include "driver/gpio.h"
#include "esp_timer.h"

#define YELLOW_LED 15
#define RED_LED 16
#define GREEN_LED 17

#define BUTTON_PIN 0

#define GREEN_TIME 3000
#define GREEN_BLINK_TIME 3000
#define YELLOW_TIME 3000
#define RED_TIME 3000
#define RED_YELLOW_TIME 3000

#define BUTTON_DEBOUNCE_TIME 200

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
    uint32_t duration_ms;
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

static esp_timer_handle_t state_timer; // one-shot

void state_timer_callback(void *arg)
{
    // Handle state transition here
}

void gpio_init()
{
    gpio_config_t io_conf;
    // Configure yellow LED
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << YELLOW_LED);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // Configure red LED
    io_conf.pin_bit_mask = (1ULL << RED_LED);
    gpio_config(&io_conf);

    // Configure green LED
    io_conf.pin_bit_mask = (1ULL << GREEN_LED);
    gpio_config(&io_conf);
}

void timer_init()
{
    const esp_timer_create_args_t timer_args = {
        .callback = state_timer_callback,
        .arg = NULL,
        .name = "state_timer"};
    esp_timer_create(&timer_args, &state_timer);
}

void enter_state(led_t state)
{
    const state_cfg_t *cfg = &states[state];
    gpio_set_level(RED_LED, cfg->red);
    gpio_set_level(YELLOW_LED, cfg->yellow);
    gpio_set_level(GREEN_LED, cfg->green);
    esp_timer_stop(state_timer);
    esp_timer_start_once(state_timer, cfg->duration_ms * 1000);
}

volatile int64_t last_press_time = 0;
static void IRAM_ATTR handle_button(void *arg)
{
    int64_t current_time = esp_timer_get_time() / 1000;
    if (current_time - last_press_time > BUTTON_DEBOUNCE_TIME)
    {
        last_press_time = current_time;
        esp_timer_stop(state_timer);
        enter_state(YELLOW_BLINKING);
    }
}

void init_button()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, handle_button, NULL);
}

void app_main()
{
    gpio_init();
    timer_init();
    enter_state(GREEN);
    init_button();
}
