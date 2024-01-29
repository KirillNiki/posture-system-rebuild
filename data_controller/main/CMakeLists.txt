idf_component_register(
    SRCS
        "data_reading.c"
        "~/esp/posture-system/interfaces/adc/adc.c"
        "~/esp/posture-system/interfaces/uart/uart.c"

    INCLUDE_DIRS
        "."
        "~/esp/posture-system/interfaces/"
        
    REQUIRES
        "driver"
        "esp_adc"
)
