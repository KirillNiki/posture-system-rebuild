
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "spiffs.h"
char file_strings[file_count][file_name_length];

void join_path(char *result_path, char *path)
{
    strcpy(result_path, "/spiffs");
    strcat(result_path, path);
}

void work_with_file(char *path, char *line, char *mode)
{
    int size = (int)strlen(path);
    char result_path[8 + size]; // len of "/spiffs" + 1 for 0
    join_path(result_path, path);

    FILE *file = fopen(result_path, mode);
    if (file == NULL)
    {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for writing");
        return;
    }
    fprintf(file, line);
    fclose(file);
}

void write_file(char *path, char *line)
{
    char *mode = "w";
    work_with_file(path, line, mode);
}

void append_file(char *path, char *line)
{
    char *mode = "a";
    work_with_file(path, line, mode);
}

void read_file(char *buffer, long size_of_buffer, char *path)
{
    if ((int)sizeof(buffer) == 0)
    {
        ESP_LOGE(SPIFFS_TAG, "error: buffer size == 0");
        return;
    }
    int size = (int)strlen(path);
    char result_path[8 + size]; // len of "/spiffs" + 1 for 0
    join_path(result_path, path);

    FILE *file = fopen(result_path, "r");
    if (file == NULL)
    {
        ESP_LOGE(SPIFFS_TAG, "error openning");
        return;
    }
    size_t n;
    n = fread(buffer, sizeof(buffer[0]), size_of_buffer, file);
    buffer[n] = 0;
    fclose(file);
}

void list_partiotions(void)
{
    DIR *dir = opendir("/spiffs");
    if (dir == NULL)
    {
        ESP_LOGE(SPIFFS_TAG, "cant open dir");
        return;
    }
    printf("\n");

    int index = 0;
    while (true)
    {
        struct dirent *de = readdir(dir);
        if (!de)
        {
            break;
        }
        int size = (int)strlen(de->d_name) + 1;
        strcpy(file_strings[index], "/");
        strcat(file_strings[index], de->d_name);
        file_strings[index][size] = 0;

        printf("Found file: %s\n", file_strings[index]);
        index++;
    }
    printf("\n\n");
    closedir(dir);
}

void init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = file_count,
        .format_if_mount_failed = true,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(SPIFFS_TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(SPIFFS_TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(SPIFFS_TAG, "Failed to initialize SPIFFS (%d)", ret);
        }
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SPIFFS_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(SPIFFS_TAG, "Partition size: total: %d, used: %d", total, used);
    }
    list_partiotions();
}
