#pragma once

#include "ervp_image.h"
#include "ervp_image_file.h"
#include "ervp_fakefile.h"

typedef enum {
	JPEGFMT_YUV420 = 0,
	JPEGFMT_YUV422 = 1,
	JPEGFMT_RGB565 = 2,
} eJPEGFMT;

//
void jpegenc_set_param(int w, int h, int ystride, int cstride, int fmt);
void jpegenc_set_imgsize(int width, int height);
void jpegenc_set_mcunum(int num);
void jpegenc_set_calcmcu(int width, int height, int fmt);
void jpegenc_set_encopt(int intrmask, int chrfmt, int rgbendian, int rotopt);
void jpegenc_set_qinit(int q);
void jpegenc_set_strmctrl(int burst);
void jpegenc_set_busctrl(int endian, int keep4k, int rndrobin);
int jpegenc_get_genbits(void);
void jpegenc_set_replace(int yrep, int urep, int vrep, int yuvsel);
void jpegenc_set_strmbuf(unsigned int addr);
void jpegenc_set_imgbufy(unsigned int addr);
void jpegenc_set_imgbufcb(unsigned int addr);
void jpegenc_set_imgbufcr(unsigned int addr);
void jpegenc_set_stride(int ystride, int cstride);
void jpegenc_set_sbufsize(unsigned int size);
void jpegenc_start(void);
int jpegenc_get_status(void);
void jpegenc_clear_status(void);
void jpegenc_wait_done(void);

ErvpImage *jpegenc_convert_image(const ErvpImage *before, ErvpImage *after);
size_t jpegenc_convert_to_file(const ErvpImage *image, const char* name);
