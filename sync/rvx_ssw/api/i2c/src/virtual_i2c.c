#include "ervp_misc_util.h"
#include "ervp_external_peri_group_api.h"
#include "orvp_i2c.h"
#include "virtual_i2c.h"
#include "ervp_printf.h"

const int VIRTUAL_I2C_FREQ = 100000;

void config_virutal_i2c(int index)
{
	printf("Virtual I2C Test\n");
	configure_i2c(index, VIRTUAL_I2C_FREQ, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void test_virutal_i2c(int index)
{
	unsigned char wdata = 0x89;
	unsigned char rdata;
	config_virutal_i2c(index);
	write_i2c_r1d1(index, VIRTUAL_I2C_MODULE_ADDR, 0, wdata);
	printf("------------------------\n");
	rdata = read_i2c_r1d1(index, VIRTUAL_I2C_MODULE_ADDR, 0);
	if(wdata==rdata)
		printf("Ok!\n");
	else
		printf("NOT Ok!\n");
}
