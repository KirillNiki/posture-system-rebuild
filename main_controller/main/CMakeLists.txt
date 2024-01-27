idf_component_register(
    SRCS
        "main.c"
        "https/server.c"
        "~/esp/posture-system/my_interfaces/wifi/wifi_ap.c"
        "~/esp/posture-system/my_interfaces/spiffs/spiffs.c"
        "~/esp/posture-system/my_interfaces/uart/uart.c"

    EMBED_TXTFILES
        "./certs/cacert.pem"
        "./certs/prvtkey.pem"

    INCLUDE_DIRS
        "."
        "~/esp/posture-system/my_interfaces/"
        
    REQUIRES
        "esp_https_server"
        "esp_wifi"
        "nvs_flash"
        "spiffs"
        "driver"
        "esp_adc"
)
