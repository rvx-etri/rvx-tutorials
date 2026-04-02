#include "ervp_uart.h"
#include "ervp_printf.h"
#include "orvp_i2c.h"
#include "ervp_external_peri_group_api.h"

#include "hdmi_adv7619.h"
#include "vio_util.h"
#include "vio_i2c.h"
#include "video_memory_map.h"

#define I2C_INDEX_FOR_HDMI_CTRL		0
#define I2C_FREQ_OF_ADV7619             100000

#if 1 //hujang
void configure_i2c_for_adv7619()
{
        configure_i2c(I2C_INDEX_FOR_HDMI_CTRL, I2C_FREQ_OF_ADV7619, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void adv7619_i2c_write(unsigned int id, unsigned int addr, unsigned int data)
{
	int i;

	id = id >> 1;

	for(i=0; i<1; i++) {
		// Addr : 1byte, Data : 1byte
		write_i2c_r1d1(I2C_INDEX_FOR_HDMI_CTRL, id, addr, data);
		i2c_delay(5);
	}
}
#else
void adv7619_i2c_write(unsigned int id, unsigned int addr, unsigned int data)
{
	int i;

	id = id / 2;

	for(i=0; i<4; i++) {
		// Addr : 1byte, Data : 1byte
		write_reg(I2C_HDMI_BASE + I2C_ID, id); 
		write_reg(I2C_HDMI_BASE + I2C_CNTR, 0);
		write_reg(I2C_HDMI_BASE + I2C_RWCNTR, 1);
		write_reg(I2C_HDMI_BASE + I2C_ADDR, addr);
		write_reg(I2C_HDMI_BASE + I2C_DATA, data);
		i2c_delay(5);
	}


	//printf("adv7619_i2c_write : id(%02X) addr(%02X) data(%02X)\n", id*2, addr, data);
	//uart_getc(0);
}
#endif

#if 1 //hujang
unsigned int adv7619_i2c_read(unsigned int id, unsigned int addr)
{
	unsigned int data;

	id = id >> 1;

	// Addr : 1byte, Data : 1byte
	data = read_i2c_r1d1(I2C_INDEX_FOR_HDMI_CTRL, id, addr);

	return data;
}
#else
unsigned int adv7619_i2c_read(unsigned int id, unsigned int addr)
{
	unsigned int data;

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

void adv7619_init_ycbcr(void)
{
	unsigned int i;

	printf( "I2C Initialization for ADV7619! \n\r");

	configure_i2c_for_adv7619();

	printf("ADV7619 Initialization Start! \n\r");
	//printf("ADV7619 Mode : 720p,1080i,1080p In - 24-bit YCbCr 422 Out: \n\r");

	// :02-07 720p,1080i,1080p In - 24-bit YCbCr 422 Out:
	adv7619_i2c_write(0x98, 0xFF, 0x80);		// 98 FF 80 ; I2C reset

	delay_ms(100);

	adv7619_i2c_write(0x98, 0xF4, 0x80);		// 98 F4 80 ; CEC
	adv7619_i2c_write(0x98, 0xF5, 0x7C);		// 98 F5 7C ; INFOFRAME
	adv7619_i2c_write(0x98, 0xF8, 0x4C);		// 98 F8 4C ; DPLL
	adv7619_i2c_write(0x98, 0xF9, 0x64);		// 98 F9 64 ; KSV
	adv7619_i2c_write(0x98, 0xFA, 0x6C);		// 98 FA 6C ; EDID
	adv7619_i2c_write(0x98, 0xFB, 0x68);		// 98 FB 68 ; HDMI
	adv7619_i2c_write(0x98, 0xFD, 0x44);		// 98 FD 44 ; CP

	delay_ms(100);

	adv7619_i2c_write(0x68, 0xC0, 0x03);		// 68 C0 03 ; ADI Required Write
	adv7619_i2c_write(0x98, 0x01, 0x06);		// 98 01 06 ; Prim_Mode =110b HDMI-GR
	adv7619_i2c_write(0x98, 0x02, 0xF5);		// 98 02 F5 ; Auto CSC, YCrCb out, Set op_656 bit
	//adv7619_i2c_write(0x98, 0x03, 0x82);		// 98 03 82 ; 24 bit SDR 422 Mode 0
	adv7619_i2c_write(0x98, 0x03, 0x80);		// 98 03 82 ; 24 bit SDR 422 Mode 0
	
	//adv7619_i2c_write(0x98, 0x05, 0x28);		// 98 05 28 ; AV Codes Off
	adv7619_i2c_write(0x98, 0x05, 0x28 | 0);		// 98 05 28 ; bit[0]:Cb/Cr order
	
	//adv7619_i2c_write(0x98, 0x06, 0xA6);		// 98 06 A6 ; Invert VS,HS pins
	//adv7619_i2c_write(0x98, 0x06, 0xA0);		// 98 06 A6 ; Invert VS,HS pins
	adv7619_i2c_write(0x98, 0x06, 0xA1);		// 98 06 A6 ; Invert VS,HS pins

	//adv7619_i2c_write(0x98, 0x14, 0x27);		// drive strength max: DR_STR
	//adv7619_i2c_write(0x98, 0x14, 0x2A);		// drive strength max: DR_STR


	adv7619_i2c_write(0x98, 0x0C, 0x42);		// 98 0C 42 ; Power up part
	adv7619_i2c_write(0x98, 0x15, 0x80);		// 98 15 80 ; Disable Tristate of Pins
	adv7619_i2c_write(0x98, 0x19, 0x83);		// 98 19 83 ; LLC DLL phase
	adv7619_i2c_write(0x98, 0x33, 0x40);		// 98 33 40 ; LLC DLL MUX enable
	adv7619_i2c_write(0x44, 0xBA, 0x01);		// 44 BA 01 ; Set HDMI FreeRun
	adv7619_i2c_write(0x44, 0x6C, 0x00);		// 44 6C 00 ; Required ADI write
	adv7619_i2c_write(0x64, 0x40, 0x81);		// 64 40 81 ; Disable HDCP 1.1 features
	adv7619_i2c_write(0x4C, 0xB5, 0x01);		// 4C B5 01 ; Setting MCLK to 256Fs
	adv7619_i2c_write(0x68, 0xC0, 0x03);		// 68 C0 03 ; ADI Required write
	adv7619_i2c_write(0x68, 0x00, 0x08);		// 68 00 08 ; Set HDMI Input Port A (BG_MEAS_PORT_SEL = 001b)
	adv7619_i2c_write(0x68, 0x02, 0x03);		// 68 02 03 ; ALL BG Ports enabled
	adv7619_i2c_write(0x68, 0x03, 0x98);		// 68 03 98 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x10, 0xA5);		// 68 10 A5 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x1B, 0x08);		// 68 1B 08 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x45, 0x04);		// 68 45 04 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x97, 0xC0);		// 68 97 C0 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x3D, 0x10);		// 68 3D 10 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x3E, 0x69);		// 68 3E 69 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x3F, 0x46);		// 68 3F 46 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x4E, 0xFE);		// 68 4E FE ; ADI Required Write 
	adv7619_i2c_write(0x68, 0x4F, 0x08);		// 68 4F 08 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x50, 0x00);		// 68 50 00 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x57, 0xA3);		// 68 57 A3 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x58, 0x07);		// 68 58 07 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x6F, 0x08);		// 68 6F 08 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x83, 0xFC);		// 68 83 FC ; Enable clock terminators for port A & B
	adv7619_i2c_write(0x68, 0x84, 0x03);		// 68 84 03 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x85, 0x10);		// 68 85 10 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x86, 0x9B);		// 68 86 9B ; ADI Required Write 
	adv7619_i2c_write(0x68, 0x89, 0x03);		// 68 89 03 ; ADI Required Write 
	adv7619_i2c_write(0x68, 0x9B, 0x03);		// 68 9B 03 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x93, 0x03);		// 68 93 03 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x5A, 0x80);		// 68 5A 80 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0x80);		// 68 9C 80 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0xC0);		// 68 9C C0 ; ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0x00);		// 68 9C 00 ; ADI Required Write


	// End
	printf( "ADV7619 Initialization Done! \n\r");

}

void adv7619_init_ycbcr_4k(void)
{
	unsigned int i;

	adv7619_i2c_write(0x98, 0xFF, 0x80); //I2C reset
	delay_ms(100);

	adv7619_i2c_write(0x98, 0xF4, 0x80); //CEC
	adv7619_i2c_write(0x98, 0xF5, 0x7C); //INFOFRAME
	adv7619_i2c_write(0x98, 0xF8, 0x4C); //DPLL
	adv7619_i2c_write(0x98, 0xF9, 0x64); //KSV
	adv7619_i2c_write(0x98, 0xFA, 0x6C); //EDID
	adv7619_i2c_write(0x98, 0xFB, 0x68); //HDMI
	adv7619_i2c_write(0x98, 0xFD, 0x44); //CP
	delay_ms(100);

	adv7619_i2c_write(0x68, 0xC0, 0x03); //ADI Required Write
	adv7619_i2c_write(0x98, 0x00, 0x19); //Set VID_STD
	adv7619_i2c_write(0x98, 0x01, 0x05); //Prim_Mode =101b HDMI-Comp
	adv7619_i2c_write(0x98, 0x02, 0xF0); //Auto CSC, YCbCr out, Set op_656 bit
	
	//adv7619_i2c_write(0x98, 0x03, 0x96); //2x24 bit SDR 422 interleaved mode 0
	adv7619_i2c_write(0x98, 0x03, 0x82); //2x24 bit SDR 422 interleaved mode 0

	adv7619_i2c_write(0x98, 0x05, 0x28); //AV Codes Off
	adv7619_i2c_write(0x98, 0x06, 0xA0); //No inversion on VS,HS pins
	adv7619_i2c_write(0x98, 0x0C, 0x42); //Power up part
	adv7619_i2c_write(0x98, 0x15, 0x80); //Disable Tristate of Pins
	adv7619_i2c_write(0x98, 0x19, 0x83); //LLC DLL phase
	adv7619_i2c_write(0x98, 0x33, 0x40); //LLC DLL MUX enable
	adv7619_i2c_write(0x98, 0xDD, 0x00); //ADI Required Write
	adv7619_i2c_write(0x98, 0xE7, 0x04); //ADI Required Write 
	adv7619_i2c_write(0x4C, 0xB5, 0x01); //Setting MCLK to 256Fs
	adv7619_i2c_write(0x4C, 0xC3, 0x80); //ADI Required Write
	adv7619_i2c_write(0x4C, 0xCF, 0x03); //ADI Required Write
	adv7619_i2c_write(0x4C, 0xDB, 0x80); //ADI Required Write
	adv7619_i2c_write(0x68, 0xC0, 0x03); //ADI Required write
	adv7619_i2c_write(0x68, 0x00, 0x08); //Set HDMI Input Port A (BG_MEAS_PORT_SEL = 001b)
	adv7619_i2c_write(0x68, 0x02, 0x03); //ALL BG Ports enabled
	adv7619_i2c_write(0x68, 0x03, 0x98); //ADI Required Write
	adv7619_i2c_write(0x68, 0x10, 0xA5); //ADI Required Write
	adv7619_i2c_write(0x68, 0x1B, 0x00); //ADI Required Write
	adv7619_i2c_write(0x68, 0x45, 0x04); //ADI Required Write
	adv7619_i2c_write(0x68, 0x97, 0xC0); //ADI Required Write
	adv7619_i2c_write(0x68, 0x3E, 0x69); //ADI Required Write
	adv7619_i2c_write(0x68, 0x3F, 0x46); //ADI Required Write
	adv7619_i2c_write(0x68, 0x4E, 0xFE); //ADI Required Write 
	adv7619_i2c_write(0x68, 0x4F, 0x08); //ADI Required Write
	adv7619_i2c_write(0x68, 0x50, 0x00); //ADI Required Write
	adv7619_i2c_write(0x68, 0x57, 0xA3); //ADI Required Write
	adv7619_i2c_write(0x68, 0x58, 0x07); //ADI Required Write
	adv7619_i2c_write(0x68, 0x6F, 0x08); //ADI Required Write
	adv7619_i2c_write(0x68, 0x83, 0xFC); //Enable clock terminators for port A & B 
	adv7619_i2c_write(0x68, 0x84, 0x03); //FP MODE
	adv7619_i2c_write(0x68, 0x85, 0x10); //ADI Required Write 
	adv7619_i2c_write(0x68, 0x86, 0x9B); //ADI Required Write 
	adv7619_i2c_write(0x68, 0x89, 0x03); //HF Gain
	adv7619_i2c_write(0x68, 0x9B, 0x03); //ADI Required Write
	adv7619_i2c_write(0x68, 0x93, 0x03); //ADI Required Write
	adv7619_i2c_write(0x68, 0x5A, 0x80); //ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0x80); //ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0xC0); //ADI Required Write
	adv7619_i2c_write(0x68, 0x9C, 0x00); //ADI Required Write

	// End
	printf( "ADV7619 Initialization Done!\n\r");
	printf( "ADV7619 Mode : 4K \n\r");

}
