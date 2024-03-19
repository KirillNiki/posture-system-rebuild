

#ifndef MY_JSON_H_
#define MY_JSON_H_

#define max_json_buffer 1000
#define max_time_buffer 20

extern uint8_t json_time_buffer[max_time_buffer];
extern uint8_t json_data_buffer[max_json_buffer];
extern uint8_t train_data[1];

void build_json(void);
void set_train(void);
void set_time(char *time_string);

#endif
