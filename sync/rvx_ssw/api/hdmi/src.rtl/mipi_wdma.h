#ifndef _H_MIPIWDMA_H_
#define _H_MIPIWDMA_H_

#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_external_peri_group_memorymap_offset.h"
#include "frvp_spi.h"
#include "ervp_reg_util.h"
#include "ervp_misc_util.h"
#include "types.h"


// HDMI TX�� ���� �޸𸮿��� RGBToDVI ���� �����͸� �־��ִ� ������ ��.

#define MIPIWDMA_BASE	0x60000000

//  register
#define MIPIWDMA_EN			(MIPIWDMA_BASE + 0x00)
#define MIPIWDMA_ADDR		(MIPIWDMA_BASE + 0x04)
#define MIPIWDMA_SIZE	    (MIPIWDMA_BASE + 0x08)
#define MIPIWDMA_STRIDE		(MIPIWDMA_BASE + 0x0C)
#define MIPIWDMA_TYPE		(MIPIWDMA_BASE + 0x10)
#define MIPIWDMA_ENDIAN		(MIPIWDMA_BASE + 0x14)
#define MIPIWDMA_SENSORPWND	(MIPIWDMA_BASE + 0x18)
//#define MIPIWDMA_MIPIPHY	(MIPIWDMA_BASE + 0x1C)
//#define MIPIWDMA_MIPICIS	(MIPIWDMA_BASE + 0x20)
//#define MIPIWDMA_BAYER2RGB	(MIPIWDMA_BASE + 0x24)
#define MIPIWDMA_AXI0		(MIPIWDMA_BASE + 0x1C)
#define MIPIWDMA_AXI1		(MIPIWDMA_BASE + 0x20)
#define MIPIWDMA_TEST	    (MIPIWDMA_BASE + 0x24)
#define MIPIWDMA_PATTERN	(MIPIWDMA_BASE + 0x28)


#define MIPIWDMA_STATUS		(MIPIWDMA_BASE + 0x40)

#define MIPIWDMA_DBG50		(MIPIWDMA_BASE + 0x50)
#define MIPIWDMA_DBG54		(MIPIWDMA_BASE + 0x54)

// define
#define HR_COLOR_ARGB	0
#define HR_COLOR_YCbCr  1

#define ALIGN1K(x)     ((((x) + 1023)>>10)<<10)


// structure
typedef struct {
	int hsize;
	int vsize;
	int stride;

	uint32_t addr;
	int type;
	int endian;
} sMWDMA;




// funciton
void mipiwdma_init_param(sMWDMA *mw, int width, int height, int type);
void mipiwdma_set_param(sMWDMA *mw);
void mipiwdma_set_base(uint32_t addr);
void mipiwdma_set_size(int width, int height);
void mipiwdma_set_stride(int stride);
void mipiwdma_set_type(int type);
void mipiwdma_set_endian(int endian);
void mipiwdma_set_sensor_pwdn(int on);



#endif