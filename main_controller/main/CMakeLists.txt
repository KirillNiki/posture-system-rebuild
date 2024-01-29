idf_component_register(
    SRCS
        "main.c"
        "https/server.c"
        "~/esp/posture-system/interfaces/wifi/wifi_ap.c"
        "~/esp/posture-system/interfaces/spiffs/spiffs.c"
        "~/esp/posture-system/interfaces/uart/uart.c"
        "~/esp/posture-system/interfaces/ds1302/ds1302.c"

    EMBED_TXTFILES
        "./certs/cacert.pem"
        "./certs/prvtkey.pem"

    INCLUDE_DIRS
        "."
        "~/esp/posture-system/interfaces/"
        
    REQUIRES
        "esp_https_server"
        "esp_wifi"
        "nvs_flash"
        "spiffs"
        "driver"
        "esp_adc"
        "json"
)
