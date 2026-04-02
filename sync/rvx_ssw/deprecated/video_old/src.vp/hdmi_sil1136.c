#include "ervp_uart.h"
#include "ervp_printf.h"
#include "orvp_i2c.h"
#include "ervp_external_peri_group_api.h"

#include "hdmi_sil1136.h"
#include "vio_util.h"
#include "vio_i2c.h"
#include "video_memory_map.h"

#define I2C_INDEX_FOR_HDMI_CTRL		0
#define I2C_FREQ_OF_SIL1136             10000

#if 1 // hujang
void configure_i2c_for_sil1136()
{
	configure_i2c(I2C_INDEX_FOR_HDMI_CTRL, I2C_FREQ_OF_SIL1136, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void sil1136_i2c_write(unsigned int id, unsigned int addr, unsigned int data)
{
	id = id >> 1;

	// Addr : 1byte, Data : 1byte
	write_i2c_r1d1(I2C_INDEX_FOR_HDMI_CTRL, id, addr, data);
	i2c_delay(5);
}
#else
void sil1136_i2c_write(unsigned int id, unsigned int addr, unsigned int data)
{
	int i;

	id = id / 2;

	for(i=0; i<1; i++) {
		// Addr : 1byte, Data : 1byte
		write_reg(I2C_HDMI_BASE + I2C_ID, id); 
		write_reg(I2C_HDMI_BASE + I2C_CNTR, 0);
		write_reg(I2C_HDMI_BASE + I2C_RWCNTR, 1);
		write_reg(I2C_HDMI_BASE + I2C_ADDR, addr);
		write_reg(I2C_HDMI_BASE + I2C_DATA, data);
		i2c_delay(5);
	}

	//putc_02x(id);
	//putc_02x(addr);
	//putc_02x(data);
	//putc_02x('\n');
	//putc_02x('\r');
	//printf("\n");
}
#endif

#if 1 // hujang
unsigned int sil1136_i2c_read(unsigned int id, unsigned int addr)
{
	unsigned int data;

	id = id >> 1;

	// Addr : 1byte, Data : 1byte
	data = read_i2c_r1d1(I2C_INDEX_FOR_HDMI_CTRL, id, addr);

	return data;
}
#else
unsigned int sil1136_i2c_read(unsigned int id, unsigned int addr)
{
	unsigned int data;

	id = id / 2;

	// Addr : 1byte, Data : 1byte
	write_reg(I2C_HDMI_BASE + I2C_ID, id); 
	write_reg(I2C_HDMI_BASE + I2C_CNTR, 0); 
	write_reg(I2C_HDMI_BASE + I2C_RWCNTR, 5); // 1byte read
	write_reg(I2C_HDMI_BASE + I2C_ADDR, addr);
	i2c_delay(10);
	data = read_reg(I2C_HDMI_BASE + I2C_DATA);

	return data;
}
#endif

void siI1136_tpi_enable(void)
{
	//i2c_hw_write(0xC7, 0x00);
	sil1136_i2c_write(SIL1136_ID, 0xC7, 0x00);
}

void siI1136_set_page0(void)
{
	//i2c_hw_write(0xBC, 0x01);
	sil1136_i2c_write(SIL1136_ID, 0xBC, 0x01);
}

void siI1136_set_vendor_spec_infoframe_4k(void)
{
	unsigned char buf[16];
	unsigned char crc;
	int i;

	buf[0] = 0x81;
	buf[1] = 1;
	buf[2] = 7;    // length
	buf[3] = 0;    // check sum
	buf[4] = 0x03; // 24bit IEEE Registration Identifier (lsb first)
	buf[5] = 0x0C; // 
	buf[6] = 0x00; //
	buf[7] = (1<<5); // extended resolution format
	//buf[8] = 1;    // 4kx2k 29.96,30Hz, 297Mhz, 3840/560/176/88/296  2160/90/8/10/72
	buf[8] = 3;      // 4kx2k 24Hz, 297Mhz, 
	buf[9] = 0;

	crc = 0x81 + 1 + 7;
	for(i=4; i<=9; i++)
		crc += buf[i];
	crc = 0x100 - crc;
	buf[3] = crc;    


	sil1136_i2c_write(SIL1136_ID, 0xC0, 0x81);
	sil1136_i2c_write(SIL1136_ID, 0xC1, 1);
	sil1136_i2c_write(SIL1136_ID, 0xC2, 7);
	sil1136_i2c_write(SIL1136_ID, 0xC3, crc);
	sil1136_i2c_write(SIL1136_ID, 0xC4, buf[4]);
	sil1136_i2c_write(SIL1136_ID, 0xC5, buf[5]);
	sil1136_i2c_write(SIL1136_ID, 0xC6, buf[6]);
	sil1136_i2c_write(SIL1136_ID, 0xC7, buf[7]);
	sil1136_i2c_write(SIL1136_ID, 0xC8, buf[8]);
	sil1136_i2c_write(SIL1136_ID, 0xC9, buf[9]);
	sil1136_i2c_write(SIL1136_ID, 0xBF, 0xC0 | 7); // enable
}

void siI1136_set_avi_infoframe_4k(void)
{
	unsigned char buf[16+1];
	unsigned char crc;

	unsigned char y2, a1, b2, s2;
	unsigned char c2, m2, r4;
	unsigned char ec3, q2, sc2;
	unsigned char vic7;
	unsigned char yq2, cn2, pr4;
	int i;

	y2   = 1;  // rgb or ycbcr : 0-rgb, 1-ycbcr422, 2-ycbcr444, 3-future
	a1   = 0;  // active format information present : 0-no data, 1-active format(r0...r3) information valid
	b2   = 0;  // bar info : 0-bar data not valid 1-vert bar valid, 2-hor bar valid, 3-vert,hor bar valid
	s2   = 0;  // scan infomation : 0-no data
	c2   = 1;  // colorimetry : 0-no data, 1-itu601, 2-709
	m2   = 2;  // picture aspect ratio : 0-no data, 1-4:3, 2-16:9
	r4   = 0;  // active format aspect ratio : 0- same as picture aspect ratio 1-4:3, 2-16:9 ...
	ec3  = 0;  // extended colorimetry : 0-xvYCC601, 1-xvYCC709, 2-sYCC601, 3-AdobeYCC601, 4-Adobergb
	q2   = 2;  // RGB Quantization Ragne : 0-default, 1-limited range 2-full range 3-reserved
	sc2  = 0;  // non-uniform picture scanling : 0-no known
	vic7 = 0;  // 0-extended, 16-1920x1080p@60Hz(no repetition)
	yq2  = 1;  // ycc quantization range 0-limited range 1-full range
	cn2  = 0;  // content type 0-no data
	pr4  = 0;  // pixel repition factor : 0-no repetition


	buf[ 0] = 0x82; // packet type  (HDMI_INFOFRAME_TYPE_AVI = 0x82)
	buf[ 1] = 0x02; // vendor
	buf[ 2] = 13;   // length
	buf[ 3] = 0;    // check sum

	buf[ 4] = (y2<<5) | (a1<<4) | (b2<<2) | (s2<<0);
	buf[ 5] = (c2<<6) | (m2<<4) | (r4<<0);
	buf[ 6] = (0<<7) | (ec3<<4) | (q2<<2) | (sc2<<0);
	buf[ 7] = vic7;
	buf[ 8] = (yq2<<6) | (cn2<<4) | (pr4<<0);
	buf[ 9] = (0); // EndTopBar lsb
	buf[10] = (0); // EndTopBar msb
	buf[11] = (0); // StartBottomBar lsb
	buf[12] = (0); // StartBottomBar msb
	buf[13] = (0); // EndLeftBar lsb
	buf[14] = (0); // EndLeftBar msb
	buf[15] = (0); // EndRightBar lsb
	buf[16] = (0); // EndRightBar msb


	crc = 0x82 + 2 + 13;
	for(i=4; i<=16; i++)
		crc += buf[i];
	crc = 0x100 - crc;
	buf[3] = crc;    

	sil1136_i2c_write(SIL1136_ID, 0x0C, buf[ 3]);
	sil1136_i2c_write(SIL1136_ID, 0x0D, buf[ 4]);
	sil1136_i2c_write(SIL1136_ID, 0x0E, buf[ 5]);
	sil1136_i2c_write(SIL1136_ID, 0x0F, buf[ 6]);
	sil1136_i2c_write(SIL1136_ID, 0x10, buf[ 7]);
	sil1136_i2c_write(SIL1136_ID, 0x11, buf[ 8]);
	sil1136_i2c_write(SIL1136_ID, 0x12, buf[ 9]);
	sil1136_i2c_write(SIL1136_ID, 0x13, buf[10]);
	sil1136_i2c_write(SIL1136_ID, 0x14, buf[11]);
	sil1136_i2c_write(SIL1136_ID, 0x15, buf[12]);
	sil1136_i2c_write(SIL1136_ID, 0x16, buf[13]);
	sil1136_i2c_write(SIL1136_ID, 0x17, buf[14]);
	sil1136_i2c_write(SIL1136_ID, 0x18, buf[15]);
	sil1136_i2c_write(SIL1136_ID, 0x19, buf[16]);
}




void sil1136_init_sg(int w, int h, int vsw, int vfp, int vbp, int hsw, int hfp, int hbp, int freqHz)
{
	printf( "SIL1136 Initialization Done! \n\r");
}



void sii1136_init(void)
{
	unsigned int device_id, device_rev, tpi_rev;

	//unsigned int i;
	printf( "sii1136_init \n");


	sil1136_i2c_write(0x72, 0x1A, 0x11);		// bit[4]:TMDS Active(0) Power down(1), bit[0]:HDMI(1), DVI(0) 
	//delay_ms(100);
	sil1136_i2c_write(0x72, 0xC7, 0x00);		// 

	sil1136_i2c_write(0x72, 0xBC, 0x01);		// Page
	sil1136_i2c_write(0x72, 0xBD, 0x82);		// offset
	sil1136_i2c_write(0x72, 0xBE, 0xA5);		// obtain read/write register access

	sil1136_i2c_write(0x72, 0xBC, 0x01);		// 
	sil1136_i2c_write(0x72, 0xBD, 0x80);		// 
	sil1136_i2c_write(0x72, 0xBE, 0x24);		// 




	device_id  = sil1136_i2c_read(SIL1136_ID, 0x1B);
	device_rev = sil1136_i2c_read(SIL1136_ID, 0x1C);
	tpi_rev    = sil1136_i2c_read(SIL1136_ID, 0x1D);
	printf("Device ID  : %02X\n", device_id);
	printf("Device Rev : %02X\n", device_rev);
	printf("TPI Rev    : %02X\n", tpi_rev);



	sil1136_i2c_write(0x72, 0x62, 0xC0);		// DE_DLY
	sil1136_i2c_write(0x72, 0x64, 0x52);		// DE_TOP
	sil1136_i2c_write(0x72, 0x66, 0x80);		// DE_CNT[7:0] 
	sil1136_i2c_write(0x72, 0x67, 0x07);		// DE_CNT[12:8]
	sil1136_i2c_write(0x72, 0x68, 0x38);		// DE_LIN[7:0
	sil1136_i2c_write(0x72, 0x69, 0x04);		// DE_LIN[11:8]

	sil1136_i2c_write(0x72, 0x00, 0xE9);		// 
	sil1136_i2c_write(0x72, 0x01, 0x39);		// 
	sil1136_i2c_write(0x72, 0x02, 0x3C);		// 
	sil1136_i2c_write(0x72, 0x04, 0x98);		// 
	sil1136_i2c_write(0x72, 0x05, 0x08);		// 
	sil1136_i2c_write(0x72, 0x06, 0x92);		// 
	sil1136_i2c_write(0x72, 0x07, 0x04);		// 

	sil1136_i2c_write(0x72, 0x60, 0x00);		// 

	sil1136_i2c_write(0x72, 0x09, 0x00);		// 
	sil1136_i2c_write(0x72, 0x0A, 0x00);		// 

	sil1136_i2c_write(0x72, 0x1A, 0x01);		// 

	// End
	printf( "SII1136 Initialization Done!\n");

	return 0;
}
