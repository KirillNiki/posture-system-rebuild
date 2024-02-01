

#ifndef MY_DATA_INTREFACE_H_
#define MY_DATA_INTREFACE_H_
#include "sdkconfig.h"
#include "adc/adc.h"
#include "ds1302/ds1302.h"

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

extern ds1302_t rtc_dev;
extern struct Info_file_struct info_file;
extern int weights[gpios_num];
extern int sitting_timer;

extern bool is_synchronized;
extern bool is_train;

#endif
