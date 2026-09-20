#include <stdio.h>
#include <inttypes.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_efuse_rtc_calib.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define POTENTIOMETER_PIN 4
#define POTENTIOMETER_CHANNEL ADC_CHANNEL_3
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12
#define ADC_MAX_RAW 4095
#define V_FS_MV 3100.0

static esp_timer_handle_t potentiometer_timer;
static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;
static bool cali_ready = false;

typedef struct
{
    int raw_value;
    double calculated_voltage;
    double calibrated_voltage;
    double margin_of_error;
} potentiometer_data_t;

static potentiometer_data_t potentiometer_data = {0};

static volatile bool potentiometer_data_update_flag;

void init_calibration()
{
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = POTENTIOMETER_CHANNEL,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    esp_err_t err = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    if (err == ESP_OK)
    {
        cali_ready = true;
    }
    else
    {
        ESP_ERROR_CHECK(err);
    }
}

void potentiometer_timer_callback(void *arg)
{
    potentiometer_data_update_flag = true;
}

void potentiometer_init()
{
    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTENTIOMETER_CHANNEL, &chan_config));

    const esp_timer_create_args_t timer_args = {
        .callback = potentiometer_timer_callback,
        .name = "potentiometer_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &potentiometer_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(potentiometer_timer, 100000));
}

/* мВ */
double calculate_voltage(int adc_value)
{
    return (adc_value / (double)ADC_MAX_RAW) * V_FS_MV;
}

/* мВ */
double calibrate_voltage(int adc_value)
{
    int calibrated = 0;
    if (cali_ready)
    {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, adc_value, &calibrated));
    }
    return calibrated;
}

double calculate_margin_of_error(double calculated_voltage, double calibrated_voltage)
{
    return (calculated_voltage - calibrated_voltage) / calibrated_voltage * 100.0;
}

void app_main()
{
    init_calibration();
    potentiometer_init();

    printf("RAW   U_manual(mV)   U_cali(mV)   Error(%%)\n");
    printf("------------------------------------------\n");

    while (1)
    {
        if (potentiometer_data_update_flag)
        {
            potentiometer_data_update_flag = false;
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, POTENTIOMETER_CHANNEL, &potentiometer_data.raw_value));
            potentiometer_data.calculated_voltage = calculate_voltage(potentiometer_data.raw_value);
            potentiometer_data.calibrated_voltage = calibrate_voltage(potentiometer_data.raw_value);

            if (potentiometer_data.calibrated_voltage > 0)
            {
                potentiometer_data.margin_of_error = calculate_margin_of_error(potentiometer_data.calculated_voltage,
                                                                               potentiometer_data.calibrated_voltage);
                printf("%4d   %12.1f   %10.0f   %8.2f\n",
                       potentiometer_data.raw_value,
                       potentiometer_data.calculated_voltage,
                       potentiometer_data.calibrated_voltage,
                       potentiometer_data.margin_of_error);
            }
            else
            {
                printf("%4d   %12.1f   %10.0f   %8s\n",
                       potentiometer_data.raw_value,
                       potentiometer_data.calculated_voltage,
                       potentiometer_data.calibrated_voltage,
                       "-");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
