

#include "esp_adc/adc_oneshot.h"

#ifndef MY_ADC_H_
#define MY_ADC_H_

#define gpios_num 10
#define serial_str_len 128
#define temp_dig_str_len 6
#define adc1_gpios_count 8
#define adc2_gpios_count 10

typedef struct gpio_tuple
{
    adc_oneshot_unit_handle_t *adc_unit_handler;
    int adc_channel;
    int gpio;
} tTuple;

int read_adc(tTuple channel_info);
void init_adc(int *channels, tTuple *channel_infos);

#endif
