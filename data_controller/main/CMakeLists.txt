idf_component_register(
    SRCS
        "adc_communicator.c"
        "data_reading.c"
        "uart_connection.c"

    INCLUDE_DIRS
        "."
        
    REQUIRES
        "driver"
        "esp_adc"
)
