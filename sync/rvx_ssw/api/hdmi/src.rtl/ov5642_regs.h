#ifndef __OV5642_REGS_H__
#define __OV5642_REGS_H__

#include "types.h"
//#include "arducam.h"
//#include "arducam_arch.h"
//#include <avr/pgmspace.h>

#define OV5642_SENSOR_ADDR  (0x3C)
#define OV5642_CHIPID_HIGH	(0x300A)
#define OV5642_CHIPID_LOW	(0x300B)

/*
typedef struct {
	unsigned short reg;
	unsigned short val;
} sensor_reg;
*/

typedef enum {
	OV5642_RAW_FHD  = 0,
	OV5642_RAW_HD   = 1,
	OV5642_RAW_VGA  = 2,
	OV5642_YUV_QVGA = 3,
	OV5642_YUV_VGA  = 4,
} eOV5642_SIZE;

/*
struct sensor_reg {
	unsigned short reg;
	unsigned short val;
};
*/

extern sensor_reg OV5642_VGA_preview_setting[];
extern sensor_reg ov5642_RAW[];
extern sensor_reg OV5642_1280x960_RAW[];
extern sensor_reg OV5642_1920x1080_RAW[];
extern sensor_reg OV5642_640x480_RAW[];
extern sensor_reg OV5642_QVGA_YUV[]; 
extern sensor_reg OV5642_VGA_YUV[]; 



#endif // header
