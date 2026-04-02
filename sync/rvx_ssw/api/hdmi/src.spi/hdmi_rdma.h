#ifndef _H_HDMIRDMA_H_
#define _H_HDMIRDMA_H_

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

#define HDMIRDMA_BASE	0x61000000

//  register
#define HDMIRDMA_EN			(HDMIRDMA_BASE + 0x00)
#define HDMIRDMA_ADDR0		(HDMIRDMA_BASE + 0x04)
#define HDMIRDMA_ADDR1		(HDMIRDMA_BASE + 0x08)
#define HDMIRDMA_IDX		(HDMIRDMA_BASE + 0x0C)
#define HDMIRDMA_SIZE	    (HDMIRDMA_BASE + 0x10)
#define HDMIRDMA_STRIDE		(HDMIRDMA_BASE + 0x14)
#define HDMIRDMA_TYPE		(HDMIRDMA_BASE + 0x18)
#define HDMIRDMA_SYNCSIZE	(HDMIRDMA_BASE + 0x1C)
#define HDMIRDMA_VPORCH		(HDMIRDMA_BASE + 0x20)
#define HDMIRDMA_HPORCH		(HDMIRDMA_BASE + 0x24)
#define HDMIRDMA_SYNCPOL	(HDMIRDMA_BASE + 0x28)
#define HDMIRDMA_ENDIAN		(HDMIRDMA_BASE + 0x2C)
#define HDMIRDMA_PATTERN	(HDMIRDMA_BASE + 0x30)


// define
#define HR_COLOR_ARGB	0
#define HR_COLOR_YCbCr  1

#define ALIGN1K(x)     ((((x) + 1023)>>10)<<10)


// structure
typedef struct {
	int hsize;
	int vsize;
	int stride;

	uint32_t addr0;
	uint32_t addr1;
	int idx;

	int type;

	int hbp, hfp; // back porch, front porch
	int vbp, vfp;
	int hss, vss; // sync size

	int h_pol;
	int v_pol;
	int hsync_mask;

	int endian;
} sHRDMA;




// funciton
void hdmirdma_init_param(sHRDMA *hr, int width, int height, int type);
void hdmirdma_set_param(sHRDMA *hr);
void hdmirdma_start(void);
void hdmirdma_stop(void);
void hdmirdma_set_base0(uint32_t addr);
void hdmirdma_set_base1(uint32_t addr);
void hdmirdma_set_idx(int idx);
void hdmirdma_set_size(int width, int height);
void hdmirdma_set_stride(int stride);
void hdmirdma_set_type(int type);
void hdmirdma_set_syncsize(int hs, int vs);
void hdmirdma_set_vporch(int vbp, int vfp);
void hdmirdma_set_hporch(int hbp, int hfp);
void hdmirdma_set_syncpol(int hpol, int vpol, int hsmask);
void hdmirdma_set_endian(int endian);
void hdmirdma_set_pattern(int en, int r, int g, int b);





#endif