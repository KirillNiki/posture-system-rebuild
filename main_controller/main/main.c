
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

#include "server.h"
#include "my_uart.h"

void rx_task(void *args)
{
    while (1)
    {
        read_bites();
        printf("%s\n", uart_buffer);
        memset(uart_buffer, 0, sizeof(uart_buffer));
        vTaskDelay(TX_DELAY);
    }
}

void app_main(void)
{
    // init_server();
    init_uart();
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
}
