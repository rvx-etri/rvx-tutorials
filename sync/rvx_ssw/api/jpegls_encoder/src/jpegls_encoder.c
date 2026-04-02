#include <stdio.h>
#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_reg_util.h"
#include "ervp_delay.h"
#include "jpegls_encoder.h"

#define JLSENC_BASE             I_JPEGLS_ENCODER_SLAVE_BASEADDR
//  register
#define JLSENC_COMMAND          (JLSENC_BASE + 0x00)
#define JLSENC_STATUS           (JLSENC_BASE + 0x00)
#define JLSENC_SYSTEM           (JLSENC_BASE + 0x04)
#define JLSENC_IMAGESIZE        (JLSENC_BASE + 0x08)
#define JLSENC_FORMATSTRIDE     (JLSENC_BASE + 0x0C)
#define JLSENC_ADDR             (JLSENC_BASE + 0x10)
#define JLSENC_STRMADDR         (JLSENC_BASE + 0x14)
#define JLSENC_STRMSIZE         (JLSENC_BASE + 0x18)

ErvpImage *jpeglsenc_convert_image(const ErvpImage *before, ErvpImage *after)
{
    int fmt;
    int width = before->width;
    int height = before->height;
    int stride;
	int img_w, img_h, img_type;

    if(before == NULL)
    {
        printf("[%s] input image is null\n", __func__);
        return 0;
    }

	if((width%8) != 0)
	{
        printf("The width of the input image must be a multiple of 8.\n");
        return 0;
	}

    if(before->format == IMAGE_FMT_XBGR_8888_PACKED)
    {
        fmt = JPEGLSFMT_RGBINTRV;
        stride = before->stride[0];
    }
    else if(before->format == IMAGE_FMT_YUV_422_PACKED)
    {
        fmt = JPEGLSFMT_YUV422;
        stride = before->stride[0];
    }
    else
    {
        printf("unsupported input format!\n");
        return 0;
    }

    if(after == NULL)
        after = image_alloc(width, height, IMAGE_FMT_JPEGLS);

	jpeglsenc_set_param(width, height, stride, fmt, FMT_YUYV, before->addr[0], after->addr[0]);
	jpeglsenc_start();
	jpeglsenc_wait_done();
	after->file_size = jpeglsenc_get_strmsize();
	printf("jpegls size: %d\n", after->file_size);

    get_jpeg_info(after->addr[0], &img_w, &img_h, &img_type);
	if(before->format == IMAGE_FMT_XBGR_8888_PACKED)
	{
		if(img_w != width || img_h != height || img_type!=0) { // type=1=yuv
			printf("bad jpeg header. %d %d %d\n", img_w, img_h, img_type);
		}
	}
    else if(before->format == IMAGE_FMT_YUV_422_PACKED)
	{
		if(img_w != width || img_h != height || img_type!=1) { // type=1=yuv
			printf("bad jpeg header. %d %d %d\n", img_w, img_h, img_type);
		}
	}

    return after;
}

void jpeglsenc_set_param(int w, int h, int stride, int ifmt, int yuvfmt, unsigned int addr, unsigned int strmaddr)
{
	jpeglsenc_set_endian_mask(0, 0);
	jpeglsenc_set_imagesize(w, h);
	jpeglsenc_set_stride_fmt(stride, ifmt, yuvfmt);
	jpeglsenc_set_addr(addr);
	jpeglsenc_set_strmaddr(strmaddr);
}

void jpeglsenc_start(void)
{
	REG32(JLSENC_COMMAND) = 1;
}

int jpeglsenc_get_status(void)
{
	return REG32(JLSENC_STATUS);
}

void jpeglsenc_clear_status(void)
{
	REG32(JLSENC_COMMAND) = (1<<1);
}

void jpeglsenc_set_endian_mask(int endian, int mask)
{
	REG32(JLSENC_SYSTEM) = (mask<<1) | (endian);
}

void jpeglsenc_set_imagesize(int width, int height)
{
	REG32(JLSENC_IMAGESIZE) = (width<<12) | (height);
}

void jpeglsenc_set_stride_fmt(int stride, int ifmt, int yuvfmt)
{
	REG32(JLSENC_FORMATSTRIDE) = (yuvfmt<<17) | (ifmt<<16) | (stride);
}

void jpeglsenc_set_addr(unsigned int addr)
{
	REG32(JLSENC_ADDR) = addr;
}

void jpeglsenc_set_strmaddr(unsigned int addr)
{
	REG32(JLSENC_STRMADDR) = addr;
}

int jpeglsenc_get_strmsize(void)
{
	return REG32(JLSENC_STRMSIZE);
}

void jpeglsenc_wait_done(void)
{
	int status;

	while(1) {
		delay_ms(1);
		status = jpeglsenc_get_status();
		if(status != 0)
			break;
	}
	jpeglsenc_clear_status();
}

void jpeglsenc_print_reg(void)
{
    printf("JLSENC_COMMAND      = %08X\n",  REG32(JLSENC_COMMAND    ));
    printf("JLSENC_STATUS       = %08X\n",  REG32(JLSENC_STATUS     ));
    printf("JLSENC_SYSTEM       = %08X\n",  REG32(JLSENC_SYSTEM     ));
    printf("JLSENC_IMAGESIZE    = %08X\n",  REG32(JLSENC_IMAGESIZE  ));
    printf("JLSENC_FORMATSTRIDE = %08X\n",  REG32(JLSENC_FORMATSTRIDE));
    printf("JLSENC_ADDR         = %08X\n",  REG32(JLSENC_ADDR       ));
    printf("JLSENC_STRMADDR     = %08X\n",  REG32(JLSENC_STRMADDR   ));
    printf("JLSENC_STRMSIZE     = %08X\n",  REG32(JLSENC_STRMSIZE   ));
}
