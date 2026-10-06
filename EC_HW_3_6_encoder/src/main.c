#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "encoder.h"
#include "safe.h"
#include "display.h"

void app_main(void)
{
    encoder_init();
    display_init();
    safe_init();

    while (1)
    {
        poll_encoder();
        poll_button();

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
