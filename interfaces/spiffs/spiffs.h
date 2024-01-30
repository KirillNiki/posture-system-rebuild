

#ifndef MY_SPIFFS_H_
#define MY_SPIFFS_H_

#define SPIFFS_TAG "spiffs"
#define main_file "/index.html"
#define info_file_name "/info.txt"
#define buffer_size 20000 // in chars
#define file_count 10
#define file_name_length 40 // in cahrs

extern char file_strings[file_count][file_name_length];


void write_file(char *path, char *line);
void append_file(char *path, char *line);
void read_file(char *buffer, long size_of_buffer, char *path);

void list_partiotions(void);
void init_spiffs(void);

#endif
