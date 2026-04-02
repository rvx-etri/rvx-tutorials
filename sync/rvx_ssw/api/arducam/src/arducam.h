#ifndef __ARDUCAM_H__
#define __ARDUCAM_H__

#include "ervp_image.h"
#include <stdint.h>

#define ARDUCAM_MIRROR                              0
#define ARDUCAM_FLIP                                1
#define ARDUCAM_MIRROR_FLIP                         2
#define ARDUCAM_NORMAL                              3

//#define ARDUCAM_RGB 	0
//#define ARDUCAM_YUV 	1
//#define ARDUCAM_JPEG	2

#define ARDUCAM_320x240 	0	//320x240
#define ARDUCAM_640x480		1	//640x480
#define ARDUCAM_1024x768	2	//1024x768
#define ARDUCAM_1280x960 	3	//1280x960
#define ARDUCAM_1600x1200	4	//1600x1200
#define ARDUCAM_2048x1536	5	//2048x1536
#define ARDUCAM_2592x1944	6	//2592x1944
#define ARDUCAM_1920x1080	7

typedef struct _IMG
{
	unsigned char A;
	unsigned char B;
	unsigned char C;
} IMG;

typedef struct _YUV
{
	unsigned char Y;
	unsigned char U;
	unsigned char V;
} YUV;

typedef struct _RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

typedef enum {
    ARDUCAM_YUV_422_QVGA = 0,
    ARDUCAM_YUV_422_VGA  = 1,
    ARDUCAM_RGB_565_QVGA  = 2,
    ARDUCAM_RGB_565_VGA  = 3,
    ARDUCAM_RGB_565_SVGA  = 4,
    ARDUCAM_YUV_444_QVGA = 5,
    ARDUCAM_YUV_444_VGA = 6,
    ARDUCAM_RGB_888_QVGA  = 7,
    ARDUCAM_RGB_888_VGA  = 8,
} ArducamFormatSize;

void arducam_init(ArducamFormatSize fs);
void arducam_config_spi();
ErvpImage* arducam_alloc_image(ErvpImage* prepared);
void arducam_config_sensor(ArducamFormatSize fs);
void arducam_set_mirror_flip(uint8_t mirror_flip);

void arducam_single_capture(ErvpImage *image);
int arducam_capture_single_rgb(unsigned char *rgb565, int width, int height, int sample);
int arducam_capture_single(YUV *img, int width, int height, int sample);
int arducam_capture_single_yuv(uint8_t *img, int width, int height, int stride);

void InitCAM(uint8_t m_fmt);
void arducam_set_jpeg_size(uint8_t size);
uint32_t get_jpeg(unsigned char *jpeg);

// test
void arducam_check_test_reg(void);
void arducam_test(unsigned char *img);
void arducam_disp_test(void);
void arducam_set_test_color_bar();
#endif
