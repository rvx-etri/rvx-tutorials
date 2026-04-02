#include "ervp_mmio_util.h"
#include "ervp_external_peri_group_api.h"

#include "platform_info.h"
#include "video_memory_map.h"
#include "vio_util.h"
#include "vo.h"

VIDEO *lcd_io;

void vom_init_param(sVOM *vom, int width, int height, int type)
{
	vom->hsize = width;
	vom->vsize = height;
	printf("width [%d] hegiht [%d]\n", vom->hsize, vom->vsize);

        if(type != VIO_FMT_RGBINTRV)
                printf("Only support RGBINTRV format on the vitual platform\n");

	vom->addr   = VOM_FRAMEBUFFER0;

        if     (width <= 1024)  vom->stride = 1024 * 4;
        else if(width <= 2048)  vom->stride = 2048 * 4;
        else                        vom->stride = 4096 * 4;

}

void vom_set_param(sVOM *vom)
{
	lcd_io = (VIDEO *)VOM_BASE;
	lcd_io->ctrl = 0x00001A2B;
	vom_set_base(vom->addr);
	vom_set_size(vom->hsize, vom->vsize);
	vom_set_stride(vom->stride);
}

void vom_enable_lcd(void)
{
	lcd_io->ctrl = 0x00001A2B | PL110_LCD_PWR_BIT;
}

void vom_enable_hdmi(void)
{
	lcd_io->ctrl = 0x00001A2B | PL110_LCD_PWR_BIT;
}

void vom_disable(void)
{
	lcd_io->ctrl = 0x00001A2B & ~PL110_LCD_PWR_BIT;
}

void vom_set_base(unsigned int addr)
{
	lcd_io->base = addr;
}

void vom_set_offset(unsigned int offset0, unsigned int offset1)
{
}

void vom_set_clkpol(int pol)
{
}

void vom_set_size(unsigned int width, unsigned int height)
{
	lcd_io->ppl = width;
	lcd_io->lpp = height;
}

void vom_set_stride(unsigned int stride)
{
	lcd_io->stride = stride;
}

void vom_set_type(int type)
{
}

void vom_set_syncsize(unsigned int hs, unsigned int vs)
{
}

void vom_set_hporch(unsigned int hfp, unsigned int hbp)
{
}

void vom_set_vporch(unsigned int vfp, unsigned int vbp)
{
}

void vom_set_sync_pol(unsigned int dpol, unsigned int hpol, unsigned int vpol, unsigned int hsync_mask)
{
}

void vom_set_lcd_reset(int reset)
{
}

void vom_set_hdmi_reset(int reset)
{
}

void vom_set_endian(int endian)
{
}

void vom_set_4kmode(int mode)
{
}

void vom_power_down(int mode)
{
        write_reg(VOM_LCD_PDN, mode);
}

