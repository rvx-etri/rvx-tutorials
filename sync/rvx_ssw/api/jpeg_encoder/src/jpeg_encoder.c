#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_reg_util.h"
#include "ervp_delay.h"
#include "ervp_assert.h"
#include "ervp_variable_allocation.h"
#include "ervp_round_int.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_video_frame.h"

#include "jpeg_encoder.h"

#define JPEGENC_BASE				I_JPEG_ENCODER_SLAVE_BASEADDR

//  register
#define JPEGENC_IMGSIZE				(JPEGENC_BASE + 0x00)
#define JPEGENC_MCUNUM				(JPEGENC_BASE + 0x08)
#define JPEGENC_ENCOPT				(JPEGENC_BASE + 0x10)
#define JPEGENC_QINIT				(JPEGENC_BASE + 0x18)
#define JPEGENC_STRMCTRL			(JPEGENC_BASE + 0x20)
#define JPEGENC_BUSCTRL				(JPEGENC_BASE + 0x28)
#define JPEGENC_GENBITS				(JPEGENC_BASE + 0x30)
#define JPEGENC_REPLACE				(JPEGENC_BASE + 0x38)
#define JPEGENC_STRMBUF				(JPEGENC_BASE + 0x40)
#define JPEGENC_CURYBUF				(JPEGENC_BASE + 0x48)
#define JPEGENC_CURCBBUF			(JPEGENC_BASE + 0x50)
#define JPEGENC_CURCRBUF			(JPEGENC_BASE + 0x58)
#define JPEGENC_STRIDE				(JPEGENC_BASE + 0x70)
#define JPEGENC_SBUFSIZE			(JPEGENC_BASE + 0x78)
#define JPEGENC_ENCSTART			(JPEGENC_BASE + 0x80)
#define JPEGENC_ENCSTATE			(JPEGENC_BASE + 0x88)

#define ALIGN1K(x)     ((((x) + 1023)>>10)<<10)

#define JE_INTR_MASK	(0)
#define JE_RGBENDIAN	(0)
#define JE_INITQ		(176)
#define JE_STRMBURST	(15)
#define	JE_RNDROBIN		(1)	
#define JE_KEEP_1KB     (1)
#define JE_BIG_ENDIAN	(0)
#define JE_SBUFSIZE		(0x100000)

 // average compressed ratio is 10:1
const int BUFFER_SIZE = (VIDEO_FRAME_MAX_SIZE/8);
static char* buffer;

static void __attribute__ ((constructor)) init_jpeg_buffer()
{
  buffer = malloc_permanent(BUFFER_SIZE, 1);
}

ErvpImage *jpegenc_convert_image(const ErvpImage *before, ErvpImage *after)
{
  ErvpImage* result;
	int fmt;
	int width = before->width;
	int height = before->height;
	int ystride, cstride;

	assert(before!=NULL);

	if(before->format == IMAGE_FMT_RGB_565_PACKED)
	{
		fmt = JPEGFMT_RGB565;
		ystride = before->stride[0];
		cstride = 0;
	}
	else if(before->format == IMAGE_FMT_YUV_420_PLANAR_YV12)
	{
		fmt = JPEGFMT_YUV420;
		ystride = before->stride[0];
		cstride = before->stride[1];
	}
	else
	{
		assert_not_implemented();
	}

	jpegenc_set_param(width, height, ystride, cstride, fmt);
	jpegenc_set_strmbuf((unsigned int)(buffer));
	jpegenc_set_imgbufy((unsigned int)(before->addr[0]));
	if(before->format == IMAGE_FMT_YUV_420_PLANAR_YV12)
	{
		jpegenc_set_imgbufcb((unsigned int)(before->addr[1]));
		jpegenc_set_imgbufcr((unsigned int)(before->addr[2]));
	}
	jpegenc_start();
	jpegenc_wait_done();

  int encoded_size = round_up_int(jpegenc_get_genbits(), 8) >> 3;
  int allocated_size = round_up_int(encoded_size,4);
  assert(encoded_size<=BUFFER_SIZE);
  if(after == NULL)
  {
		result = image_alloc_wo_internals(width, height, IMAGE_FMT_JPEG);
    result->addr[0] = malloc(allocated_size);
		assert(result->addr[0]);
  }
  else
  {
    assert_not_implemented();
  }
  memcpy(result->addr[0], buffer, encoded_size);
	result->file_size = allocated_size;
	return result;
}

