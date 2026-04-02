#include "ervp_uart.h"
#include "ervp_printf.h"
#include "orvp_i2c.h"
#include "ervp_external_peri_group_api.h"
#include "platform_info.h"

#include "cis_mt9p111.h"
#include "video_memory_map.h"
#include "vio_util.h"
//#include "ov2640_regs.h"

#define MT9P_IMG_WIDTH 	 1024
#define MT9P_IMG_HEIGHT   600

#define I2C_FREQ_OF_CIS_MT9P111		100000

void configure_i2c_for_cis_mt9p111()
{
        configure_i2c(I2C_INDEX_FOR_CIS, I2C_FREQ_OF_CIS_MT9P111, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void mt9p111_i2c_write(unsigned int id, unsigned int addr, unsigned int data, int len)
{
	if(len == WORD_LEN)
		write_i2c_r2d2(I2C_INDEX_FOR_CIS, id, addr, data);
	else
		write_i2c_r2d1(I2C_INDEX_FOR_CIS, id, addr, data);
	delay_ms(30);
}

/*
uint8_t cam_read_i2c(uint16_t addr)
{
        uint8_t data;

        start_i2c_transmisstion(I2C_INDEX_FOR_CIS, OV2640_SENSOR_ADDR);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr>>8);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr&0xff);

        start_i2c_reading(I2C_INDEX_FOR_CIS, OV2640_SENSOR_ADDR);

        // writing
        data = finish_reading_a_byte_on_i2c(I2C_INDEX_FOR_CIS);

        return data;
}

int cam_setup_check(void)
{
        uint8_t  vid, pid;

        // Check if the camera module is ov5642
        vid = cam_read_i2c(OV2640_CHIPID_HIGH);
        pid = cam_read_i2c(OV2640_CHIPID_LOW);

        if ((vid != 0x56) || (pid != 0x42)) { printf("Can't find OV2640 module!\n"); return 0; }
        else                                    { printf("Find OV2640 module!\n"); }

        return 1;
}
*/


void mt9p111_init_sg(void)
{
	printf( "I2C Initialization for MT9P111 Start! \n\r");
	configure_i2c_for_cis_mt9p111();

	printf( "MT9P111 Initialization Start! \n\r");


	mt9p111_i2c_write(MT9P111_ID, SENSOR_RESET_REG, SENSOR_RESET_VAL, SENSOR_RESET_REG_LEN); delay_ms(50);

	mt9p111_i2c_write(MT9P111_ID, 0x0010, 0x0340, WORD_LEN);
	mt9p111_i2c_write(MT9P111_ID, 0x0012, 0x00f0, WORD_LEN);     // 0xF0=51MHz
	mt9p111_i2c_write(MT9P111_ID, 0x0014, 0x2025, WORD_LEN);
	mt9p111_i2c_write(MT9P111_ID, 0x001E, 0x0565, WORD_LEN);    // PAD_SLEW_PAD_CONFIG
	mt9p111_i2c_write(MT9P111_ID, 0x0022, 0x0030, WORD_LEN);   //0x01E0// VDD_DIS_COUNTER
	mt9p111_i2c_write(MT9P111_ID, 0x002A, 0x7FFF, WORD_LEN);  
	mt9p111_i2c_write(MT9P111_ID, 0x002C, 0x0000, WORD_LEN); 	// PLL_P7_DIVIDER
	mt9p111_i2c_write(MT9P111_ID, 0x002E, 0x0000, WORD_LEN); 	// SENSOR_CLOCK_DIVIDER
	mt9p111_i2c_write(MT9P111_ID, 0x0018, 0x4008, WORD_LEN); 	// STANDBY_CONTROL_AND_STATUS
	delay_ms(20);
	mt9p111_i2c_write(MT9P111_ID, 0x0010, 0x0340, WORD_LEN); 	// PLL_DIVIDERS
	mt9p111_i2c_write(MT9P111_ID, 0x098E, 0x483A, WORD_LEN); 	// LOGICAL_ADDRESS_ACCESS [CAM_CORE_A_Y_ADDR_START]
	mt9p111_i2c_write(MT9P111_ID, 0x098E, 0x1000, WORD_LEN);
	mt9p111_i2c_write(MT9P111_ID, 0xC86C, 0x0518, WORD_LEN);	//* Output Width (A) = 1304 
	mt9p111_i2c_write(MT9P111_ID, 0xC86E, 0x03D4, WORD_LEN);	//* Output Height (A) = 980 
	mt9p111_i2c_write(MT9P111_ID, 0xC83A, 0x000C, WORD_LEN);	//* Row Start (A) = 12 
	mt9p111_i2c_write(MT9P111_ID, 0xC83C, 0x0018, WORD_LEN);	//* Column Start (A) = 24 
	mt9p111_i2c_write(MT9P111_ID, 0xC83E, 0x07B1, WORD_LEN);	//* Row End (A) = 1969 
	mt9p111_i2c_write(MT9P111_ID, 0xC840, 0x0A45, WORD_LEN);	//* Column End (A) = 2629 
	mt9p111_i2c_write(MT9P111_ID, 0xC842, 0x0001, WORD_LEN);	//* Row Speed (A) = 1 
	mt9p111_i2c_write(MT9P111_ID, 0xC844, 0x0103, WORD_LEN);	//* Core Skip X (A) = 259 
	mt9p111_i2c_write(MT9P111_ID, 0xC846, 0x0103, WORD_LEN);	//* Core Skip Y (A) = 259 
	mt9p111_i2c_write(MT9P111_ID, 0xC848, 0x0103, WORD_LEN);	//* Pipe Skip X (A) = 259 
	mt9p111_i2c_write(MT9P111_ID, 0xC84A, 0x0103, WORD_LEN);	//* Pipe Skip Y (A) = 259 
	mt9p111_i2c_write(MT9P111_ID, 0xC84C, 0x00F6, WORD_LEN);	//* Power Mode (A) = 246 
	mt9p111_i2c_write(MT9P111_ID, 0xC84E, 0x0001, WORD_LEN);	//* Bin Mode (A) = 1 
	mt9p111_i2c_write(MT9P111_ID, 0xC850, 0x00  , BYTE_LEN);	//* Orientation (A) = 0 
	mt9p111_i2c_write(MT9P111_ID, 0xC851, 0x00  , BYTE_LEN);	//* Pixel Order (A) = 0 
	mt9p111_i2c_write(MT9P111_ID, 0xC852, 0x019C, WORD_LEN);	//* Fine Correction (A) = 412 
	mt9p111_i2c_write(MT9P111_ID, 0xC854, 0x0732, WORD_LEN);	//* Fine IT Min (A) = 1842 
	mt9p111_i2c_write(MT9P111_ID, 0xC856, 0x048E, WORD_LEN);	//* Fine IT Max Margin (A) = 1166 
	mt9p111_i2c_write(MT9P111_ID, 0xC858, 0x0002, WORD_LEN);	//* Coarse IT Min (A) = 2 
	mt9p111_i2c_write(MT9P111_ID, 0xC85A, 0x0001, WORD_LEN);	//* Coarse IT Max Margin (A) = 1 
	mt9p111_i2c_write(MT9P111_ID, 0xC85C, 0x0423, WORD_LEN);	//* Min Frame Lines (A) = 1059 
	mt9p111_i2c_write(MT9P111_ID, 0xC85E, 0xFFFF, WORD_LEN);	//* Max Frame Lines (A) = 65535 
	mt9p111_i2c_write(MT9P111_ID, 0xC860, 0x0423, WORD_LEN);	//* Base Frame Lines (A) = 1059 
	mt9p111_i2c_write(MT9P111_ID, 0xC862, 0x0EDB, WORD_LEN);	//* Min Line Length (A) = 3803 
	mt9p111_i2c_write(MT9P111_ID, 0xC864, 0xFFFE, WORD_LEN);	//* Max Line Length (A) = 65534 
	mt9p111_i2c_write(MT9P111_ID, 0xC866, 0x7F7C, WORD_LEN);	//* P456 Divider (A) = 32636 
	mt9p111_i2c_write(MT9P111_ID, 0xC868, 0x0423, WORD_LEN);	//* Frame Lines (A) = 1059 
	mt9p111_i2c_write(MT9P111_ID, 0xC86A, 0x0EDB, WORD_LEN);	//* Line Length (A) = 3803 
	mt9p111_i2c_write(MT9P111_ID, 0xC870, 0x0014, WORD_LEN);	//* RX FIFO Watermark (A) = 20 
	mt9p111_i2c_write(MT9P111_ID, 0xC8AA, MT9P_IMG_WIDTH, WORD_LEN); 	// CAM_OUTPUT_0_IMAGE_WIDTH
	mt9p111_i2c_write(MT9P111_ID, 0xC8AC, MT9P_IMG_HEIGHT, WORD_LEN); 	// CAM_OUTPUT_0_IMAGE_HEIGHT
	mt9p111_i2c_write(MT9P111_ID, 0x8404, 0x06, BYTE_LEN); 	// SEQ_CMD
	delay_ms(100);


	printf( "MT9P111 Initialization Done! \n\r");
}

