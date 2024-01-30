
#include "sdkconfig.h"
#include "struct_serialization.h"

void serialize_info_struct(struct Info_file_struct *info_struct, char *buffer)
{
    int *pointer = (int *)buffer;
    *pointer = info_struct->current_index;
    pointer++;

    for (int i = 0; i < CONFIG_MAX_INFO_VALUES; i++)
    {
        *pointer = info_struct->info_file_cell[i].weight_at_time;
        pointer++;
        char *char_pointer = (char *)pointer;

        for (int j = 0; j < time_str_size; j++)
        {
            *char_pointer = info_struct->info_file_cell[i].specific_time[j];
            char_pointer++;
            pointer++;
        }
    }
}

void deserialize_info_struct(struct Info_file_struct *info_struct, char *buffer)
{
    int *pointer = (int *)buffer;
    info_struct->current_index = *pointer;
    pointer++;

    for (int i = 0; i < CONFIG_MAX_INFO_VALUES; i++)
    {
        info_struct->info_file_cell[i].weight_at_time = *pointer;
        pointer++;
        char *char_pointer = (char *)pointer;

        for (int j = 0; j < time_str_size; j++)
        {
            info_struct->info_file_cell[i].specific_time[j] = *char_pointer;
            char_pointer++;
            pointer++;
        }
    }
}
