
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <sys/time.h>

#include "uart.h"
char uart_buffer[uart_chank_buffer_size];

unsigned int millis(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + (t.tv_usec + 500) / 1000;
}

void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    // Set UART pins(TX: IO16 (UART2 default), RX: IO17 (UART2 default), RTS: IO18, CTS: IO19)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, uart_buffer_size,
                                        uart_buffer_size, 10, &uart_queue, 0));
}

void write_bites(char *string)
{
    uart_write_bytes(UART_NUM, (const char *)string, strlen(string));
}

void read_bites(void)
{
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t *)&length));
    if (length > uart_chank_buffer_size)
    {
        length = uart_chank_buffer_size;
    }
    uart_read_bytes(UART_NUM, uart_buffer, length, 100);
}
