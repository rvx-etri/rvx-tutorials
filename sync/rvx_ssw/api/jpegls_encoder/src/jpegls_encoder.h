#ifndef __ERVP_JPEGLS_ENCODER_H__
#define __ERVP_JPEGLS_ENCODER_H__

#include "ervp_image.h"

typedef enum {
	JPEGLSFMT_RGBINTRV = 0,
	JPEGLSFMT_YUV422 = 1,
} eIFMT;

typedef enum {
	FMT_YUYV = 0,
	FMT_UYVY = 1,
} eFMTYUV;

void jpeglsenc_set_param(int w, int h, int stride, int ifmt, int yuvfmt, unsigned int addr, unsigned int strmaddr);
void jpeglsenc_start(void);
int jpeglsenc_get_status(void);
void jpeglsenc_clear_status(void);
void jpeglsenc_set_endian_mask(int endian, int mask);
void jpeglsenc_set_imagesize(int width, int height);
void jpeglsenc_set_stride_fmt(int stride, int ifmt, int yuvfmt);
void jpeglsenc_set_addr(unsigned int addr);
void jpeglsenc_set_strmaddr(unsigned int addr);
int jpeglsenc_get_strmsize(void);
void jpeglsenc_wait_done(void);

ErvpImage *jpeglsenc_convert_image(const ErvpImage *before, ErvpImage *after);

#endif
