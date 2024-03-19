#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
#include "data_transer/data_transer.h"
#include "bluetooth.h"
#include "bluetooth_config.h"

#define GATTS_TAG "GATTS_DEMO"
#define GATT_MAX_ATTR_LEN 500

int data_buffer_offset = 0;

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising start failed");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed");
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(
            GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
            param->update_conn_params.status,
            param->update_conn_params.min_int,
            param->update_conn_params.max_int,
            param->update_conn_params.conn_int,
            param->update_conn_params.latency,
            param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_A;

        esp_err_t ret;
        if ((ret = esp_bt_dev_set_device_name(DEVICE_NAME)) != ESP_OK)
        {
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", ret);
        }

        if ((ret = esp_ble_gap_config_adv_data(&adv_data)) != ESP_OK)
        {
            ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        if ((ret = esp_ble_gap_config_adv_data(&scan_rsp_data)) != ESP_OK)
        {
            ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_A);
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);

        for (int i = 0; i < CHARS_NUM; i++)
        {
            esp_err_t add_char_ret = esp_ble_gatts_add_char(
                gl_profile_tab[PROFILE_A_APP_ID].service_handle,
                &gatts_chars[i].uuid,
                gatts_chars[i].perm,
                gatts_chars[i].property,
                &gatts_chars[i].attr_value,
                NULL);
            if (add_char_ret)
            {
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        uint16_t length = 0;
        const uint8_t *prf_char;
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

        for (int i = 0; i < CHARS_NUM; i++)
        {
            if (gatts_chars[i].uuid.uuid.uuid16 == param->add_char.char_uuid.uuid.uuid16)
            {
                gatts_chars[i].handle = param->add_char.attr_handle;
                gatts_chars[i].descr_uuid.len = ESP_UUID_LEN_16;
                gatts_chars[i].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
                esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &prf_char);
                if (get_attr_ret == ESP_FAIL)
                {
                    ESP_LOGE(GATTS_TAG, "ILLEGAL HANDLE");
                }

                ESP_LOGI(GATTS_TAG, "the gatts demo char length = %x", length);
                esp_gatt_perm_t perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
                esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(
                    gl_profile_tab[PROFILE_A_APP_ID].service_handle,
                    &gatts_chars[i].descr_uuid,
                    perm,
                    NULL,
                    NULL);
                if (add_descr_ret)
                {
                    ESP_LOGE(GATTS_TAG, "add char descr failed, error code =%x", add_descr_ret);
                }
            }
        }
        break;
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        for (int i = 0; i < CHARS_NUM; i++)
        {
            if (gatts_chars[i].descr_uuid.uuid.uuid16 == param->add_char_descr.descr_uuid.uuid.uuid16)
            {
                gatts_chars[i].descr_handle = param->add_char_descr.attr_handle;
                ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d",
                         param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
            }
        }
        break;
    case ESP_GATTS_CONNECT_EVT:
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        conn_params.latency = 0;
        conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 600;  // timeout = 6000*10ms = 6000ms
        ESP_LOGI(
            GATTS_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
            param->connect.conn_id,
            param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
            param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;

        esp_ble_gap_update_conn_params(&conn_params);
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        data_buffer_offset = 0;
        break;
    case ESP_GATTS_READ_EVT:
    {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;

        for (int i = 0; i < CHARS_NUM; i++)
        {
            if (param->read.handle == gatts_chars[i].handle && strcmp(gatts_chars[i].definition, "data") == 0)
            {
                int copy_size = GATT_MAX_ATTR_LEN;
                bool add_end = false;
                if (strlen((char *)gatts_chars[i].attr_value.attr_value) - data_buffer_offset <= GATT_MAX_ATTR_LEN)
                {
                    copy_size = strlen((char *)gatts_chars[i].attr_value.attr_value) - data_buffer_offset;
                    add_end = true;
                }

                memcpy(rsp.attr_value.value, gatts_chars[i].attr_value.attr_value + data_buffer_offset, copy_size);
                data_buffer_offset += GATT_MAX_ATTR_LEN;
                rsp.attr_value.len = copy_size;
                if (add_end)
                {
                    strcat((char *)rsp.attr_value.value, "\r");
                    rsp.attr_value.len += 1;
                    data_buffer_offset = 0;
                }
            }
        }
        esp_ble_gatts_send_response(
            gatts_if,
            param->read.conn_id,
            param->read.trans_id,
            ESP_GATT_OK,
            &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT:
    {
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep)
        {
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            for (int i = 0; i < CHARS_NUM; i++)
            {
                if (gatts_chars[i].handle == param->write.handle)
                {
                    memset(gatts_chars[i].attr_value.attr_value, 0, gatts_chars[i].attr_value.attr_len);
                    memcpy(gatts_chars[i].attr_value.attr_value, param->write.value, param->write.len);
                    gatts_chars[i].attr_value.attr_value[param->write.len] = 0;

                    if (strcmp(gatts_chars[i].definition, "train") == 0)
                    {
                        set_train();
                    }
                    if (strcmp(gatts_chars[i].definition, "time") == 0)
                    {
                        printf("%s\n", (char *)gatts_chars[i].attr_value.attr_value);
                        set_time((char *)gatts_chars[i].attr_value.attr_value);
                    }
                }
            }
        }
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
        // esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        // example_exec_write_event_env(&a_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK)
        {
            esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE ||
                gatts_if == gl_profile_tab[idx].gatts_if)
            {
                if (gl_profile_tab[idx].gatts_cb)
                {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void init_bluetooth(void)
{
    esp_err_t ret;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_ble_gatts_register_callback(gatts_event_handler)) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    if ((ret = esp_ble_gap_register_callback(gap_event_handler)) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }

    if ((ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID)) != ESP_OK)
    {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(700);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    return;
}
