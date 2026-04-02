#include "hdmi_rdma.h"

VIDEO *video_io;

void hdmirdma_init_param(sHRDMA *hr, int width, int height, int type)
{
	hr->hsize = width;
	hr->vsize = height;

	hr->type = type;

	if(type == HR_COLOR_ARGB) {
		hr->stride = ALIGN1K(width * 4);
	}
	else {
		printf("Only support RGBINTRV format on the vitual platform\n");
	}

	//hr->addr0 = VOM_FRAMEBUFFER0;
}

void hdmirdma_set_param(sHRDMA *hr)
{
	video_io = (VIDEO *)HDMIRDMA_BASE;
	video_io->ctrl = 0x0000182B;
	hdmirdma_set_base0(hr->addr0);
	hdmirdma_set_size(hr->hsize, hr->vsize);
	hdmirdma_set_stride(hr->stride);
}

void hdmirdma_start(void)
{
	video_io->ctrl = 0x0000182B | PL110_LCD_PWR_BIT;
}

void hdmirdma_stop(void)
{
	video_io->ctrl = 0x0000182B & ~PL110_LCD_PWR_BIT;
}

void hdmirdma_set_base0(uint32_t addr)
{
	video_io->base = addr;
}

void hdmirdma_set_base1(uint32_t addr)
{
}

void hdmirdma_set_idx(int idx)
{
}

void hdmirdma_set_size(int width, int height)
{
	video_io->ppl = width;
	video_io->lpp = height;
}

void hdmirdma_set_stride(int stride)
{
	video_io->stride = stride;
}

void hdmirdma_set_type(int type)
{
}

void hdmirdma_set_syncsize(int hs, int vs)
{
}

void hdmirdma_set_vporch(int vbp, int vfp)
{
}

void hdmirdma_set_hporch(int hbp, int hfp)
{
}

void hdmirdma_set_syncpol(int hpol, int vpol, int hsmask)
{
}

void hdmirdma_set_endian(int endian)
{
}

void hdmirdma_set_pattern(int en, int r, int g, int b)
{
}
