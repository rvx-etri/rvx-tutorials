#include "ervp_multicore_synch.h"
#include "ervp_mmio_util.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_external_peri_group_memorymap.h"

#include "orvp_i2c_defines.h"
#include "orvp_i2c_driver.h"
#include "orvp_i2c.h"

void configure_i2c(int i2c_index, int target_freq, int i2c_enable, int i2c_interrupt_enable)
{
	unsigned int divisor;
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);

	divisor = I2C_CLK_HZ/(5 * target_freq) - 1;
	REG32(i2c_addr + I2C_PRER_LO)= 0xff & divisor;
	REG32(i2c_addr + I2C_PRER_HI)= 0xff & (divisor>>8);
	//printf("divisor : 0x%x\n", divisor);
	//printf("I2C_PRER_LO: 0x%x\n", REG32(get_i2c_base_addr(i2c_index) + I2C_PRER_LO)); 
	//printf("I2C_PRER_HI: 0x%x\n", REG32(get_i2c_base_addr(i2c_index) + I2C_PRER_HI)); 
	REG32(i2c_addr + I2C_CTR)= (I2C_EN*i2c_enable)
		| (I2C_IEN*i2c_interrupt_enable);
}

void write_i2c_r0dn(int i2c_index, uint8_t module_address, uint8_t *data, int num_bytes) {
	int i = 0;

	start_i2c_transmisstion(i2c_index, module_address);

	for(i=0; i<num_bytes; i++) {
		if(i!=(num_bytes-1))
			write_a_byte_on_i2c(i2c_index, data[i]);
		else
			finish_writing_a_byte_on_i2c(i2c_index, data[i]);
	}
}

void write_i2c_r1dn(int i2c_index, uint8_t module_address, uint8_t reg_address, uint8_t *data, int num_bytes) {
	int i = 0;

	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address);

	for(i=0; i<num_bytes; i++) {
		if(i!=(num_bytes-1))
			write_a_byte_on_i2c(i2c_index, data[i]);
		else
			finish_writing_a_byte_on_i2c(i2c_index, data[i]);
	}
}

void read_i2c_r0dn(int i2c_index, uint8_t module_address, uint8_t *data, int num_bytes) {
	int i = 0;

	start_i2c_reading(i2c_index, module_address);

	for(i=0; i<num_bytes; i++) {
		if(i!=(num_bytes-1))
			data[i] = read_a_byte_on_i2c(i2c_index);
		else
			data[i] = finish_reading_a_byte_on_i2c(i2c_index);
	}
}

void read_i2c_r1dn(int i2c_index, uint8_t module_address, uint8_t reg_address, uint8_t *data, int num_bytes) {
	int i = 0;

	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address);

	start_i2c_reading(i2c_index, module_address);

	for(i=0; i<num_bytes; i++) {
		if(i!=(num_bytes-1))
			data[i] = read_a_byte_on_i2c(i2c_index);
		else
			data[i] = finish_reading_a_byte_on_i2c(i2c_index);
	}
}

uint8_t read_i2c_r1d1(int i2c_index, uint8_t module_address, uint8_t reg_address)
{
	uint8_t data;

	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address);
	//finish_writing_a_byte_on_i2c(i2c_index, reg_address);

	start_i2c_reading(i2c_index, module_address);

	data = finish_reading_a_byte_on_i2c(i2c_index);

	return data;
}

void write_i2c_r1d1(int i2c_index, uint8_t module_address, uint8_t reg_address, uint8_t data)
{
	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address);

	finish_writing_a_byte_on_i2c(i2c_index, data);
}

void write_i2c_r2d1(int i2c_index, uint8_t module_address, uint16_t reg_address, uint8_t data)
{
	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address>>8);
	write_a_byte_on_i2c(i2c_index, reg_address&0xFF);

	finish_writing_a_byte_on_i2c(i2c_index, data);
}

void write_i2c_r2d2(int i2c_index, uint8_t module_address, uint16_t reg_address, uint16_t data)
{
	start_i2c_transmisstion(i2c_index, module_address);

	write_a_byte_on_i2c(i2c_index, reg_address>>8);
	write_a_byte_on_i2c(i2c_index, reg_address&0xFF);

	write_a_byte_on_i2c(i2c_index, data>>8);
	finish_writing_a_byte_on_i2c(i2c_index, data&0xFF);
}
