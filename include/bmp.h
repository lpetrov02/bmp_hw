#ifndef BMP_H_GUARD

#define BMP_H_GUARD

#include <stdint.h>

#define UNUSED(x) (void)(x)

#define MAX_BUF 10
#define BITS_PER_SYMBOL 5
#define MAKE_LAST_BIT_NULL 126
#define MESSAGE_READING_BLOCK_SIZE 50
#define CODE_OF_ZERO 48
#define BYTES_FROM_FILE_BEGIN 54
#define BYTE_SIZE 8
#define BYTES_PER_PIXEL 3

#define MALLOC_ERROR 1
#define FILE_ERROR 2
#define OUT_OF_RANGE 3
#define NOT_ENOUGH_OR_TOO_MANY_ARGS 4
#define INVALID_PIC 5
#define INVALID_KEY 6
#define FILE_READING_ERROR 7


typedef struct pixel_in_bmp
{
	char pixel[3];
} Pix;


#pragma pack(push, 1)
typedef struct tagBITMAPHEADER {
	uint16_t bfType;			// не менять
	uint32_t bfSize;
  	uint16_t bfReserved1;		// не менять
  	uint16_t bfReserved2;		// не менять
  	uint32_t bfOffBits;			// не менять
  	
  	uint32_t biSize;			// не менять
  	int32_t biWidth;
  	int32_t biHeight;
  	uint16_t biPlanes;			// не менять
  	uint16_t biBitCount;	   	// не менять
  	uint32_t biCompression;		// не менять
  	uint32_t biSizeImage;		// не менять
  	int32_t biXPelsPerMeter;	// не менять
  	int32_t biYPelsPerMeter;	// не менять
  	uint32_t biClrUsed;			// не менять
  	uint32_t biClrImportant;	// не менять
} BITMAPHEADER;
#pragma pack(pop)


typedef struct bmp_picture
{
	BITMAPHEADER header;
	Pix** pixels;
} Bmp;


void init_bmp(Bmp* pic);
Pix** alloc_two_dimention_array(int w, int h, int* rt);
Bmp crop(Bmp* pic, int w, int h, int x, int y, int* rt);
Bmp rotate(Bmp* rect, int* rt);
void free_two_dimention_pixel_array(Pix** array);
int read_header(FILE* in_file_pointer, BITMAPHEADER* header);
int load_bmp(FILE* file, Bmp* pic);
int save_bmp(FILE* out_file, Bmp* rect);


#endif
