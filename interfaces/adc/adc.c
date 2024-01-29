
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "adc.h"

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_handle_t adc2_handle;
const adc_bitwidth_t width = ADC_BITWIDTH_DEFAULT;
const adc_atten_t atten = ADC_ATTEN_DB_12;

int ADC1_GPIOS[adc1_gpios_count] = {36, 37, 38, 39, 32, 33, 34, 35};
int ADC2_GPIOS[adc2_gpios_count] = {4, 0, 2, 15, 13, 12, 14, 27, 25, 26};

tTuple get_gpio_info(int gpio)
{
    for (int i = 0; i < adc1_gpios_count; i++)
    {
        if (gpio == ADC1_GPIOS[i])
        {
            tTuple tuple = {
                .adc_unit_handler = &adc1_handle,
                .adc_channel = i,
                .gpio = gpio,
            };
            return tuple;
        }
    }
    for (int i = 0; i < adc2_gpios_count; i++)
    {
        if (gpio == ADC2_GPIOS[i])
        {
            tTuple tuple = {
                .adc_unit_handler = &adc2_handle,
                .adc_channel = i,
                .gpio = gpio,
            };
            return tuple;
        }
    }
    tTuple tuple = {.adc_channel = -1};
    return tuple;
}

int read_adc(tTuple channel_info)
{
    int value;
    adc_oneshot_read(*channel_info.adc_unit_handler, channel_info.adc_channel, &value);
    return value;
}

void init_adc(int *channels, tTuple *channel_infos)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = width,
        .atten = atten,
    };
    for (int i = 0; i < gpios_num; i++)
    {
        tTuple channel_info = get_gpio_info(channels[i]);
        channel_infos[i] = channel_info;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(*channel_info.adc_unit_handler, channel_info.adc_channel, &config));
    }
}
