#include "ervp_mmio_util.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_printf.h"

#include "platform_info.h"
#include "vio_util.h"
#include "vi.h"

void vim_init_param(sVIM *vim, int width, int height, int type)
{
	vim->hsize = width;
	vim->vsize = height;
	
	if     (width <= 1024)	vim->stride = 1024;
	else if(width <= 2048)  vim->stride = 2048;
	else	  	            vim->stride = 4096;
		
	vim->type    = type;

	vim->addr0   = VIM_FRAME0;
	vim->addr1   = VIM_FRAME1;


	if(type == VIO_FMT_YUVPLANE) { // YUV420 Plane
		vim->offset0 = vim->stride * vim->vsize; 
		vim->offset1 = vim->offset0 + (vim->stride * vim->vsize) / 2; 
                //vim->offset0 = 0x1000000;
                //vim->offset1 = 0x1000000 * 2;

	}
	else if(type == VIO_FMT_YUVINTRV) { // YUV420 UV interleave
		vim->offset0 = vim->stride * vim->vsize; 
		vim->offset1 = 0; 
	}
	else if(type == VIO_FMT_RGBPLANE) {
		vim->offset0 = vim->stride * vim->vsize; 
		vim->offset1 = 2 * vim->stride * vim->vsize; 
	}
	else if(type == VIO_FMT_RGBINTRV) {
		vim->offset0 = 0;
		vim->offset1 = 0;

		vim->stride *= 4;
	}

	vim->en      = 0;

	vim->endian  = 0; // bit[1]:4byte, bit[0]:4word
}

void vim_set_param(sVIM *vim)
{
	vim_set_base0(vim->addr0);
	vim_set_base1(vim->addr1);
	vim_set_offset(vim->offset0, vim->offset1);
	vim_set_size(vim->hsize, vim->vsize);
	vim_set_stride(vim->stride);
	vim_set_type(vim->type);
	//vim_set_clkpol(vim->clk_pol);
	//vim_set_cispin(vim->reset, vim->pwdn);
	//vim_set_hdmi_reset(vim->reset);
	vim_set_endian(vim->endian);
}

void vim_enable_cis(void)
{
	mmio_write_data(VIM_EN, VIM_EN_CIS);
}

void vim_enable_hdmi(void)
{
	mmio_write_data(VIM_EN, VIM_EN_HDMI);
}

void vim_disable(void)
{
	mmio_write_data(VIM_EN, 0);
}

void vim_set_base0(unsigned int addr)
{
	mmio_write_data(VIM_ADDR0, addr);
}

void vim_set_base1(unsigned int addr)
{
	mmio_write_data(VIM_ADDR1, addr);
}

void vim_set_offset(unsigned int offset0, unsigned int offset1)
{
	mmio_write_data(VIM_OFFSET0, offset0);
	mmio_write_data(VIM_OFFSET1, offset1);
}

void vim_set_clkpol(int pol)
{
	mmio_write_data(VIM_CLKPOL, pol);
}

void vim_set_size(unsigned int width, unsigned int height)
{
	mmio_write_data(VIM_SIZE, (height<<16) | width);
}

void vim_set_stride(unsigned int stride)
{
	mmio_write_data(VIM_STRIDE, stride);
}

void vim_set_type(int type)
{
	mmio_write_data(VIM_TYPE, type);
}

void vim_set_cispin(int reset, int pwdn)
{
	mmio_write_data(VIM_CIS_PIN, (pwdn<<1) | reset);
}

void vim_set_hdmi_reset(int reset)
{
	mmio_write_data(VIM_HDMI_RESET, reset);
}

void vim_set_endian(int endian)
{
	mmio_write_data(VIM_ENDIAN, endian);
}

int vim_get_idx(void)
{
	return mmio_read_data(VIM_IDX);
}

int vim_get_status(void)
{
	return mmio_read_data(VIM_STATUS);
}

void vim_clear_status(void)
{
	mmio_write_data(VIM_STATUS, 1);
}
