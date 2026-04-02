#ifndef __OV5642_REGS_H__
#define __OV5642_REGS_H__

#define OV5642_SENSOR_ADDR  	(0x3C)
#define OV5642_CHIPID_HIGH		(0x300A)
#define OV5642_CHIPID_LOW		(0x300B)

typedef struct {
    unsigned short reg;
    unsigned short val;
} sensor_reg;

extern const sensor_reg OV5642_VGA_preview_setting[];
extern const sensor_reg OV5642_RAW[];
extern const sensor_reg OV5642_1280x960_RAW[];
extern const sensor_reg OV5642_1920x1080_RAW[];
extern const sensor_reg OV5642_640x480_RAW[];
extern const sensor_reg OV5642_QVGA_YUV[]; 
extern const sensor_reg OV5642_VGA_YUV[]; 
extern const sensor_reg OV5642_RGB_565[]; 
extern const sensor_reg OV5642_QSXGA_JPEG[];

extern const sensor_reg OV5642_SVGA[];	//800x600
extern const sensor_reg OV5642_QSXGA[];	//2560x2048
extern const sensor_reg OV5642_SXGA[];	//1280x1024
extern const sensor_reg OV5642_QXGA[];	//2048x1536
extern const sensor_reg OV5642_UXGA[];	//1600x1200
extern const sensor_reg OV5642_CIF[];		//352x288
extern const sensor_reg OV5642_QCIF[];	//176x144

extern const sensor_reg OV5642_320x240[];
extern const sensor_reg OV5642_640x480[];
extern const sensor_reg OV5642_1280x960[];
extern const sensor_reg OV5642_1600x1200[];
extern const sensor_reg OV5642_1024x768[];
extern const sensor_reg OV5642_2048x1536[];
extern const sensor_reg OV5642_2592x1944[];
extern const sensor_reg OV5642_QVGA_Preview[];
extern const sensor_reg OV5642_JPEG_Capture_QSXGA[];

#endif // header
