#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"
#include "ov2640_regs.h"
#include "orvp_i2c.h"
#include "ervp_delay.h"
#include "orvp_i2c_driver.h"

#define I2C_FREQ_OF_CIS_OV2640         100000
//#define I2C_FREQ_OF_CIS_OV2640         10000
#define OV2640_IMG_WIDTH   1024
#define OV2640_IMG_HEIGHT   600


void configure_i2c_for_cis_ov2640()
{
        configure_i2c(I2C_INDEX_FOR_CIS, I2C_FREQ_OF_CIS_OV2640, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void cis_ov2640_write_i2c(unsigned char id, unsigned char addr, unsigned char data)
{
        id = id >> 1;

        // Addr : 1byte, Data : 1byte
        write_i2c_r1d1(I2C_INDEX_FOR_CIS, id, addr, data);
        delay_ms(15);
}

void cis_ov2640_write_sensor_vals(sensor_reg reglist[])
{
        unsigned char addr = 0;
        unsigned char data = 0;
        sensor_reg *next = reglist;


                cis_ov2640_write_i2c(OV2640_SENSOR_ADDR, 0xff, 0x01);
                cis_ov2640_write_i2c(OV2640_SENSOR_ADDR, 0xff, 0x80);
                delay_ms(100);

        while((addr != 0xff) | (data != 0xff)) {
                addr = next->reg;
                data = next->val;
                cis_ov2640_write_i2c(OV2640_SENSOR_ADDR, addr, data);
                next++;
        }
       printf("CIS configration Done.\n");
}

unsigned int cis_ov2640_read_i2c(unsigned int id, unsigned int addr)
{
        unsigned int data;

        id = id >> 1;

        // Addr : 1byte, Data : 1byte
        data = read_i2c_r1d1(I2C_INDEX_FOR_CIS, id, addr);

        return data;
}

void cis_ov2640_check_test_reg(void)
{
        int vid, pid;
        int temp;

        printf("OV2640 CAM CHECK TEST REG!\n");

        // i2c test
        configure_i2c_for_cis_ov2640();

        cis_ov2640_write_i2c(OV2640_SENSOR_ADDR,0xff, 0x01);  // table 13
        cis_ov2640_write_i2c(OV2640_SENSOR_ADDR,0x12, 0x80);  // reset release
        vid = cis_ov2640_read_i2c(OV2640_SENSOR_ADDR, OV2640_CHIPID_HIGH);
        pid = cis_ov2640_read_i2c(OV2640_SENSOR_ADDR, OV2640_CHIPID_LOW);

        printf("vid(0x26) : %02X\n", vid);
        printf("pid(0x42) : %02X\n", pid);
}

