#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#include "data_transer/data_transer.h"

#ifndef MY_BLUET_CONFIG_H_
#define MY_BLUET_CONFIG_H_

#define DEVICE_NAME "ESP_GATTS_DEMO"
#define MANUFACTURER_DATA_LEN 17

#define GATTS_SERVICE_UUID_A 0x00FF
#define GATTS_DESCR_UUID_A 0x3333
#define GATTS_NUM_HANDLE_A 10

#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define CHARS_NUM 3

static uint8_t adv_service_uuid128[32] = {
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
};

struct gatts_char_t
{
    char *definition;
    esp_bt_uuid_t uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    esp_attr_value_t attr_value;

    uint16_t handle;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t time_buffer[max_time_buffer];
static struct gatts_char_t gatts_chars[CHARS_NUM] = {
    {
        .definition = "data",
        .uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0xFF01,
            },
        },
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
        .attr_value = {
            .attr_max_len = max_json_buffer,
            .attr_len = max_json_buffer,
            .attr_value = json_data_buffer,
        },
    },
    {
        .definition = "time",
        .uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0xFF02,
            },
        },
        .perm = ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
        .attr_value = {
            .attr_max_len = max_time_buffer,
            .attr_len = max_time_buffer,
            .attr_value = json_time_buffer,
        },
    },
    {
        .definition = "train",
        .uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0xFF03,
            },
        },
        .perm = ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
        .attr_value = {
            .attr_max_len = sizeof(train_data),
            .attr_len = sizeof(train_data),
            .attr_value = train_data,
        },
    },
};

#endif
