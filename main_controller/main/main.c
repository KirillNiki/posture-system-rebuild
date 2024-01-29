
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

#include "https/server.h"
#include "uart/uart.h"
#include "adc/adc.h"
#include "ds1302/ds1302.h"

int weights[gpios_num];
struct tm time_struct;
ds1302_t rtc_dev = {
    .ce_pin = CONFIG_CE_PIN,
    .io_pin = CONFIG_IO_PIN,
    .sclk_pin = CONFIG_SCLK_PIN,
};

void parse_data(void)
{
    read_bites();
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
}

void rx_task(void *args)
{
    while (1)
    {
        // parse_data();
        ds1302_get_time(&rtc_dev, &time_struct);
        printf("%s\n", asctime(&time_struct));
        vTaskDelay(TX_DELAY);
    }
}

void app_main(void)
{
    ds1302_init(&rtc_dev);
    ds1302_start(&rtc_dev, true);
    // init_server();
    // init_uart();

    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
}
