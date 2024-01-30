

#include "sdkconfig.h"

#ifndef MY_SEIALIZE_H_
#define MY_SEIALIZE_H_

#define time_str_size 13
#define serialization_buffer_size 1 + CONFIG_MAX_INFO_VALUES * (2 + time_str_size)

struct Info_file_cell
{
    int weight_at_time;
    char specific_time[time_str_size];
};
struct Info_file_struct
{
    int current_index;
    struct Info_file_cell info_file_cell[CONFIG_MAX_INFO_VALUES];
};

extern struct Info_file_struct info_file;
void serialize_info_struct(struct Info_file_struct *info_struct, char *buffer);
void deserialize_info_struct(struct Info_file_struct *info_struct, char *buffer);

#endif
