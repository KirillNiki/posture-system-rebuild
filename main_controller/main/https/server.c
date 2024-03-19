#include <stdio.h>
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
#include "data_transer/data_transer.h"

static const char *TAG = "my_server";
static httpd_uri_t file_uris[file_count - 1]; // not for main file

static int set_cors_headers(httpd_req_t *req)
{
    printf("%s : .... %s\n", req->uri, CONFIG_CORS_ACCESS_ORIGIN);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", CONFIG_CORS_ACCESS_ORIGIN);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, HEAD, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-type");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
    return ESP_OK;
}

static esp_err_t send_by_chunks(char *path, httpd_req_t *req)
{
    int size = (int)strlen(path);
    char result_path[8 + size];
    join_path(result_path, path);

    FILE *file = fopen(result_path, "r");
    char *buffer = malloc(buffer_size);

    while (1)
    {
        size_t n = read_chunk_file(file, buffer, buffer_size);
        httpd_resp_send_chunk(req, buffer, n);
        memset(buffer, 0, buffer_size);

        if (n == 0)
        {
            break;
        }
    }
    fclose(file);
    return ESP_OK;
}

static esp_err_t main_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    send_by_chunks(main_file, req);

    httpd_resp_set_status(req, "200 OK");

    return ESP_OK;
}

static esp_err_t file_handler(httpd_req_t *req)
{
    printf("sending: >>>>> %s\n", req->uri);
    char *path = (char *)req->uri;
    char *file_type = strrchr(path, '.');

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
    send_by_chunks(path, req);
    memset(path, 0, strlen(path));

    httpd_resp_set_status(req, "200 OK");

    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req)
{
    char buffer[100];
    memset(buffer, 1, sizeof(buffer));
    httpd_req_get_hdr_value_str(req, "referer", buffer, sizeof(buffer));

    printf("%s\n", json_data_buffer);

    set_cors_headers(req);
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (char *)json_data_buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t time_handler(httpd_req_t *req)
{
    if (is_synchronized == false)
    {
        char unix_seconds[100];
        memset(unix_seconds, 0, sizeof(unix_seconds));
        size_t recv_size = MIN(req->content_len, sizeof(unix_seconds));
        int ret = httpd_req_recv(req, unix_seconds, recv_size);

        set_time(unix_seconds);
    }
    set_cors_headers(req);
    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t train_handler(httpd_req_t *req)
{
    set_train();

    set_cors_headers(req);
    main_handler(req);
    return ESP_OK;
}

static esp_err_t options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
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

static const httpd_uri_t data_options_uri = {
    .uri = "/data",
    .method = HTTP_OPTIONS,
    .handler = options_handler,
};

static const httpd_uri_t watch_options_uri = {
    .uri = "/watchTime",
    .method = HTTP_OPTIONS,
    .handler = options_handler,
};

static const httpd_uri_t train_options_uri = {
    .uri = "/train",
    .method = HTTP_OPTIONS,
    .handler = options_handler,
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
    httpd_register_uri_handler(server, &watch_uri);
    httpd_register_uri_handler(server, &train_uri);

    httpd_register_uri_handler(server, &watch_options_uri);
    httpd_register_uri_handler(server, &data_options_uri);
    httpd_register_uri_handler(server, &train_options_uri);

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
