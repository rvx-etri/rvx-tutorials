#ifndef OV5640_MP5_REGS_H
#define OV5640_MP5_REGS_H
//#include "ArduCAM.h"
//#include <avr/pgmspace.h>

#define OV5640_MP5_SENSOR_ADDR (0x3C) 
//#define OV5642_MP5_SENSOR_ADDR (0x78) 
#define OV5640_MP5_CHIPID_HIGH 0x300a
#define OV5640_MP5_CHIPID_LOW 0x300b

typedef struct {
        unsigned short reg;
        unsigned short val;
} sensor_reg;

extern sensor_reg OV5640_Auto_Focus[];
extern sensor_reg OV5640YUV_Sensor_Dvp_Init[];
extern sensor_reg OV5640_RGB_QVGA[];
extern sensor_reg OV5640_JPEG_QSXGA[];
extern sensor_reg OV5640_5MP_JPEG[];
extern sensor_reg OV5640_QSXGA2QVGA[];
extern sensor_reg OV5640_QSXGA2VGA[];
extern sensor_reg OV5640_QSXGA2WVGA[];
extern sensor_reg OV5640_QSXGA2CIF[];
extern sensor_reg OV5640_QSXGA2SXGA[];
extern sensor_reg OV5640_QSXGA2QXGA[];
extern sensor_reg OV5640_QSXGA2UXGA[];
extern sensor_reg OV5640_QSXGA2XGA[];
extern sensor_reg OV5640_VGA_Preview[];


#endif
