
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"
#include "freertos/task.h"
#include "my_uart.h"
#include "my_adc.h"

static char serial_string[serial_str_len];
static int weights[gpios_num];
static int gpios[gpios_num] = {13, 12, 14, 27, 33, 26, 25, 34, 32, 35};
tTuple channel_infos[gpios_num];

static void read_data(void)
{
    memset(serial_string, 0, sizeof(serial_string));
    char temp_digit[temp_dig_str_len];
    for (int i = 0; i < gpios_num; i++)
    {
        int gpio_val = gpio_get_level(gpios[i]);
        int function_val = 1.8567 * gpio_val - 281.4006;
        weights[i] = function_val >= gpio_val ? function_val : gpio_val;

        snprintf(temp_digit, temp_dig_str_len, "%d", weights[i]);
        strcat(serial_string, temp_digit);
    }
}

static void tx_task(void *args)
{
    while (1)
    {
        read_data();
        for (int i = 0; i < gpios_num; i++)
        {
            int result = read_adc(channel_infos[i]);
            printf("%d: %d\n", channel_infos[i].gpio, result);
        }
        // write_bites(serial_string);
        vTaskDelay(TX_DELAY);
    }
}

void app_main(void)
{
    for (int i = 0; i < gpios_num; i++)
    {
        gpio_set_direction(gpios[i], GPIO_MODE_INPUT);
    }

    init_adc(gpios, channel_infos);
    // init_uart();
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}
