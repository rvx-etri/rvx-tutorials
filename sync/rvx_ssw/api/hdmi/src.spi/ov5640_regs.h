#ifndef OV5640_REGS_H
#define OV5640_REGS_H

#include "types.h"

#define OV5640_SENSOR_ADDR  (0x3C)
#define OV5640_CHIPID_HIGH	(0x300A)
#define OV5640_CHIPID_LOW	(0x300B)

/*
typedef struct {
	unsigned short reg;
	unsigned short val;
} sensor_reg;
*/

typedef enum {
	OV5640_720P         = 0,
	OV5640_1080P_15FPS  = 1,
	OV5640_1080P_30FPS  = 2
} eOV5640_SIZE;



extern sensor_reg  OV5640_cfg_init_[];
extern sensor_reg OV5640_cfg_720p_60fps_[];
extern sensor_reg OV5640_cfg_1080p_15fps_[];
extern sensor_reg OV5640_cfg_1080p_30fps_[];
extern sensor_reg  OV5640_cfg_advanced_awb_[];
extern sensor_reg  OV5640_cfg_simple_awb_[];
extern sensor_reg  OV5640_cfg_disable_awb_[];




#endif