#ifndef STEGO_GUARD

#define STEGO_GUARD

#include "bmp.h"

int read_and_insert_message(int arg_num, char** arg_values, FILE* input_file);
int extract_message(int arg_num, char** arg_values, FILE* input_file);
char* read_message(char* message_file_name, int* rt);
int insert_message(char* key_file_name, char* message, Bmp* pic);
int get_key_file_length(FILE* file);
int get_message(char* message, Bmp* pic, FILE* key);
int save_message(char* file_name, char* message);

#endif
