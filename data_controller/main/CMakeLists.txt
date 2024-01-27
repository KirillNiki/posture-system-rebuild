idf_component_register(
    SRCS
        "data_reading.c"
        "~/esp/posture-system/my_interfaces/adc/adc.c"
        "~/esp/posture-system/my_interfaces/uart/uart.c"

    INCLUDE_DIRS
        "."
        "~/esp/posture-system/my_interfaces/"
        
    REQUIRES
        "driver"
        "esp_adc"
)
