

#ifndef MY_DATA_INTREFACE_H_
#define MY_DATA_INTREFACE_H_
#include "sdkconfig.h"
#include "adc/adc.h"

struct Info_file_cell
{
    int weight_at_time;
    int unix_time;
};
struct Info_file_struct
{
    int current_index;
    struct Info_file_cell info_file_cell[CONFIG_MAX_INFO_VALUES];
};

extern struct Info_file_struct info_file;
extern int weights[gpios_num];

#endif
