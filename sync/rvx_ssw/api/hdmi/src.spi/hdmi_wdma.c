#include "hdmi_wdma.h"



void hdmiwdma_init_param(sHWDMA *hw, int width, int height, int type)
{
	hw->hsize = width;
	hw->vsize = height;

	hw->type = type;

	if(type == HR_COLOR_ARGB) {
		hw->stride = ALIGN1K(width * 4);
	}
	else {
		hw->stride = ALIGN1K(width * 2);
	}

	hw->endian = 2; // for risc-v
}

void hdmiwdma_set_param(sHWDMA *hw)
{
	hdmiwdma_set_base(hw->addr);
	hdmiwdma_set_size(hw->hsize, hw->vsize);
	hdmiwdma_set_stride(hw->stride);
	hdmiwdma_set_type(hw->type);
	hdmiwdma_set_endian(hw->endian);
}

void hdmiwdma_set_base(uint32_t addr)
{
	REG32(HDMIWDMA_ADDR) = addr;
}

void hdmiwdma_set_size(int width, int height)
{
	REG32(HDMIWDMA_SIZE) = (height<<16) | (width);
}

void hdmiwdma_set_stride(int stride)
{
	REG32(HDMIWDMA_STRIDE) = stride;
}

void hdmiwdma_set_type(int type)
{
	REG32(HDMIWDMA_TYPE) = type;
}

void hdmiwdma_set_endian(int endian)
{
	REG32(HDMIWDMA_ENDIAN) = endian;
}

void hdmiwdma_set_hpd(int on)
{
	REG32(HDMIWDMA_HPD) = on;
}

void hdmiwdma_reset_dvi2rgb(int reset)
{
	REG32(HDMIWDMA_D2R_RESET) = reset;
}

int  hdmiwmda_read_plllock(void)
{
	return REG32(HDMIWDMA_PLLLOCK);
}

void hdmiwmda_start(void)
{
	REG32(HDMIWDMA_EN) = 1;
}

void hdmiwmda_stop(void)
{
	REG32(HDMIWDMA_EN) = 0;
}
