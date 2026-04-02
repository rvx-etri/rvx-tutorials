#ifndef _H_VIOI2C_H_
#define _H_VIOI2C_H_

#define I2C_CIS		0
#define I2C_HDMI	1

// addr
#define I2C_CNTR		0x000
#define I2C_ID			0x004
#define I2C_RWCNTR		0x008
#define I2C_ADDR		0x00C
#define I2C_DATA		0x010

void i2c_init(int sel, int id);
void i2c_write(int sel, unsigned int addr, unsigned int data, int data_len);
unsigned int i2c_read(int sel, unsigned int addr, int data_len);
void i2c_write2addr(int sel, unsigned int addr, unsigned int data, int data_len);
unsigned int i2c_read2addr(int sel, unsigned int addr, int data_len);
void i2c_delay(int ms);

#endif
