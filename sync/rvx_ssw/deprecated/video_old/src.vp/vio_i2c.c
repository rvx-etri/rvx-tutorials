#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"

#include "vio_i2c.h"
#include "video_memory_map.h"
#include "vio_util.h"

void i2c_delay(int ms)
{
	//delay_time(ms*(1000000/80));
	delay_ms(3*ms);
}

void i2c_init(int sel, int id)
{
	if(sel == I2C_CIS) write_reg(I2C_CIS_BASE  + I2C_ID, id);
	else			   write_reg(I2C_HDMI_BASE + I2C_ID, id); 
}

void i2c_write(int sel, unsigned int addr, unsigned int data, int data_len)
{
	unsigned int base;

	if(sel == I2C_CIS) base = I2C_CIS_BASE;
	else               base = I2C_HDMI_BASE;

	write_reg((base + I2C_CNTR), 0);
	write_reg((base + I2C_RWCNTR), data_len);
	write_reg((base + I2C_ADDR), addr);
	write_reg((base + I2C_DATA), data);
	i2c_delay(3);
}

unsigned int i2c_read(int sel, unsigned int addr, int data_len)
{
	unsigned int base, data;

	if(sel == I2C_CIS) base = I2C_CIS_BASE;
	else               base = I2C_HDMI_BASE;

	write_reg((base + I2C_CNTR), 0);
	write_reg((base + I2C_RWCNTR), 4 | data_len);
	write_reg((base + I2C_ADDR), addr);
	i2c_delay(3);
	data = read_reg(base + I2C_DATA);
	
	return data;
}

void i2c_write2addr(int sel, unsigned int addr, unsigned int data, int data_len)
{
	int i;
	unsigned int base;

	printf(" i2c_write :%d %04X-%04X %d\n", sel, addr, data, data_len);

	if(sel == I2C_CIS) base = I2C_CIS_BASE;
	else               base = I2C_HDMI_BASE;

	write_reg((base + I2C_CNTR), 8);
	write_reg((base + I2C_RWCNTR), data_len);

	for(i=0; i<1; i++) {
		write_reg((base + I2C_ADDR), addr);
		write_reg((base + I2C_DATA), data);
		i2c_delay(5);
	}
}

unsigned int i2c_read2addr(int sel, unsigned int addr, int data_len)
{
	unsigned int base, data;

	if(sel == I2C_CIS) base = I2C_CIS_BASE;
	else               base = I2C_HDMI_BASE;

	write_reg((base + I2C_CNTR), 8);
	write_reg((base + I2C_RWCNTR), 4 | data_len);
	write_reg((base + I2C_ADDR), addr);
	i2c_delay(3);
	data = read_reg(base + I2C_DATA);

	return data;
}
