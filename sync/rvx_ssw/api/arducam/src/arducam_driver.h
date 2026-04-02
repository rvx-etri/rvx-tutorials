#ifndef __ARDUCAM_DRIVER_H__
#define __ARDUCAM_DRIVER_H__

#include <stdint.h>
#include "platform_info.h"
#include "ervp_external_peri_group_memorymap.h"
#include "frvp_spi.h"
#include "orvp_i2c.h"

#include "arducam_regs.h"
#include "ov5642_regs.h"

// SPI (ArduCam)
#define ARDUCAM_SPI_PORT_ID		(SPI_INDEX_FOR_ARDUCAM)
#define ARDUCAM_SPI_FREQ		(6250000)        // 6.25MHz (max 8MHz)
#define ARDUCAM_SPI_MODE		(SPI_SCKMODE_0)

// I2C (Sensor)
#define ARDUCAM_I2C_INDEX (I2C_INDEX_FOR_ARDUCAM)
#define ARDUCAM_I2C_FREQ   (400000)

// function
void arducam_spi_write_abyte(uint8_t addr, uint8_t data);
uint8_t arducam_spi_read_abyte(uint8_t addr);
void arducam_spi_read_bytes(uint8_t addr, uint8_t *data, int32_t num);
void arducam_i2c_write(uint16_t addr, uint8_t data);
void arducam_write_sensor_vals(sensor_reg reglist[]);
uint8_t arducam_i2c_read(uint16_t addr);

#endif
