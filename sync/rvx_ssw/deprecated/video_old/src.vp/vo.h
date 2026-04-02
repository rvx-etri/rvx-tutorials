#ifndef _H_VO_H_
#define _H_VO_H_

#define VOM_FRAMEBUFFER0    (DRAM_ADDR+ 0x8000000)
#define VOM_FRAMEBUFFER1    (DRAM_ADDR+ 0xC000000)

#define VOM_EN          (VOM_BASE + 0x0000)
#define VOM_ADDR        (VOM_BASE + 0x0004)
#define VOM_OFFSET0     (VOM_BASE + 0x000C)
#define VOM_OFFSET1     (VOM_BASE + 0x0010)
#define VOM_CLKPOL      (VOM_BASE + 0x0014)
#define VOM_SIZE        (VOM_BASE + 0x0018)
#define VOM_STRIDE      (VOM_BASE + 0x001C)
#define VOM_TYPE        (VOM_BASE + 0x0020)
#define VOM_SYNCSIZE    (VOM_BASE + 0x0024)
#define VOM_VPORCH      (VOM_BASE + 0x0028)
#define VOM_HPORCH      (VOM_BASE + 0x002C)
#define VOM_4KMODE      (VOM_BASE + 0x0030)
#define VOM_SYNCPOL     (VOM_BASE + 0x003C)
#define VOM_LCD_RESET   (VOM_BASE + 0x0040)
#define VOM_HDMI_RESET  (VOM_BASE + 0x0044)
#define VOM_ENDIAN      (VOM_BASE + 0x0048)
#define VOM_LCD_PATTERN (VOM_BASE + 0x004C)
#define VOM_LCD_PDN     (VOM_BASE + 0x0050)

#define PL110_LCD_PWR_BIT       (1<<11)

typedef enum {
        VOM_EN_LCD  = 1,
        VOM_EN_HDMI = 2
} eVOM_EN;

typedef struct {
	int hsize;
	int vsize;
	unsigned int stride;

	//unsigned int addr0;
	//unsigned int addr1;
	unsigned int addr;
	unsigned int offset0;
	unsigned int offset1;

	int type;

	int en;
	int endian;

	// sync 
	// H
	unsigned int hss;
	unsigned int hfp;
	unsigned int hbp;
	// V
	unsigned int vss;
	unsigned int vfp;
	unsigned int vbp;

	unsigned int d_pol;
	unsigned int h_pol;
	unsigned int v_pol;
} sVOM;

typedef struct _VIDEO
{
        volatile unsigned int ppl;   /* pixels per line : 0x0 */
        volatile unsigned int lpp;   /* lines per panel : 0x4 */ 
        volatile unsigned int base;  /* BASE :		0x8 */ 
        volatile unsigned int imask;  /* Interrupt Mask Set/Clear Register : 	0xC*/ 
        volatile unsigned int ctrl;   /* Control : 0x10 */
        volatile unsigned int stride; /* Stride : 0x14 */
} VIDEO;

void vom_init_param(sVOM *vom, int width, int height, int type);
void vom_set_param(sVOM *vom);
void vom_enable_lcd(void);
void vom_enable_hdmi(void);
void vom_disable(void);
void vom_set_base(unsigned int addr);
void vom_set_offset(unsigned int offset0, unsigned int offset1);
void vom_set_clkpol(int pol);
void vom_set_size(unsigned int width, unsigned int height);
void vom_set_stride(unsigned int stride);
void vom_set_type(int type);
void vom_set_syncsize(unsigned int hs, unsigned int vs);
void vom_set_hporch(unsigned int hfp, unsigned int hbp);
void vom_set_vporch(unsigned int vfp, unsigned int vbp);
void vom_set_sync_pol(unsigned int dpol, unsigned int hpol, unsigned int vpol, unsigned int hsync_mask);
void vom_set_lcd_reset(int reset);
void vom_set_hdmi_reset(int reset);
void vom_set_endian(int endian);
void vom_set_4kmode(int mode);

#endif
