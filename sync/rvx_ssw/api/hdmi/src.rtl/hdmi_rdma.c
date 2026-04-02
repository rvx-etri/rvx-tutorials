#include "hdmi_rdma.h"



void hdmirdma_init_param(sHRDMA *hr, int width, int height, int type)
{
	hr->hsize = width;
	hr->vsize = height;

	hr->type = type;

	if(type == HR_COLOR_ARGB) {
		hr->stride = ALIGN1K(width * 4);
	}
	else {
		hr->stride = ALIGN1K(width * 2);
	}

	
	hr->idx  = 0;

	hr->hss = 100; 
	hr->hfp = 100; 
	hr->hbp = 100;
	hr->vss =  17; 
	hr->vfp =  15; 
	hr->vbp =  15;

	hr->h_pol = 0;
	hr->v_pol = 0;
	hr->hsync_mask = 0;

	hr->endian = 2; // for risc-v

	//hr->addr0 = 0;
	//hr->addr1 = 0;
}



void hdmirdma_set_param(sHRDMA *hr)
{
	/*
	printf("hsize      = %d \n", hr->hsize     );
	printf("vsize      = %d \n", hr->vsize     );
	printf("stride     = %d \n", hr->stride    );
	printf("addr0      = %d \n", hr->addr0     );
	printf("addr1      = %d \n", hr->addr1     );
	printf("idx        = %d \n", hr->idx       );
	printf("type       = %d \n", hr->type      );
	printf("hbp        = %d \n", hr->hbp       );
	printf("hfp        = %d \n", hr->hfp       );
	printf("vbp        = %d \n", hr->vbp       );
	printf("vfp        = %d \n", hr->vfp       );
	printf("hss        = %d \n", hr->hss       );
	printf("vss        = %d \n", hr->vss       );
	printf("h_pol      = %d \n", hr->h_pol     );
	printf("v_pol      = %d \n", hr->v_pol     );
	printf("hsync_mask = %d \n", hr->hsync_mask);
	printf("endian     = %d \n", hr->endian    );
	*/
	hdmirdma_set_base0(hr->addr0);
	hdmirdma_set_base1(hr->addr1);
	hdmirdma_set_idx(hr->idx);
	hdmirdma_set_size(hr->hsize, hr->vsize);
	hdmirdma_set_stride(hr->stride);
	hdmirdma_set_type(hr->type);
	hdmirdma_set_syncsize(hr->hss, hr->vss);
	hdmirdma_set_vporch(hr->vfp, hr->vfp);
	hdmirdma_set_hporch(hr->hfp, hr->hfp);
	hdmirdma_set_syncpol(hr->h_pol, hr->v_pol, hr->hsync_mask);
	hdmirdma_set_endian(hr->endian);
	
	hdmirdma_set_pattern(0, 0, 0, 0);

}

void hdmirdma_start(void)
{
	REG32(HDMIRDMA_EN) = 1;
}

void hdmirdma_stop(void)
{
	REG32(HDMIRDMA_EN) = 0;
}

void hdmirdma_set_base0(uint32_t addr)
{
	REG32(HDMIRDMA_ADDR0) = addr;
}

void hdmirdma_set_base1(uint32_t addr)
{
	REG32(HDMIRDMA_ADDR1) = addr;
}

void hdmirdma_set_idx(int idx)
{
	REG32(HDMIRDMA_IDX) = idx;
}

void hdmirdma_set_size(int width, int height)
{
	REG32(HDMIRDMA_SIZE) = (height<<16) | (width);
}

void hdmirdma_set_stride(int stride)
{
	REG32(HDMIRDMA_STRIDE) = stride;
}

void hdmirdma_set_type(int type)
{
	REG32(HDMIRDMA_TYPE) = type;
}

void hdmirdma_set_syncsize(int hs, int vs)
{
	REG32(HDMIRDMA_SYNCSIZE) = (vs<<16) | (hs);
}

void hdmirdma_set_vporch(int vbp, int vfp)
{
	REG32(HDMIRDMA_VPORCH) = (vfp<<16) | (vbp);
}

void hdmirdma_set_hporch(int hbp, int hfp)
{
	REG32(HDMIRDMA_HPORCH) = (hfp<<16) | (hbp);
}

void hdmirdma_set_syncpol(int hpol, int vpol, int hsmask)
{
	REG32(HDMIRDMA_SYNCPOL) = (hsmask<<2) | (hpol<<1) | (vpol);
}

void hdmirdma_set_endian(int endian)
{
	REG32(HDMIRDMA_ENDIAN) = endian;
}

void hdmirdma_set_pattern(int en, int r, int g, int b)
{
	REG32(HDMIRDMA_PATTERN) = (en<<24) | (r<<16) | (g<<8) | (b);
}
