idf_component_register(
    SRCS
        "main.c"
        "server.c"
        "wifi_connection.c"
        "wifi_ap_setup.c"
        "spiffs_communicator.c"
        "uart_connection.c"

    EMBED_TXTFILES
        "./certs/cacert.pem"
        "./certs/prvtkey.pem"

    INCLUDE_DIRS
        "."
        
    REQUIRES
        "esp_https_server"
        "esp_wifi"
        "nvs_flash"
        "spiffs"
        "driver"
        "esp_adc"
)
