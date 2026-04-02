#include "ervp_mmio_util.h"
#include "ervp_printf.h"

#include "orvp_i2c_defines.h"
#include "orvp_i2c_driver.h"

void check_i2c_write(int i2c_index) {
	if(REG32(get_i2c_base_addr(i2c_index) + I2C_SR) & I2C_RXACK)
		printf("\nDid not receive acknowledge from module!\n");
}

void wait_i2c_processing(int i2c_index) {	// wait for TIP flag to negate
	while (REG32(get_i2c_base_addr(i2c_index) + I2C_SR) & I2C_TIP)
		IDLE;
}

void start_i2c_transmisstion(int i2c_index, unsigned char module_address)
{
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	/* begin transmmission */
	REG32(i2c_addr + I2C_TXR) = (module_address<<1) & ~I2C_TXR_RW_MASK; // module address + rw bit(0)
	REG32(i2c_addr + I2C_CR) = I2C_STA | I2C_WR;  // set command  (start, write)
	wait_i2c_processing(i2c_index);
	check_i2c_write(i2c_index);
}

void write_a_byte_on_i2c(int i2c_index, unsigned char data)
{
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	REG32(i2c_addr + I2C_TXR) = data;
	REG32(i2c_addr + I2C_CR) = I2C_WR;
	wait_i2c_processing(i2c_index);
	check_i2c_write(i2c_index);
}

void finish_writing_a_byte_on_i2c(int i2c_index, unsigned char data)
{
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	REG32(i2c_addr + I2C_TXR) = data;
	REG32(i2c_addr + I2C_CR) = I2C_WR | I2C_STO; // set command (stop, write)
	wait_i2c_processing(i2c_index);
	check_i2c_write(i2c_index);
}

void start_i2c_reading(int i2c_index, unsigned char module_address)
{
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	REG32(i2c_addr + I2C_TXR) = (module_address<<1) | I2C_TXR_RW_MASK; // module address + rw bit(1)
	REG32(i2c_addr + I2C_CR) = I2C_STA | I2C_WR;  // set command  (start, write)
	wait_i2c_processing(i2c_index);
	check_i2c_write(i2c_index);
}

unsigned char read_a_byte_on_i2c(int i2c_index)
{
	unsigned char data;
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	REG32(i2c_addr + I2C_CR) = I2C_RD & ~I2C_ACK; // set command (read, 0x40)
	wait_i2c_processing(i2c_index);
	data = (unsigned char)(REG32(i2c_addr + I2C_RXR) & 0xFF);
	return data;
}

unsigned char finish_reading_a_byte_on_i2c(int i2c_index)
{
	unsigned char data;
	mmio_addr_t i2c_addr = get_i2c_base_addr(i2c_index);
	REG32(i2c_addr + I2C_CR) = I2C_RD | I2C_ACK | I2C_STO; // set command (stop)
	wait_i2c_processing(i2c_index);
	data = (unsigned char)(REG32(i2c_addr + I2C_RXR) & 0xFF);
	return data;
}

