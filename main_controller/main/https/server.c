
#include "string.h"
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
#include <time.h>

#include "ds1302/ds1302.h"
#include "wifi/wifi.h"
#include "spiffs/spiffs.h"
#include "server.h"
#include "data_interface.h"

static const char *TAG = "my_server";
static httpd_uri_t file_uris[file_count - 1]; // not for main file

static int set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", CONFIG_CORS_ACCESS_ORIGIN);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, HEAD, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-type");
    return ESP_OK;
}

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
    char buffer[100];
    memset(buffer, 1, sizeof(buffer));
    httpd_req_get_hdr_value_str(req, "referer", buffer, sizeof(buffer));
    printf("req referrer >>> %s\n", buffer);

    cJSON *root = cJSON_CreateObject();
    cJSON *weights_json = cJSON_CreateIntArray(weights, gpios_num);
    cJSON_AddItemToObject(root, "weights", weights_json);

    cJSON *sitting_timer_json = cJSON_CreateNumber((double)sitting_timer);
    cJSON_AddItemToObject(root, "sittingTimer", sitting_timer_json);

    cJSON *infos_json = cJSON_CreateArray();
    int index = info_file.current_index;
    for (int i = 0; i < CONFIG_MAX_INFO_VALUES; i++)
    {
        if (index == CONFIG_MAX_INFO_VALUES)
        {
            index = 0;
        }
        cJSON *info_part = cJSON_CreateObject();
        cJSON *time = cJSON_CreateNumber((double)info_file.info_file_cell[index].unix_time);
        cJSON *weight = cJSON_CreateNumber((double)info_file.info_file_cell[index].weight_at_time);
        cJSON_AddItemToObject(info_part, "time", time);
        cJSON_AddItemToObject(info_part, "weight", weight);

        cJSON_AddItemToArray(infos_json, info_part);
        index++;
    }
    cJSON_AddItemToObject(root, "infoData", infos_json);

    char *json_string = cJSON_Print(root);
    set_cors_headers(req);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(root);
    cJSON_free(json_string);
    return ESP_OK;
}

static esp_err_t time_options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t time_handler(httpd_req_t *req)
{
    if (is_synchronized == false)
    {
        struct tm time;
        char unix_seconds[100];
        memset(unix_seconds, 0, sizeof(unix_seconds));
        size_t recv_size = MIN(req->content_len, sizeof(unix_seconds));
        int ret = httpd_req_recv(req, unix_seconds, recv_size);

        if (ret <= 0)
        {
            ESP_LOGE(TAG, "Error parsing post request!");
        }
        time_t time_val = 0;
        cJSON *req_json = cJSON_Parse(unix_seconds);
        cJSON *component_json;
        cJSON_ArrayForEach(component_json, req_json)
        {
            if (strcoll(component_json->string, "time:"))
            {
                time_val = (time_t)component_json->valueint;
            }
        }
        memcpy(&time, localtime((&time_val)), sizeof(struct tm));
        ds1302_set_time(&rtc_dev, &time);

        memset(&time, 0, sizeof(struct tm));
        memset(unix_seconds, 0, sizeof(unix_seconds));
        is_synchronized = true;
    }
    set_cors_headers(req);
    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t train_handler(httpd_req_t *req)
{
    is_train = !is_train;
    if (is_train == false)
    {
        struct tm time;
        ds1302_get_time(&rtc_dev, &time);
        sitting_timer = mktime(&time);
    }
    set_cors_headers(req);
    main_handler(req);
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

static const httpd_uri_t watch_options_uri = {
    .uri = "/watchTime",
    .method = HTTP_OPTIONS,
    .handler = time_options_handler,
};

static const httpd_uri_t watch_uri = {
    .uri = "/watchTime",
    .method = HTTP_POST,
    .handler = time_handler,
};

static const httpd_uri_t train_uri = {
    .uri = "/train",
    .method = HTTP_GET,
    .handler = train_handler,
};

esp_err_t not_found_handler(httpd_req_t *req, httpd_err_code_t error)
{
    printf("not found >>>>>> %s\n", req->uri);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    // httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.httpd.stack_size = httpd_stack_size;
    config.httpd.max_uri_handlers = max_handlers;
    config.httpd.server_port = 80;

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
    // esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, &not_found_handler);

    httpd_register_uri_handler(server, &main_uri);
    httpd_register_uri_handler(server, &data_uri);
    httpd_register_uri_handler(server, &watch_options_uri);
    httpd_register_uri_handler(server, &watch_uri);
    httpd_register_uri_handler(server, &train_uri);

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