size_t jpegenc_convert_to_file(const ErvpImage *image, const char* name)
{
	FAKEFILE* result;
	int fmt;
	int width = image->width;
	int height = image->height;
	int ystride, cstride;

	assert(image!=NULL);

	if(image->format == IMAGE_FMT_RGB_565_PACKED)
	{
		fmt = JPEGFMT_RGB565;
		ystride = image->stride[0];
		cstride = 0;
	}
	else if(image->format == IMAGE_FMT_YUV_420_PLANAR_YV12)
	{
		fmt = JPEGFMT_YUV420;
		ystride = image->stride[0];
		cstride = image->stride[1];
	}
	else
	{
		assert_not_implemented();
	}

	jpegenc_set_param(width, height, ystride, cstride, fmt);
	jpegenc_set_strmbuf((unsigned int)(buffer));
	jpegenc_set_imgbufy((unsigned int)(image->addr[0]));
	if(image->format == IMAGE_FMT_YUV_420_PLANAR_YV12)
	{
		jpegenc_set_imgbufcb((unsigned int)(image->addr[1]));
		jpegenc_set_imgbufcr((unsigned int)(image->addr[2]));
	}
	jpegenc_start();
	jpegenc_wait_done();

	int encoded_size = round_up_int(jpegenc_get_genbits(), 8) >> 3;
	int allocated_size = round_up_int(encoded_size,4);
	int file_size;
	assert(encoded_size<=BUFFER_SIZE);

	uint8_t* raw_data = malloc(allocated_size);
	assert(raw_data);
	memcpy(raw_data, buffer, allocated_size);
	file_size = fakefile_generate(raw_data, allocated_size, name);
	assert(file_size==allocated_size);
	return allocated_size;
}

void jpegenc_set_param(int w, int h, int ystride, int cstride, int fmt)
{
	jpegenc_set_imgsize(w, h);
	jpegenc_set_calcmcu(w, h, fmt);
	jpegenc_set_encopt(JE_INTR_MASK, fmt, JE_RGBENDIAN, 0);
	jpegenc_set_qinit(JE_INITQ);
	jpegenc_set_strmctrl(JE_STRMBURST);
	jpegenc_set_busctrl(JE_BIG_ENDIAN, JE_KEEP_1KB, JE_RNDROBIN);
	jpegenc_set_replace(0, 0, 0, 0);
	jpegenc_set_stride(ystride, cstride);
	jpegenc_set_sbufsize(JE_SBUFSIZE);


	//jpegenc_set_strmbuf(unsigned int addr);
	//jpegenc_set_imgbufy(unsigned int addr);
	//jpegenc_set_imgbufcb(unsigned int addr);
	//jpegenc_set_imgbufcr(unsigned int addr);
}

void jpegenc_set_imgsize(int width, int height)
{
	REG32(JPEGENC_IMGSIZE) = (width<<16) | (height);
}

void jpegenc_set_mcunum(int num)
{
	REG32(JPEGENC_MCUNUM) = num;
}

void jpegenc_set_calcmcu(int width, int height, int fmt)
{
	int mbnum;
	int vsize;
	int hsize = ((width + 15)/16)*16;

	if(fmt) {
		vsize = ((height + 7)/8)*8;
		mbnum = (hsize * vsize) / 128;
	}
	else {
		vsize = ((height + 15)/16)*16;
		mbnum = (hsize * vsize) / 256;
	}

	jpegenc_set_mcunum(mbnum);
}

void jpegenc_set_encopt(int intrmask, int chrfmt, int rgbendian, int rotopt)
{
	REG32(JPEGENC_ENCOPT) = (rotopt<<4) | (rgbendian<<3) | (chrfmt<<1) | (intrmask);
}

void jpegenc_set_qinit(int q)
{
	REG32(JPEGENC_QINIT) = q;
}

void jpegenc_set_strmctrl(int burst)
{

	REG32(JPEGENC_STRMCTRL) = burst;
}

void jpegenc_set_busctrl(int endian, int keep4k, int rndrobin)
{
	REG32(JPEGENC_BUSCTRL) = (rndrobin<<2) | (keep4k<<1) | (endian);
}

int jpegenc_get_genbits(void)
{
	return REG32(JPEGENC_GENBITS);
}

void jpegenc_set_replace(int yrep, int urep, int vrep, int yuvsel)
{
	REG32(JPEGENC_REPLACE) = (yuvsel<<24) | (yrep<<16) | (urep<<8) | (vrep);
}

void jpegenc_set_strmbuf(unsigned int addr)
{
	REG32(JPEGENC_STRMBUF) = addr;
}

void jpegenc_set_imgbufy(unsigned int addr)
{
	REG32(JPEGENC_CURYBUF) = addr;
}

void jpegenc_set_imgbufcb(unsigned int addr)
{
	REG32(JPEGENC_CURCBBUF) = addr;
}

void jpegenc_set_imgbufcr(unsigned int addr)
{
	REG32(JPEGENC_CURCRBUF) = addr;
}

void jpegenc_set_stride(int ystride, int cstride)
{
	REG32(JPEGENC_STRIDE) = (ystride<<16) | (cstride);
}

void jpegenc_set_sbufsize(unsigned int size)
{
	REG32(JPEGENC_SBUFSIZE) = size;
}

void jpegenc_start(void)
{
	REG32(JPEGENC_ENCSTART) = 1;
}

int jpegenc_get_status(void)
{
	return REG32(JPEGENC_ENCSTATE);
}

void jpegenc_clear_status(void)
{
	return REG32(JPEGENC_ENCSTATE) = 1;
}


void jpegenc_wait_done(void)
{
	int status;

	while(1) {
		delay_ms(1);
		status = jpegenc_get_status();
		if(status != 0)
			break;
	}
	jpegenc_clear_status();
}
