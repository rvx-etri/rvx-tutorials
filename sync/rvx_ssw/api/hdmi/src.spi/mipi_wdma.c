#include "mipi_wdma.h"



void mipiwdma_init_param(sMWDMA *mw, int width, int height, int type)
{
	mw->hsize = width;
	mw->vsize = height;

	mw->type = type;

	if(type == HR_COLOR_ARGB) {
		mw->stride = ALIGN1K(width * 4);
	}
	else {
		mw->stride = ALIGN1K(width * 2);
	}

	mw->endian = 2; // for risc-v
}

void mipiwdma_set_param(sMWDMA *mw)
{
	mipiwdma_set_base(mw->addr);
	mipiwdma_set_size(mw->hsize, mw->vsize);
	mipiwdma_set_stride(mw->stride);
	mipiwdma_set_type(mw->type);
	mipiwdma_set_endian(mw->endian);
}

void mipiwdma_set_base(uint32_t addr)
{
	REG32(MIPIWDMA_ADDR) = addr;
}

void mipiwdma_set_size(int width, int height)
{
	REG32(MIPIWDMA_SIZE) = (height<<16) | (width);
}

void mipiwdma_set_stride(int stride)
{
	REG32(MIPIWDMA_STRIDE) = stride;
}

void mipiwdma_set_type(int type)
{
	REG32(MIPIWDMA_TYPE) = type;
}

void mipiwdma_set_endian(int endian)
{
	REG32(MIPIWDMA_ENDIAN) = endian;
}

void mipiwdma_set_sensor_pwdn(int on)
{
	REG32(MIPIWDMA_SENSORPWND) = on;
}