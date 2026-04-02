#include "arducam_driver.h"
#include "orvp_i2c_driver.h"

// I/O Interfacing
void arducam_spi_write_abyte(uint8_t addr, uint8_t data)
{
	addr |= (1<<7); // write command

	spi_start();
	spi_write(1, &addr);
	spi_write(1, &data);
	spi_end();
}

uint8_t arducam_spi_read_abyte(uint8_t addr)
{
	uint8_t data;

	addr &= ~(1<<7);

	spi_start();
	spi_write(1, &addr);
	spi_read(1, &data);
	spi_end();

	return data;
}

void arducam_spi_read_bytes(uint8_t addr, uint8_t *data, int32_t num)
{
	addr &= ~(1<<7);
	spi_start();
	spi_write(1, &addr);
	spi_read(num, data);
	spi_end();
}

void arducam_i2c_write(uint16_t addr, uint8_t data)
{
	start_i2c_transmisstion(ARDUCAM_I2C_INDEX, OV5642_SENSOR_ADDR);
	write_a_byte_on_i2c(ARDUCAM_I2C_INDEX, addr>>8);
	write_a_byte_on_i2c(ARDUCAM_I2C_INDEX, addr&0xff);
	finish_writing_a_byte_on_i2c(ARDUCAM_I2C_INDEX, data);
}

void arducam_write_sensor_vals(sensor_reg reglist[])
{
	uint32_t addr;
	uint8_t data;
	sensor_reg *next = reglist;

	while((addr != 0xffff) | (data != 0xff)) {
		addr = next->reg;
		data = next->val;
		arducam_i2c_write(addr, data);
		next++;
	}
}

uint8_t arducam_i2c_read(uint16_t addr)
{
	uint8_t data;

	start_i2c_transmisstion(ARDUCAM_I2C_INDEX, OV5642_SENSOR_ADDR);
	write_a_byte_on_i2c(ARDUCAM_I2C_INDEX, addr>>8);
	write_a_byte_on_i2c(ARDUCAM_I2C_INDEX, addr&0xff);

	start_i2c_reading(ARDUCAM_I2C_INDEX, OV5642_SENSOR_ADDR);
	data = finish_reading_a_byte_on_i2c(ARDUCAM_I2C_INDEX);

	return data;
}
