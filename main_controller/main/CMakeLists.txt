idf_component_register(
    SRCS
        "main.c"
        "~/esp/posture-system/my_interfaces/server.c"
        "~/esp/posture-system/my_interfaces/wifi_ap.c"
        "~/esp/posture-system/my_interfaces/spiffs.c"
        "~/esp/posture-system/my_interfaces/uart.c"

    EMBED_TXTFILES
        "./certs/cacert.pem"
        "./certs/prvtkey.pem"

    INCLUDE_DIRS
        "."
        "~/esp/posture-system/my_interfaces"
        
    REQUIRES
        "esp_https_server"
        "esp_wifi"
        "nvs_flash"
        "spiffs"
        "driver"
        "esp_adc"
)
