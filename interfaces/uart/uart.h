

#ifndef MY_UART_H_
#define MY_UART_H_

#define UART_NUM UART_NUM_2
#define TX_PIN 17
#define RX_PIN 16
#define uart_buffer_size 1024 * 2
#define uart_chank_buffer_size 256
#define TX_DELAY 2000 / portTICK_PERIOD_MS // seconds

extern char uart_buffer[uart_chank_buffer_size];

unsigned int millis(void);
void init_uart(void);
void write_bites(char *string);
void read_bites(void);

#endif
