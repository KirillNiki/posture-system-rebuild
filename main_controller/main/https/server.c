
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
#include "cJSON.h"

#include "wifi/wifi.h"
#include "spiffs/spiffs.h"
#include "server.h"
#include "data_interface.h"

static const char *TAG = "my_server";
static httpd_uri_t file_uris[file_count - 1]; // not for main file

static esp_err_t main_handler(httpd_req_t *req)
{
    char buffer[buffer_size];
    read_my_file(buffer, buffer_size, main_file);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t file_handler(httpd_req_t *req)
{
    printf("sending: >>>>> %s\n", req->uri);
    char buffer[buffer_size];
    char *path = (char *)req->uri;
    read_my_file(buffer, buffer_size, path);

    char *file_type = strchr(path, '.');
    if (strcmp(file_type, ".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
    else if (strcmp(file_type, ".js") == 0)
    {
        httpd_resp_set_type(req, "text/javascript");
    }
    else if (strcmp(file_type, ".json") == 0)
    {
        httpd_resp_set_type(req, "text/json");
    }
    else if (strcmp(file_type, ".png") == 0)
    {
        httpd_resp_set_type(req, "image/png");
    }
    else if (strcmp(file_type, ".svg") == 0)
    {
        httpd_resp_set_type(req, "image/svg+xml");
    }
    httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *weights_json = cJSON_CreateIntArray(weights, gpios_num);
    cJSON_AddItemToObject(root, "weights", weights_json);

    char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    cJSON_Delete(root);
    cJSON_free(json_string);
    return ESP_OK;
}

static const httpd_uri_t main_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = main_handler,
};

static const httpd_uri_t data_uri = {
    .uri = "/data",
    .method = HTTP_GET,
    .handler = data_handler,
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.httpd.stack_size = httpd_stack_size;
    config.httpd.max_uri_handlers = max_handlers;

    /* Load server certificate */
    extern const unsigned char servercert_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char servercert_end[] asm("_binary_cacert_pem_end");
    config.servercert = servercert_start;
    config.servercert_len = servercert_end - servercert_start;

    /* Load server private key */
    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");
    config.prvtkey_pem = prvtkey_pem_start;
    config.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    esp_err_t ret = httpd_ssl_start(&server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }
    httpd_register_uri_handler(server, &main_uri);
    httpd_register_uri_handler(server, &data_uri);

    int index = 0;
    for (int i = 0; i < file_count; i++)
    {
        if (strcmp(file_strings[i], main_file) == 0)
        {
            continue;
        }
        file_uris[index] = (httpd_uri_t){
            .uri = file_strings[i],
            .method = HTTP_GET,
            .handler = file_handler,
        };
        httpd_register_uri_handler(server, &file_uris[index]);
        index++;
    }
    return server;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    return httpd_ssl_stop(server);
}

static void connect_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)handler_arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}

static void disconnect_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)handler_arg;
    if (*server)
    {
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop https server");
        }
    }
}

void init_server(void)
{
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    init_spiffs();

#ifdef CONFIG_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    wifi_connect_init();
#endif
#ifdef CONFIG_SET_WIFI_AP
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &disconnect_handler, &server));
    wifi_ap_setup();
#endif
}
