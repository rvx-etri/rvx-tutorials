#ifndef OV2640_REGS_H
#define OV2640_REGS_H

//#include "types.h"

//#include "ArduCAM.h"
//#include <avr/pgmspace.h>

#define OV2640_SENSOR_ADDR (0x60) 
#define OV2640_CHIPID_HIGH 	0x0A
#define OV2640_CHIPID_LOW 	0x0B


typedef struct {
        unsigned short reg;
        unsigned short val;
} sensor_reg;


typedef enum {
        OV2640_RAW_QVGA    = 0,
        OV2640_RAW_SVGA    = 1,
        OV2640_YUV_QVGA    = 2,
        OV2640_YUV_SVGA    = 3
} eOV2640_SIZE;


extern sensor_reg OV2640_QVGA_YUV422[];
extern sensor_reg OV2640_SVGA_YUV422[];

extern sensor_reg OV2640_JPEG_INIT[];
extern sensor_reg OV2640_YUV422[];
extern sensor_reg OV2640_JPEG[];
extern sensor_reg OV2640_VGA_JPEG[];

#endif
