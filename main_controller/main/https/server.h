

#include "adc/adc.h"

#ifndef SERVER_H_
#define SERVER_H_

#define httpd_stack_size 30000
#define max_handlers 15

void init_server(void);

#endif
