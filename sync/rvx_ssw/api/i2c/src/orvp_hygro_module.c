// ****************************************************************************
// Copyright SoC Design Research Group, All rights reserved.    
// Electronics and Telecommunications Research Institute (ETRI)
//
// THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
// WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
// TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
// REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
// SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
// IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
// COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
// ****************************************************************************
// 13.Apr.2017 
// Sukho Lee (shle99@etri.re.kr)
// ****************************************************************************
// ****************************************************************************

#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "frvp_spi.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "orvp_hygro_module.h"
#include "orvp_i2c.h"

#define I2C_INDEX_FOR_HYGRO_MODULE	0
#define I2C_FREQ_OF_HYGRO_MODULE	500

#define HYGRO_MODULE_ADDR               0x40

#define TEMPERATURE_REG_ADDR            0x00    // read only
#define HUMIDITY_REG_ADDR               0x01    // read only
#define CONFIGURATION_REG_ADDR          0x02

/* Configuration Register */
#define HYGRO_COFIG_REG_HI_RST              (1<<7)
#define HYGRO_COFIG_REG_HI_HEAT             (1<<5)
#define HYGRO_COFIG_REG_HI_MODE             (1<<4)
#define HYGRO_COFIG_REG_HI_BTST             (1<<3)
#define HYGRO_COFIG_REG_HI_TRES             (1<<2)
#define HYGRO_COFIG_REG_HI_HRES_14_BIT      0x0
#define HYGRO_COFIG_REG_HI_HRES_11_BIT      0x1
#define HYGRO_COFIG_REG_HI_HRES_8_BIT       0x2
#define HYGRO_COFIG_REG_LO                  0x0

#define NUM_BYTES_OF_COFIG_REG			2
#define NUM_BYTES_OF_TEMPERATURE_DATA		2
#define NUM_BYTES_OF_HUMIDITY_DATA		2

void configure_i2c_for_hygro_module()
{
	configure_i2c(I2C_INDEX_FOR_HYGRO_MODULE, I2C_FREQ_OF_HYGRO_MODULE, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void configure_hygro_module_for_temperature_or_humidity()
{
	unsigned char config_reg[NUM_BYTES_OF_COFIG_REG];
	config_reg[0] = 0x0;
	config_reg[1] = HYGRO_COFIG_REG_LO;

	write_i2c_r1dn(I2C_INDEX_FOR_HYGRO_MODULE, HYGRO_MODULE_ADDR, CONFIGURATION_REG_ADDR, config_reg, NUM_BYTES_OF_COFIG_REG);
}

int read_temperature()
{
	int temperature, temp;
	unsigned char data[NUM_BYTES_OF_TEMPERATURE_DATA];

	read_i2c_r1dn(I2C_INDEX_FOR_HYGRO_MODULE, HYGRO_MODULE_ADDR, TEMPERATURE_REG_ADDR, data, NUM_BYTES_OF_TEMPERATURE_DATA);
	temp = (data[0] << 8) | data[1];
	temperature = ((temp * 0xa5) >> 16) - 0x28;

	return temperature;
}

int read_humidity()
{
	int humidity, temp;
	unsigned char data[NUM_BYTES_OF_HUMIDITY_DATA];

	read_i2c_r1dn(I2C_INDEX_FOR_HYGRO_MODULE, HYGRO_MODULE_ADDR, HUMIDITY_REG_ADDR, data, NUM_BYTES_OF_HUMIDITY_DATA);
	temp = (data[0] << 8) | data[1];
	humidity = ((temp * 0x64) >> 16);

	return humidity;
}

void configure_hygro_module_for_temperature_and_humidity()
{
	unsigned char config_reg[NUM_BYTES_OF_COFIG_REG];
	config_reg[0] = HYGRO_COFIG_REG_HI_MODE;
	config_reg[1] = HYGRO_COFIG_REG_LO;

	write_i2c_r1dn(I2C_INDEX_FOR_HYGRO_MODULE, HYGRO_MODULE_ADDR, CONFIGURATION_REG_ADDR, config_reg, NUM_BYTES_OF_COFIG_REG);
}

void read_temperature_and_humidity(int *temperature, int *humidity)
{
	int temp;
	unsigned char data[NUM_BYTES_OF_TEMPERATURE_DATA+NUM_BYTES_OF_HUMIDITY_DATA];

	read_i2c_r1dn(I2C_INDEX_FOR_HYGRO_MODULE, HYGRO_MODULE_ADDR, TEMPERATURE_REG_ADDR, data, NUM_BYTES_OF_TEMPERATURE_DATA+NUM_BYTES_OF_HUMIDITY_DATA);

	// temperature data
	temp = (data[0] << 8) | data[1];
	*temperature = ((temp * 0xa5) >> 16) - 0x28;

	// humidity data
	temp = (data[2] << 8) | data[3];
	*humidity = ((temp * 0x64) >> 16);
}
