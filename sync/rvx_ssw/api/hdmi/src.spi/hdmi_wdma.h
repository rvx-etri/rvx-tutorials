#ifndef _H_HDMIWDMA_H_
#define _H_HDMIWDMA_H_

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


// HDMI RX���� ���� �����͸� �޸𸮿� Write��. .

#define HDMIWDMA_BASE	0x60000000

//  register
#define HDMIWDMA_EN			(HDMIWDMA_BASE + 0x00)
#define HDMIWDMA_ADDR		(HDMIWDMA_BASE + 0x04)
#define HDMIWDMA_SIZE	    (HDMIWDMA_BASE + 0x08)
#define HDMIWDMA_STRIDE		(HDMIWDMA_BASE + 0x0C)
#define HDMIWDMA_TYPE		(HDMIWDMA_BASE + 0x10)
#define HDMIWDMA_ENDIAN		(HDMIWDMA_BASE + 0x14)
#define HDMIWDMA_HPD		(HDMIWDMA_BASE + 0x18)
#define HDMIWDMA_D2R_RESET	(HDMIWDMA_BASE + 0x24)
#define HDMIWDMA_TEST		(HDMIWDMA_BASE + 0x28)
#define HDMIWDMA_INTR		(HDMIWDMA_BASE + 0x40)
#define HDMIWDMA_PLLLOCK	(HDMIWDMA_BASE + 0x50)


// define
#define HR_COLOR_ARGB	0
#define HR_COLOR_YCbCr  1

#define ALIGN1K(x)     ((((x) + 1023)>>10)<<10)

// structure
typedef struct {
	int hsize;
	int vsize;
	int stride;

	int type;
	uint32_t addr;
	int endian;
} sHWDMA;




// funciton
void hdmiwdma_init_param(sHWDMA *hw, int width, int height, int type);
void hdmiwdma_set_param(sHWDMA *hw);
void hdmiwdma_set_base(uint32_t addr);
void hdmiwdma_set_size(int width, int height);
void hdmiwdma_set_stride(int stride);
void hdmiwdma_set_type(int type);
void hdmiwdma_set_endian(int endian);
void hdmiwdma_set_hpd(int on);
void hdmiwdma_reset_dvi2rgb(int reset);
int  hdmiwmda_read_plllock(void);
void hdmiwmda_start(void);
void hdmiwmda_stop(void);



#endif