#ifndef OV5642_MP5_REGS_H
#define OV5642_MP5_REGS_H
//#include "ArduCAM.h"
//#include <avr/pgmspace.h>

#define OV5642_MP5_SENSOR_ADDR (0x3C) 
//#define OV5642_MP5_SENSOR_ADDR (0x78) 
#define OV5642_MP5_CHIPID_HIGH 0x300a
#define OV5642_MP5_CHIPID_LOW 0x300b

typedef struct {
        unsigned short reg;
        unsigned short val;
} sensor_reg;

extern sensor_reg ov5642_MP5_RAW[];
extern sensor_reg OV5642_MP5_1280x960_RAW[];
extern sensor_reg OV5642_MP5_1920x1080_RAW[];
extern sensor_reg OV5642_MP5_640x480_RAW[];
extern sensor_reg ov5642_MP5_320x240[];
extern sensor_reg ov5642_MP5_640x480[];
extern sensor_reg ov5642_MP5_1280x960[];
extern sensor_reg ov5642_MP5_1600x1200[];
extern sensor_reg ov5642_MP5_1024x768[];
extern sensor_reg ov5642_MP5_2048x1536[];
extern sensor_reg ov5642_MP5_2592x1944[];
extern sensor_reg ov5642_MP5_dvp_zoom8[];
extern sensor_reg OV5642_MP5_QVGA_YUV422[];
extern sensor_reg OV5642_MP5_JPEG_Capture_QSXGA[];
extern sensor_reg OV5642_MP5_1080P_Video_setting[];
extern sensor_reg OV5642_MP5_720P_Video_setting[];
extern sensor_reg OV5642_MP5_VGA_YUV[];
#endif
