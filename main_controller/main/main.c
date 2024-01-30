
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"

#include "esp_http_server.h"
#include <esp_https_server.h>
#include "esp_tls.h"
#include "sdkconfig.h"
#include "freertos/task.h"
#include <time.h>

#include "sort.h"
#include "https/server.h"
#include "uart/uart.h"
#include "adc/adc.h"
#include "ds1302/ds1302.h"

long sitting_timer = 0;
long not_sitting_timer = 0;
long current_weight = 0;
int weights[gpios_num];

struct tm time_struct;
ds1302_t rtc_dev = {
    .ce_pin = CONFIG_CE_PIN,
    .io_pin = CONFIG_IO_PIN,
    .sclk_pin = CONFIG_SCLK_PIN,
};

void sitting_timer_change()
{
    int sitting_counter = 0;
    bool is_sitting = false;
    for (int i = 0; i < gpios_num; i++)
    {
        if (weights[i] > CONFIG_COUNTABLE_WEIGHT)
        {
            sitting_counter++;
        }
        if (sitting_counter >= CONFIG_MIN_COUNTABLE_WEIGHTS)
        {
            is_sitting = true;
            break;
        }
    }
    if (is_sitting == false)
    {
        if ((int)mktime(&time_struct) - not_sitting_timer >= CONFIG_MAX_NOT_SIT_TIME)
        {
            sitting_timer = (int)mktime(&time_struct);
        }
    }
    else
    {
        not_sitting_timer = (int)mktime(&time_struct);
    }
}

void calculate_weight(void)
{
    int sorted_weights[gpios_num];
    for (int i = 0; i < gpios_num; i++)
    {
        sorted_weights[i] = weights[i];
    }
    quick_sort(sorted_weights, gpios_num);
    current_weight = (sorted_weights[gpios_num - 1] + sorted_weights[gpios_num - 2]) / 2 * CONFIG_WEIGTH_FACTOR;
    memset(sorted_weights, 0, sizeof(sorted_weights));
}

// return was read or not
bool parse_data(void)
{
    read_bites();
    if (strlen(uart_buffer) == 0)
    {
        return false;
    }

    char temp_str[temp_dig_str_len];
    int temp_str_index = 0;
    int weights_index = 0;
    for (int i = 0; i < strlen(uart_buffer); i++)
    {
        if (uart_buffer[i] == ',')
        {
            weights[weights_index] = atoi(temp_str);
            weights_index++;
            memset(temp_str, 0, sizeof(temp_str));
            temp_str_index = 0;
            continue;
        }
        if (uart_buffer[i] == ';')
        {
            break;
        }
        temp_str[temp_str_index] = uart_buffer[i];
        temp_str_index++;
    }
    memset(uart_buffer, 0, sizeof(uart_buffer));
    return true;
}

void rx_task(void *args)
{
    while (true)
    {
        bool was_read = parse_data();
        if (was_read)
        {
            calculate_weight();
        }
        ds1302_get_time(&rtc_dev, &time_struct);
        sitting_timer_change();
        vTaskDelay(TX_DELAY);
    }
}

void save_data_task(void *args)
{
    while (true)
    {
        
        vTaskDelay(TX_DELAY);
    }
}

void app_main(void)
{
    init_server();
    init_uart();
    ds1302_init(&rtc_dev);
    ds1302_start(&rtc_dev, true);

    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(save_data_task, "save_data_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
}
