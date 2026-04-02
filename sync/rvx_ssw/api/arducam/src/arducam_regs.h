#ifndef __ARDUCAM_REGS_H__
#define __ARDUCAM_REGS_H__

// Arducam Register
#define regSPICAM_TEST				0x00
#define regSPICAM_CAPTURE			0x01
#define regSPICAM_MODE				0x02
#define regSPICAM_SENSOR_TIMING		0x03
#define regSPICAM_FIFO          	0x04
#define regSPICAM_GPIO_DIR      	0x05
#define regSPICAM_GPIO_WRITE    	0x06
#define regSPICAM_BURST_READ    	0x3C
#define regSPICAM_SINGLE_READ   	0x3D
#define regSPICAM_VERSION       	0x40
#define regSPICAM_STATUS        	0x41
#define regSPICAM_FIFOSIZE1     	0x42
#define regSPICAM_FIFOSIZE2     	0x43
#define regSPICAM_FIFOSIZE3     	0x44

#define ARDUCAM_MAX_FIFO_SIZE		0x007FFFFF	//8MByte

// regSPICAM_SENSOR_TIMING bits
#define HREF_LEVEL_MASK    0x01  //0 = High active , 	1 = Low active
#define VSYNC_LEVEL_MASK   0x02  //0 = High active , 	1 = Low active
#define LCD_BKEN_MASK      0x04  //0 = Enable, 			1 = Disable
#define PCLK_DELAY_MASK    0x08  //0 = data no delay,	1 = data delayed one PCLK

//
#define CLEAR_WRITE_FIFO_DONE_FLAG              (1<<0)
#define START_CAPTURE                           (1<<1)
#define CAMERA_WRITE_FIFO_DONE_FLAG             (1<<3)

#endif
