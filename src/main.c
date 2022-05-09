#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bmp.h"
#include "stego.h"


typedef struct working_mode_t {
	FILE* f;
	int (*action)(int, char**, FILE*);
} Working_mode;


int crop_rotate(int arg_num, char** arg_values, FILE* input_file)
{
	int rt = 0;
	FILE* out_file = NULL;

	if (arg_num != 5)
	{
		rt = NOT_ENOUGH_OR_TOO_MANY_ARGS;
		goto out;
	}

	int w = atoi(arg_values[3]), 
		h = atoi(arg_values[4]), 
		x = atoi(arg_values[1]), 
		y = atoi(arg_values[2]);
	
	BITMAPHEADER header; 
	rt = read_header(input_file, &header);
	if (rt != 0)
		goto out;
	
	if (header.biWidth < 0 || header.biHeight <= 0)
	{
		rt = INVALID_PIC;
		goto out;
	}
	
	Bmp pic, rect, rotated_rect;
	rt = load_bmp(input_file, &pic);
	if (rt != 0)
		goto out;
	
	rect = crop(&pic, w, h, x, y, &rt);
	free_two_dimention_pixel_array(pic.pixels);
	if (rt != 0)
		goto out;
		
	rotated_rect = rotate(&rect, &rt);
	free_two_dimention_pixel_array(rect.pixels);
	if (rt != 0)
		goto out;
		
	// теперь откроем файл для сохранения картинки
	out_file = fopen(arg_values[0], "wb");
	if (out_file == NULL)
	{
		rt = FILE_ERROR;
		goto out;
	}

	rt = save_bmp(out_file, &rotated_rect);
	
	free_two_dimention_pixel_array(rotated_rect.pixels);
	
	goto out;
	
	out:
		if (out_file)
			fclose(out_file);
		return rt;
}


int read_and_insert_message(int arg_num, char** arg_values, FILE* input_file)
{
	int rt = 0;
	FILE* out_file = NULL;
	Bmp pic;
	init_bmp(&pic);
	
	if (arg_num != 3)
	{
		rt = NOT_ENOUGH_OR_TOO_MANY_ARGS;
		goto out;
	}
	
	out_file = fopen(arg_values[0], "wb");
	if (out_file == NULL)
	{
		rt = FILE_ERROR;
		goto out;
	}
	
	char* message = read_message(arg_values[2], &rt);
	if (rt != 0)
		goto out;
		
	rt = load_bmp(input_file, &pic);
	if (rt != 0)
		goto out;
	
	rt = insert_message(arg_values[1], message, &pic);
	if (rt != 0)
		goto out;
		
	rt = save_bmp(out_file, &pic);
	
	goto out;
	
	out:
		free_two_dimention_pixel_array(pic.pixels);
		if (out_file)
			fclose(out_file);
		free(message);
		return rt;
}


int extract_message(int arg_num, char** arg_values, FILE* input_file)
{
	int rt = 0;
	
	FILE* key = NULL;
	if (arg_num != 2)
	{
		rt = NOT_ENOUGH_OR_TOO_MANY_ARGS;
		goto out;
	}
	
	Bmp pic;
	char* message = NULL;
	int size;
	
	rt = load_bmp(input_file, &pic);
	if (rt != 0)
		goto out;
	
	key = fopen(arg_values[0], "r");
	if (key == NULL)
	{
		rt = FILE_ERROR;
		goto out;
	}
	
	size = get_key_file_length(key);

	message = malloc(size + 1);
	if (message == NULL)
	{
		rt = MALLOC_ERROR;
		goto out;
	}
	
	rt = get_message(message, &pic, key);
	if (rt != 0)
		goto out;
		
	rt = save_message(arg_values[1], message);
	if (rt != 0)
		goto out;
	
	
	goto out;
	
	out:
		free_two_dimention_pixel_array(pic.pixels);
		if (key)
			fclose(key);
		free(message);
		return rt;
}



int main(int argc, char** argv)
{	
	int rt = 0;
	Working_mode mode;
	
	if (argc < 2)
	{
		rt = -1;
		goto out;
	}
	
	if (!strcmp(argv[1], "crop-rotate"))
	{
		mode.f = fopen(argv[2], "rb");
		mode.action = crop_rotate;
	}
	else if (!strcmp(argv[1], "insert"))
	{
		mode.f = fopen(argv[2], "rb");
		mode.action = read_and_insert_message;
	}
	else if (!strcmp(argv[1], "extract"))
	{
		mode.f = fopen(argv[2], "rb");
		mode.action = extract_message;
	}
	else
	{
		printf("Unknown command\n");
		rt = -1;
		goto out;
	}
	
	if (mode.f == NULL)
	{
		printf("File opening error\n");
		rt = -1;
		goto out;
	}
	
	rt = mode.action(argc - 3, argv + 3, mode.f);
	
	if (rt == MALLOC_ERROR)
		printf("Memory allocation error!\n");
	if (rt == FILE_ERROR)
		printf("File opening error!\n");
	if (rt == OUT_OF_RANGE)
		printf("Error! Coordinates out of range!\n");
	if (rt == NOT_ENOUGH_OR_TOO_MANY_ARGS)
		printf("Invalid number of arguments!\n");
	if (rt == INVALID_PIC)
		printf("Your bmp is invalid((\n");
	if (rt == INVALID_KEY)
		printf("Invalid key!\n");
	if (rt == FILE_READING_ERROR)
		printf("File reading error!\n");
	
	goto out;
	
	out:
	{	
		if (mode.f)
			fclose(mode.f);
		return rt;
	}
}
