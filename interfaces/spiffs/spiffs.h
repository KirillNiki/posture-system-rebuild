

#ifndef MY_SPIFFS_H_
#define MY_SPIFFS_H_

#define SPIFFS_TAG "spiffs"
#define main_file "/index.html"
#define info_file_name "/info.bin"
#define buffer_size 20000 // in chars
#define file_count 10
#define file_name_length 40 // in cahrs

extern char file_strings[file_count][file_name_length];

void read_my_file(char *buffer, long size_of_buffer, char *path);
void read_bin_file(void);
void write_bin_file(void);

void list_partiotions(void);
void init_spiffs(void);
void join_path(char *result_path, char *path);
size_t read_chunk_file(FILE *file, char *buffer, long size_of_buffer);

#endif
