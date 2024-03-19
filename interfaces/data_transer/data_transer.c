
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>

#include "ds1302/ds1302.h"
#include "esp_http_server.h"
#include <esp_https_server.h>
#include "esp_tls.h"
#include "sdkconfig.h"
#include "cJSON.h"
#include "data_transer.h"
#include "data_interface.h"

uint8_t json_data_buffer[max_json_buffer];
uint8_t json_time_buffer[max_time_buffer];
uint8_t train_data[] = {0x00};

void set_train(void)
{
  printf(">>>>>>>>>> train was seted");
  is_train = !is_train;
  if (is_train == false)
  {
    struct tm time;
    ds1302_get_time(&rtc_dev, &time);
    sitting_timer = mktime(&time);
  }
}

void set_time(char *time_string)
{
  struct tm time;
  time_t time_val = 0;
  cJSON *req_json = cJSON_Parse(time_string);
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
  memset(time_string, 0, strlen(time_string));
  is_synchronized = true;
}

void build_json(void)
{
  cJSON *root = cJSON_CreateObject();

  cJSON *weights_json = cJSON_CreateArray();
  for (int i = 0; i < gpios_num; i++)
  {
    cJSON *weight = cJSON_CreateObject();
    cJSON *weight_num = cJSON_CreateNumber(weights[i]);
    cJSON_AddItemToObject(weight, "weight", weight_num);
    cJSON_AddItemToArray(weights_json, weight);
  }
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
    cJSON_AddItemToObject(info_part, "value", weight);

    cJSON_AddItemToArray(infos_json, info_part);
    index++;
  }
  cJSON_AddItemToObject(root, "infoData", infos_json);

  char *json_data = cJSON_Print(root);
  // size_t copy_size = strlen(json_data);
  // if (copy_size > max_json_buffer)
  // {
  //   copy_size = max_json_buffer;
  // }

  memset(json_data_buffer, 0, max_json_buffer);
  memcpy(json_data_buffer, json_data, strlen(json_data));
  cJSON_Delete(root);
  cJSON_free(json_data);
}
