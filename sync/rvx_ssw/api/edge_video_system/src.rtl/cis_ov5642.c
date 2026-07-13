#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"
#include "ervp_delay.h"
#include "orvp_i2c_driver.h"

#include "ov5642_mp5_regs.h"
#include "orvp_i2c.h"

#define I2C_FREQ_OF_CIS_OV5642         100000
//#define I2C_FREQ_OF_CIS_OV5642         10000
#define OV5642_IMG_WIDTH   1024
#define OV5642_IMG_HEIGHT   600


void configure_i2c_for_cis_ov5642()
{
        configure_i2c(I2C_INDEX_FOR_CIS, I2C_FREQ_OF_CIS_OV5642, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void cis_ov5642_write_i2c(unsigned char id, unsigned int addr, unsigned char data)
{
        //id = id >> 1;

        // Addr : 2byte, Data : 1byte
        write_i2c_r2d1(I2C_INDEX_FOR_CIS, id, addr, data);
        delay_ms(15);
}

//void cis_ov5642_write_i2c(unsigned int addr, unsigned char data)
//{
//
//        start_i2c_transmisstion(I2C_INDEX_FOR_CIS, OV5642_MP5_SENSOR_ADDR);
//        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr>>8);
//        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr&0xff);
//        // writing
//        finish_writing_a_byte_on_i2c(I2C_INDEX_FOR_CIS, data);
//}

void cis_ov5642_write_sensor_vals(sensor_reg reglist[])
{
        unsigned int addr = 0;
        unsigned char data = 0;
        sensor_reg *next = reglist;

        while((addr != 0xffff) | (data != 0xff)) {
                addr = next->reg;
                data = next->val;
                cis_ov5642_write_i2c(OV5642_MP5_SENSOR_ADDR, addr, data);
                next++;
        }

       printf("CIS configration Done.\n");
}

unsigned char cis_ov5642_read_i2c(unsigned int addr)
{
        unsigned char data;

        start_i2c_transmisstion(I2C_INDEX_FOR_CIS, OV5642_MP5_SENSOR_ADDR);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr>>8);
        write_a_byte_on_i2c(I2C_INDEX_FOR_CIS, addr&0xff);

        start_i2c_reading(I2C_INDEX_FOR_CIS, OV5642_MP5_SENSOR_ADDR);

        // writing
        data = finish_reading_a_byte_on_i2c(I2C_INDEX_FOR_CIS);

        return data;
}


//unsigned int cis_ov5642_read_i2c(unsigned int id, unsigned int addr)
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

void cis_ov5642_check_test_reg(void)
{
        int vid, pid;

        printf("OV5642_MP5 CAM CHECK TEST REG!\n");

        // i2c test
        configure_i2c_for_cis_ov5642();
        printf("config done.\n");


        cis_ov5642_write_i2c(OV5642_MP5_SENSOR_ADDR, 0x3008, 0x80);  // software reset

        vid = cis_ov5642_read_i2c(OV5642_MP5_CHIPID_HIGH);
        pid = cis_ov5642_read_i2c(OV5642_MP5_CHIPID_LOW);
        //temp = cis_ov5642_read_i2c(0x3008);
        //printf("temp(0x80) : %02X\n", temp);

        printf("vid(0x56) : %02X\n", vid);
        printf("pid(0x42) : %02X\n", pid);
}

