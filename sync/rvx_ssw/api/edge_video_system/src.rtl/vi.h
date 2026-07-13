#ifndef _H_VI_H_
#define _H_VI_H_

#include "platform_info.h"

#define VIM_EN          (VIM_BASE + 0x0000)
#define VIM_ADDR0       (VIM_BASE + 0x0004)
#define VIM_ADDR1       (VIM_BASE + 0x0008)
#define VIM_OFFSET0     (VIM_BASE + 0x000C)
#define VIM_OFFSET1     (VIM_BASE + 0x0010)
#define VIM_CLKPOL      (VIM_BASE + 0x0014)
#define VIM_SIZE        (VIM_BASE + 0x0018)
#define VIM_STRIDE      (VIM_BASE + 0x001C)
#define VIM_TYPE        (VIM_BASE + 0x0020)
#define VIM_CIS_PIN     (VIM_BASE + 0x0024)
#define VIM_HDMI_RESET  (VIM_BASE + 0x0028)
#define VIM_ENDIAN      (VIM_BASE + 0x002C)
#define VIM_IDX         (VIM_BASE + 0x0030)
#define VIM_STATUS      (VIM_BASE + 0x0040)

typedef enum {
        VIM_EN_CIS  = 1,
        VIM_EN_HDMI = 2
} eVIM_EN;

typedef struct {
	unsigned int hsize;
	unsigned int vsize;	
	unsigned int stride;
	
	unsigned int addr0;
	unsigned int addr1;
	unsigned int offset0;
	unsigned int offset1;

	int type;
	
	int en;	
	int endian;

	int idx;
} sVIM;

void vim_init_param(sVIM *vim, int width, int height, int type);
void vim_set_param(sVIM *vim);
void vim_enable_cis(void);
void vim_enable_hdmi(void);
void vim_disable(void);
void vim_set_base0(unsigned int addr);
void vim_set_base1(unsigned int addr);
void vim_set_offset(unsigned int offset0, unsigned int offset1);
void vim_set_clkpol(int pol);
void vim_set_size(unsigned int width, unsigned int height);
void vim_set_stride(unsigned int stride);
void vim_set_type(int type);
void vim_set_cispin(int reset, int pwdn);
void vim_set_hdmi_reset(int reset);
void vim_set_endian(int endian);
int vim_get_idx(void);
int vim_get_status(void);
void vim_clear_status(void);

#endif
