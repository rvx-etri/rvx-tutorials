#ifndef _H_CAMSPI_H_
#define _H_CAMSPI_H_

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
#include "ov5642_regs.h"

// SPI (ArduCam)
#define SPI_FREQ_OF_SPI_CAM   5000000        // 5MHz (max 8MHz)
#define SPI_MODE_OF_SPI_CAM   SPI_SCKMODE_0
#define PORT_ID_OF_SPI_CAM    0

#ifndef SPI_BASEADDR
#define SPI_BASEADDR        (EXTERNAL_PERI_GROUP_BASEADDR+SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_SPI_GROUP0)
#endif

// Arducam Register
#define regSPICAM_TEST			0x00
#define regSPICAM_CAPTURE		0x01
#define regSPICAM_MODE			0x02
#define regSPICAM_SENSOR_TIMING	0x03
#define regSPICAM_FIFO          0x04
#define regSPICAM_GPIO_DIR      0x05
#define regSPICAM_GPIO_WRITE    0x06
#define regSPICAM_BURST_READ    0x3C
#define regSPICAM_SINGLE_READ   0x3D
#define regSPICAM_VERSION       0x40
#define regSPICAM_STATUS        0x41
#define regSPICAM_FIFOSIZE1     0x42
#define regSPICAM_FIFOSIZE2     0x43
#define regSPICAM_FIFOSIZE3     0x44

//
#define CLEAR_WRITE_FIFO_DONE_FLAG              (1<<0)
#define START_CAPTURE                           (1<<1)
#define CAMERA_WRITE_FIFO_DONE_FLAG             (1<<3)


// I2C (Sensor)
#define I2C_INDEX_FOR_SPI_CAM 1
//#define I2C_FREQ_OF_SPI_CAM   500
#define I2C_FREQ_OF_SPI_CAM   400000

// function
void cam_spi_init_hw(void);
int cam_spi_setup(void);
void cam_spi_config_sensor(eOV5642_SIZE size);
int cam_spi_capture_single(uint8_t *img);
int cam_spi_capture_single_yuv(uint8_t *img, int width, int height, int stride);
uint32_t cam_spi_read_fifo_size(void);
void cam_spi_write_spi(uint8_t addr, uint8_t data);
uint8_t cam_spi_read_spi(uint8_t addr);
void cam_spi_write_i2c(uint16_t addr, uint8_t data);
void cam_spi_write_sensor_vals(sensor_reg reglist[]);
uint8_t cam_spi_read_i2c(uint16_t addr);


#endif