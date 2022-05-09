#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stego.h"
#include "bmp.h"


static int rgb_to_number(char rgb)
{
	if (rgb == 'R')
		return 0;
	if (rgb == 'G')
		return 1;
	if (rgb == 'B')
		return 2;
	return -1;
}


static char new_code(char c)
{
	if (c == ' ')
		return 0;
	if (c == ',')
		return 1;
	if (c == '.')
		return 2;
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 3;
	return -1;
}


static char old_code(char c)
{
	if (c == 0)
		return ' ';
	if (c == 1)
		return ',';
	if (c == 2)
		return '.';
	if (c >= 3 && c <= 28)
		return c + 'A' - 3;
	return -1;
}


char* read_message(char* message_file_name, int* rt)
{
	FILE* msg = NULL;
	msg = fopen(message_file_name, "r");
	if (msg == NULL)
	{
		*rt = FILE_ERROR;
		goto out;
	}
	
	int capacity = MESSAGE_READING_BLOCK_SIZE;
	int block_capacity = MESSAGE_READING_BLOCK_SIZE;
	int size = 0;
	int j = 0;
	int i;
	char* message_code = (char*)malloc(BITS_PER_SYMBOL * capacity);
	if (message_code == NULL)
	{
		*rt = MALLOC_ERROR;
		goto out;
	}
	
	char c;
	while (fread(&c, 1, 1, msg) == 1 && c != '\n')
	{
		size++;
		c = new_code(c);
		
		for (i = 0; i < BITS_PER_SYMBOL; i++)
			message_code[j++] = ((c & (1 << i)) == (1 << i)) + CODE_OF_ZERO;
		
		if (size == capacity)
		{
			capacity += block_capacity;
			message_code = (char*)realloc(message_code, capacity * BITS_PER_SYMBOL);
			if (message_code == NULL)
			{
				*rt = MALLOC_ERROR;
				goto out;
			}
		}
	}
	message_code[size * BITS_PER_SYMBOL] = 0;
	
	goto out;
	
	out:
		if (msg)
			fclose(msg);	
		return message_code;	
}


int get_key_file_length(FILE* file)
{
	fseek(file, 0, SEEK_SET);
	int res = 0;
	char c;
	
	while (fread(&c, 1, 1, file) == 1)
		if (c == '\n')
			res++;
			
	return res; 
}


int insert_message(char* key_file_name, char* message, Bmp* pic)
{
	int rt = 0;
	int i = 0;
	int err;
	
	int message_length = strlen(message);
	FILE* key = NULL;
	
	key = fopen(key_file_name, "r");
	if (key == NULL)
	{
		rt = FILE_ERROR;
		goto out;
	}
	
	int x, y, color_channel, bit;
	char channel;
	for (i = 0; i < message_length; i++)
	{
		err = fscanf(key, "%i %i %c", &x, &y, &channel);
		if (err != 3)
		{
			rt = INVALID_KEY;
			goto out;
		}
		color_channel = rgb_to_number(channel);
		if (x < 0 || x >= pic->header.biWidth || y < 0 || y >= pic->header.biHeight)
		{
			rt = INVALID_KEY;
			goto out;
		}
		bit = message[i] - CODE_OF_ZERO;
		color_channel = rgb_to_number(channel);
		pic->pixels[pic->header.biHeight - 1 - y][x].pixel[color_channel] = \
		(pic->pixels[pic->header.biHeight - 1 - y][x].pixel[color_channel] & MAKE_LAST_BIT_NULL) | bit;
	}
	
	goto out;
	
	out:
		if (key)
			fclose(key);
		return rt;
}


int get_message(char* message, Bmp* pic, FILE* key)
{
	fseek(key, 0, SEEK_SET);
	
	int x, y, color_channel, bit;
	int i = 0;
	char channel;
	
	while (fscanf(key, "%i %i %c", &x, &y, &channel) == 3)
	{
		if (x < 0 || x >= pic->header.biWidth || y < 0 || y >= pic->header.biHeight)
			return INVALID_KEY;
		color_channel = rgb_to_number(channel);
		bit = pic->pixels[pic->header.biHeight - 1 - y][x].pixel[color_channel];
		
		message[i++] = (bit & 1) + CODE_OF_ZERO;
	}
	message[i] = 0;
	
	return 0;
}


int save_message(char* file_name, char* message)
{
	int rt = 0;
	FILE* msg = NULL;
	msg = fopen(file_name, "w");
	if (msg == NULL)
	{
		rt = FILE_ERROR;
		goto out;
	}

	char symbol = 0;
	int j = 0;
	size_t i, size = strlen(message);
	for (i = 0; i < size; i++)
	{
		symbol |= (message[i] - CODE_OF_ZERO) << j++;
		if (j == 5)
		{
			symbol = old_code(symbol);
			fprintf(msg, "%c", symbol);
			j = 0;
			symbol = 0;
		}
	}
	
	fprintf(msg, "\n");	
	out:
		if (msg)
			fclose(msg);
		return rt;
}

