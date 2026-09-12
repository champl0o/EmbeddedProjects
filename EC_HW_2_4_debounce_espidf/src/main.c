#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DEBOUNCE";

#define BUTTON_PIN 16
#define DEBOUNCE_DELAY_MS 50

#define SELECTED_TASK 4

// TASK 1: Basic interrupt (no debounce)
#if SELECTED_TASK == 1

static volatile uint32_t interrupt_count = 0;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    interrupt_count++;
    ESP_EARLY_LOGI(TAG, "Interrupt #%lu triggered", interrupt_count);
}

void init_gpio_input()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
}

void app_main()
{
    ESP_LOGI(TAG, "=== TASK 1: Basic Interrupt (No Debounce) ===");
    init_gpio_input();
    ESP_LOGI(TAG, "Press button - watch for multiple interrupts per press");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// TASK 2: Time-based debounce (using interrupt + timer check)
#elif SELECTED_TASK == 2

static volatile uint32_t button_press_count = 0;
static volatile int64_t last_interrupt_time = 0;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    int64_t current_time = esp_timer_get_time();
    int64_t time_since_last = (current_time - last_interrupt_time) / 1000;

    if (time_since_last > DEBOUNCE_DELAY_MS)
    {
        button_press_count++;
        last_interrupt_time = current_time;
        ESP_EARLY_LOGI(TAG, "Valid press #%lu detected", button_press_count);
    }
    else
    {
        ESP_EARLY_LOGI(TAG, "Bounce rejected (dt=%lld ms)", time_since_last);
    }
}

void init_gpio_input()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
}

void app_main()
{
    ESP_LOGI(TAG, "=== TASK 2: Time-Based Debounce ===");
    ESP_LOGI(TAG, "Debounce delay: %d ms", DEBOUNCE_DELAY_MS);
    init_gpio_input();
    ESP_LOGI(TAG, "Press button - fewer interrupts than Task 1");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// TASK 3: State-based debounce (interrupt + level check in task)
#elif SELECTED_TASK == 3

static volatile uint32_t press_detected = 0;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    gpio_intr_disable(BUTTON_PIN);
    press_detected++;
}

void debounce_task(void *arg)
{
    uint32_t last_press_count = 0;

    while (1)
    {
        if (press_detected != last_press_count)
        {
            last_press_count = press_detected;

            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));

            if (gpio_get_level(BUTTON_PIN) == 0)
            {
                ESP_LOGI(TAG, "Button press confirmed! Count: %lu", last_press_count);

                while (gpio_get_level(BUTTON_PIN) == 0)
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
            }

            gpio_intr_enable(BUTTON_PIN);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void init_gpio_input()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
}

void app_main()
{
    ESP_LOGI(TAG, "=== TASK 3: State-Based Debounce ===");
    ESP_LOGI(TAG, "Debounce delay: %d ms", DEBOUNCE_DELAY_MS);
    init_gpio_input();

    xTaskCreate(debounce_task, "debounce_task", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Press button - exactly 1 reaction per press");
}

// TASK 4: Polling + State machine (no interrupts, pure polling)
#elif SELECTED_TASK == 4

typedef enum
{
    STATE_IDLE,
    STATE_PRESSED,
    STATE_RELEASED
} ButtonStateMachine;

void poll_task(void *arg)
{
    ButtonStateMachine state = STATE_IDLE;
    uint32_t stable_count = 0;
    uint32_t press_count = 0;
    const uint32_t STABLE_THRESHOLD = DEBOUNCE_DELAY_MS / 10;

    ESP_LOGI(TAG, "Polling task started");

    while (1)
    {
        int current_level = gpio_get_level(BUTTON_PIN);

        switch (state)
        {
        case STATE_IDLE:
            if (current_level == 0)
            {
                stable_count++;
                if (stable_count > STABLE_THRESHOLD)
                {
                    state = STATE_PRESSED;
                    press_count++;
                    ESP_LOGI(TAG, "Button PRESSED! Count: %lu", press_count);
                    stable_count = 0;
                }
            }
            else
            {
                stable_count = 0;
            }
            break;

        case STATE_PRESSED:
            if (current_level == 1)
            {
                stable_count++;
                if (stable_count > STABLE_THRESHOLD)
                {
                    state = STATE_IDLE;
                    ESP_LOGI(TAG, "Button RELEASED");
                    stable_count = 0;
                }
            }
            else
            {
                stable_count = 0;
            }
            break;

        default:
            state = STATE_IDLE;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10ms
    }
}

void init_gpio_input()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
}

void app_main()
{
    ESP_LOGI(TAG, "=== TASK 4: Polling + State Machine ===");
    ESP_LOGI(TAG, "Poll interval: 10ms, Debounce: %d ms", DEBOUNCE_DELAY_MS);
    init_gpio_input();

    xTaskCreate(poll_task, "poll_task", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Polling without interrupts - most stable method");
}

#endif
