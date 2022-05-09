#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"


void init_bmp(Bmp* pic)
{
	pic->pixels = NULL;
}


static int count_padding(int w)
{
	return (w % 4 == 0) ? 0 : 4 - (3 * w) % 4;
}


Pix** alloc_two_dimention_array(int w, int h, int* rt)
{
	Pix** bmp = NULL;

	Pix* bmp_pixels = malloc(w * h * sizeof(Pix));
	if (bmp_pixels == NULL)
	{
		*rt = MALLOC_ERROR;
		return bmp;
	}
	
	bmp = malloc(h * sizeof(Pix*));
	if (bmp == NULL)
	{
		free(bmp_pixels);
		*rt = MALLOC_ERROR;
		return bmp;
	}
	
	int i;
	for (i = 0; i < h; i++)
		bmp[i] = &bmp_pixels[i * w];
	
	*rt = 0;
	
	return bmp;
}


static void change_header(BITMAPHEADER* header, int w, int h)
{
	int dw = count_padding(w);
	
	header->biSizeImage = (3 * w + dw) * h;
	header->bfOffBits = sizeof(BITMAPHEADER);
	header->bfSize = header->biSizeImage + header->bfOffBits;
	header->biWidth = w;
	header->biHeight = h;
}


int read_header(FILE* in_file, BITMAPHEADER* header)
{
	int err = 0;
	
	err = fread(&header->bfType, sizeof(BITMAPHEADER), 1, in_file);
	if (err != 1)
		return FILE_READING_ERROR;
	
	fseek(in_file, 0, SEEK_SET);
	return 0;
}


void free_two_dimention_pixel_array(Pix** array)
{
	if (array != NULL)
	{
		free(array[0]);
		free(array);
	}
}


int load_bmp(FILE* file, Bmp* pic)
{
	int i, rt, err;
	
	BITMAPHEADER picture_header;
	rt = read_header(file, &picture_header);
	if (rt != 0)
		return rt;
	
	int dw = count_padding(picture_header.biWidth);
	
	if (picture_header.biWidth <= 0 || picture_header.biHeight <= 0)
		return INVALID_PIC;
	
	Pix** bmp = alloc_two_dimention_array(picture_header.biWidth, picture_header.biHeight, &rt);
	if (rt != 0)
		return rt;
		
	fseek(file, picture_header.bfOffBits, SEEK_SET);

	for (i = 0; i < picture_header.biHeight; i++)
	{
		err = fread(bmp[i], 1, BYTES_PER_PIXEL * picture_header.biWidth, file);
		if (err != BYTES_PER_PIXEL * picture_header.biWidth)
			return FILE_READING_ERROR;
		
		fseek(file, dw, SEEK_CUR);
	}
	
	picture_header.bfSize = picture_header.biSizeImage + sizeof(BITMAPHEADER);
	picture_header.bfOffBits = sizeof(BITMAPHEADER);
	pic->header = picture_header;
	pic->pixels = bmp;
	
	return 0;
}


Bmp crop(Bmp* pic, int w, int h, int x, int y, int* rt)
{	
	Pix** rect = NULL;
	int i;
	
	Bmp piece;
	init_bmp(&piece);
	
	if (x < 0 || x >= x + w || x + w > pic->header.biWidth ||
		y < 0 || y >= y + h || y + h > pic->header.biHeight)
	{
		*rt = OUT_OF_RANGE;
		return piece;
	}
	
	rect = alloc_two_dimention_array(w, h, rt);
	if (*rt != 0)
		return piece;		

	for (i = 0; i < h; i++)
		memcpy(rect[i], pic->pixels[pic->header.biHeight - y - h + i] + x, w * BYTES_PER_PIXEL);
	
	piece.pixels = rect;
	piece.header = pic->header;
	
	change_header(&piece.header, w, h);
	
	return piece;
}


Bmp rotate(Bmp* rect, int* rt)
{
	BITMAPHEADER new_rect_header;
	new_rect_header = rect->header;
	
	change_header(&new_rect_header, rect->header.biHeight, rect->header.biWidth);

	Bmp rotated_pic;
	init_bmp(&rotated_pic);
	
	int i, j;
	
	
	Pix** rotated_rect = NULL;
	rotated_rect = alloc_two_dimention_array(new_rect_header.biWidth, new_rect_header.biHeight, rt);
	if (*rt != 0)
		return rotated_pic;
		
	Pix tmp;
	for (i = 0; i < new_rect_header.biHeight; i++)
	{
		for (j = 0; j < new_rect_header.biWidth; j++)
		{
			tmp = rect->pixels[j][rect->header.biWidth - 1 - i];
			rotated_rect[i][j] = tmp;
		}
	}
	
	rotated_pic.pixels = rotated_rect;
	rotated_pic.header = new_rect_header;
	
	return rotated_pic;
}


int save_bmp(FILE* out_file, Bmp* rect)
{
	int i, j;
	int dw = count_padding(rect->header.biWidth);
	char null_byte = '\0';
	fseek(out_file, 0, SEEK_SET);
	
	fwrite(&rect->header, sizeof(BITMAPHEADER), 1, out_file);
	
	for (i = 0; i < rect->header.biHeight; i++)
	{
		fwrite(rect->pixels[i], 3 * rect->header.biWidth, 1, out_file);
		for (j = 0; j < dw; j++)
			fwrite(&null_byte, 1, 1, out_file);
	}
	
	return 0;
}

