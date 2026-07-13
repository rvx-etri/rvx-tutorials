#include "ervp_mmio_util.h"
#include "ervp_external_peri_group_api.h"

#include "platform_info.h"
#include "vio_util.h"
#include "vo.h"

void vom_init_param(sVOM *vom, int width, int height, int type)
{
	vom->hsize = width;
	vom->vsize = height;

	if     (width <= 1024)	vom->stride = 1024;
	else if(width <= 2048)	vom->stride = 2048;
	else		            vom->stride = 4096;

	vom->type    = type;


	vom->addr   = VOM_FRAME0;

	if(type == VIO_FMT_YUVPLANE) { // YUV420 Plane
		vom->offset0 = vom->stride * vom->vsize; 
		vom->offset1 = vom->offset0 + (vom->stride * vom->vsize) / 2; 
                //vom->offset0 = 0x1000000;
                //vom->offset1 = 0x1000000 * 2;

	}
	else if(type == VIO_FMT_YUVINTRV) { // YUV420 UV interleave
		vom->offset0 = vom->stride * vom->vsize; 
		vom->offset1 = 0; 
	}
	else if(type == VIO_FMT_RGBPLANE) {
		vom->offset0 = vom->stride * vom->vsize; 
		vom->offset1 = 2 * vom->stride * vom->vsize; 
	}
	else if(type == VIO_FMT_RGBINTRV) {
		vom->offset0 = 0;
		vom->offset1 = 0;

		vom->stride *= 4;
	}


	vom->en     = 0;

	vom->endian = 0; 

	/*
	vom->d_pol = 1;
	vom->h_pol = 1;
	vom->v_pol = 1;

	// sync
	vom->hss = 320;
	vom->hfp = 0;
	vom->hbp = 0;

	vom->vss = 35;
	vom->vfp = 0;
	vom->vbp = 0;
	*/

}

void vom_set_param(sVOM *vom)
{
	vom_set_base(vom->addr);
	vom_set_offset(vom->offset0, vom->offset1);
	vom_set_size(vom->hsize, vom->vsize);
	vom_set_stride(vom->stride);
	vom_set_type(vom->type);
	vom_set_syncsize(vom->hss, vom->vss);
	vom_set_hporch(vom->hfp, vom->hbp);
	vom_set_vporch(vom->vfp, vom->vbp);
	vom_set_sync_pol(vom->d_pol, vom->h_pol, vom->v_pol, 0);
	vom_set_endian(vom->endian);

	mmio_write_data(VOM_LCD_PATTERN, (0<<24) | (255<<16) | (0<<8) | (0)); // R G B

}

void vom_enable_lcd(void)
{
	mmio_write_data(VOM_EN, VOM_EN_LCD);
}

void vom_enable_hdmi(void)
{
	mmio_write_data(VOM_EN, VOM_EN_HDMI);
}

void vom_disable(void)
{
	mmio_write_data(VOM_EN, 0);
}

void vom_set_base(unsigned int addr)
{
	mmio_write_data(VOM_ADDR, addr);
}

//void vom_set_base1(unsigned int addr)
//{
//	mmio_write_data(VOM_ADDR1, addr);
//}

void vom_set_offset(unsigned int offset0, unsigned int offset1)
{
	mmio_write_data(VOM_OFFSET0, offset0);
	mmio_write_data(VOM_OFFSET1, offset1);
}

void vom_set_clkpol(int pol)
{
	mmio_write_data(VOM_CLKPOL, pol);
}

void vom_set_size(unsigned int width, unsigned int height)
{
	mmio_write_data(VOM_SIZE, (height<<16) | width);
}

void vom_set_stride(unsigned int stride)
{
	mmio_write_data(VOM_STRIDE, stride);
}

void vom_set_type(int type)
{
	mmio_write_data(VOM_TYPE, type);
}

void vom_set_syncsize(unsigned int hs, unsigned int vs)
{
	mmio_write_data(VOM_SYNCSIZE, (vs<<16) | hs);
}

void vom_set_hporch(unsigned int hfp, unsigned int hbp)
{
	mmio_write_data(VOM_HPORCH, (hfp<<16) | hbp);
}

void vom_set_vporch(unsigned int vfp, unsigned int vbp)
{
	mmio_write_data(VOM_VPORCH, (vfp<<16) | vbp);
}

void vom_set_sync_pol(unsigned int dpol, unsigned int hpol, unsigned int vpol, unsigned int hsync_mask)
{
	mmio_write_data(VOM_SYNCPOL, (hsync_mask<<3) | (dpol<<2) | (hpol<<1) | (vpol));
}

void vom_set_lcd_reset(int reset)
{
	mmio_write_data(VOM_LCD_RESET , reset);
}

void vom_set_hdmi_reset(int reset)
{
	mmio_write_data(VOM_HDMI_RESET , reset);
}

void vom_set_endian(int endian)
{
	mmio_write_data(VOM_ENDIAN, endian);
}

void vom_set_4kmode(int mode)
{
	mmio_write_data(VOM_4KMODE, mode);
}

void vom_power_down(int mode)
{
        mmio_write_data(VOM_LCD_PDN, mode);
}

