#ifndef _H_CAMMIPI_H_
#define _H_CAMMIPI_H_

#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_external_peri_group_memorymap_offset.h"
#include "frvp_spi.h"
#include "ervp_reg_util.h"
#include "ervp_misc_util.h"
#include "types.h"
#include "orvp_i2c.h"
#include "ov5640_regs.h"


typedef enum {
	MODE_720P_1280_720_60fps = 0, 
	MODE_1080P_1920_1080_15fps, 
	MODE_1080P_1920_1080_30fps, 
	MODE_END
} mode_t;




// I2C (Sensor)
#define I2C_INDEX_FOR_MIPI_CAM 0
#define I2C_FREQ_OF_MIPI_CAM   400000



// function
void cam_mipi_init_hw(void);
int cam_mipi_setup(void);
void cam_mipi_config_sensor(eOV5640_SIZE size);
void cam_mipi_config_sensor_size(mode_t mode);
void cam_mipi_write_i2c(uint16_t addr, uint8_t data);
uint8_t cam_mipi_read_i2c(uint16_t addr);
void cam_mipi_write_sensor_vals(sensor_reg reglist[]);




#endif