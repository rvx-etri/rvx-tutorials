#include "platform_info.h"
#include "ov5640_mp5_regs.h"
#include "orvp_i2c.h"
#include "ervp_printf.h"
#include "ervp_delay.h"
#include "orvp_i2c_driver.h"

#define I2C_FREQ_OF_CIS_OV5640         100000
//#define I2C_FREQ_OF_CIS_OV5640         10000
#define OV5640_IMG_WIDTH   1024
#define OV5640_IMG_HEIGHT   600


void configure_i2c_for_cis_ov5640()
{
        configure_i2c(I2C_INDEX_FOR_CIS, I2C_FREQ_OF_CIS_OV5640, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void cis_ov5640_write_i2c(unsigned char id, unsigned int addr, unsigned char data)
{
        //id = id >> 1;

        // Addr : 2byte, Data : 1byte
        write_i2c_r2d1(I2C_INDEX_FOR_CIS, id, addr, data);
        delay_ms(15);
}

//void cis_ov5640_write_i2c(unsigned int addr, unsigned char data)
//{
//
//        start_i2c_transmisstion(I2C_INDEX_FOR_CIS, OV5640_MP5_SENSOR_ADDR);
//        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr>>8);
//        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr&0xff);
//        // writing
//        finish_writing_a_byte_on_i2c(I2C_INDEX_FOR_CIS, data);
//}

void cis_ov5640_write_sensor_vals(sensor_reg reglist[])
{
        unsigned int addr = 0;
        unsigned char data = 0;
        sensor_reg *next = reglist;

        while((addr != 0xffff) | (data != 0xff)) {
                addr = next->reg;
                data = next->val;
                cis_ov5640_write_i2c(OV5640_MP5_SENSOR_ADDR, addr, data);
                next++;
        }

       printf("CIS configration Done.\n");
}

unsigned char cis_ov5640_read_i2c(unsigned int addr)
{
        unsigned char data;

        start_i2c_transmisstion(I2C_INDEX_FOR_CIS, OV5640_MP5_SENSOR_ADDR);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr>>8);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr&0xff);

        start_i2c_reading(I2C_INDEX_FOR_CIS, OV5640_MP5_SENSOR_ADDR);

        // writing
        data = finish_reading_a_byte_on_i2c(I2C_INDEX_FOR_CIS);

        return data;
}


//unsigned int cis_ov5640_read_i2c(unsigned int id, unsigned int addr)
//{
//        unsigned int data;
//
//        id = id >> 1;
//
//        // Addr : 2byte, Data : 1byte
//        data = read_i2c_r2d1(I2C_INDEX_FOR_CIS, id, addr);
//
//        return data;
//}

void cis_ov5640_check_test_reg(void)
{
        int vid, pid;
        unsigned char autofocus;
        int temp;

        printf("OV5640_MP5 CAM CHECK TEST REG!\n");

        // i2c test
        configure_i2c_for_cis_ov5640();
        printf("config done.\n");


        //cis_ov5640_write_i2c(OV5640_MP5_SENSOR_ADDR, 0x3103, 0x11);  // SCCB system ctrl1 
        cis_ov5640_write_i2c(OV5640_MP5_SENSOR_ADDR, 0x3008, 0x82);  // software reset 

        vid = cis_ov5640_read_i2c(OV5640_MP5_CHIPID_HIGH);
        pid = cis_ov5640_read_i2c(OV5640_MP5_CHIPID_LOW);

        //cis_ov5640_write_i2c(OV5640_MP5_SENSOR_ADDR, 0x3037, 0x12);
        //temp = cis_ov5640_read_i2c(0x3037);

        //printf("temp(0x02) : %02X\n", temp);

        printf("vid(0x56) : %02X\n", vid);
        printf("pid(0x40) : %02X\n", pid);

        //cis_ov5640_write_sensor_vals(OV5640_Auto_Focus);

        delay_ms(15);

        cis_ov5640_write_i2c(OV5640_MP5_SENSOR_ADDR, 0x3022, 0x03);
 
        //autofocus = cis_ov5640_read_i2c(0x3029);
       
#if 0 
        if(autofocus==0x10) 
                printf("auto focus config done.\n");
        else
                printf("auto focus config error.\n");
#endif

}

