#ifndef _H_MT9P111_H_
#define _H_MT9P111_H_

// https://github.com/wendal/rk2918_uzone_f0_top/blob/master/drivers/media/video/mt9p111.h

//#define MT9P111_ID	(0x78>>1)
#define MT9P111_ID	(0x7A>>1)

#define WORD_LEN             0x04
#define BYTE_LEN             0x02

#define SEQUENCE_INIT        0x00
#define SEQUENCE_NORMAL      0x01
#define SEQUENCE_CAPTURE     0x02
#define SEQUENCE_PREVIEW     0x03

#define SEQUENCE_PROPERTY    0xFFFC
#define SEQUENCE_WAIT_MS     0xFFFD
#define SEQUENCE_WAIT_US     0xFFFE
#define SEQUENCE_END	     0xFFFF

/*configure register for flipe and mirror during initial*/
#define CONFIG_SENSOR_FLIPE     0
#define CONFIG_SENSOR_MIRROR    0
#define CONFIG_SENSOR_MIRROR_AND_FLIPE  0
#define CONFIG_SENSOR_NONE_FLIP_MIRROR  1
/**adjust part parameter to solve bug******/

#define ADJUST_FOR_720P_FALG      1
#define ADJUST_FOR_VGA_FALG       1
#define ADJUST_FOR_CAPTURE_FALG   1
#define ADJUST_PCLK_FRE_FALG      1

/*
#define ADJUST_FOR_720P_FALG      0
#define ADJUST_FOR_VGA_FALG       1
#define ADJUST_FOR_CAPTURE_FALG   0
#define ADJUST_PCLK_FRE_FALG      0
*/

/**optimize code to shoten open time******/
#define ADJUST_OPTIMIZE_TIME_FALG      1





////
/* Sensor Driver Configuration */
#define SENSOR_NAME RK29_CAM_SENSOR_MT9P111
#define SENSOR_V4L2_IDENT V4L2_IDENT_MT9P111
#define SENSOR_ID SEQUENCE_END
#define SENSOR_ID_REG SEQUENCE_END
#define SENSOR_RESET_REG 0x0010
#define SENSOR_RESET_VAL 0x0115
#define SENSOR_RESET_REG_LEN  WORD_LEN
#define SENSOR_MIN_WIDTH    176
#define SENSOR_MIN_HEIGHT   144
#define SENSOR_MAX_WIDTH    2592
#define SENSOR_MAX_HEIGHT   1944
#define SENSOR_INIT_WIDTH	640			/* Sensor pixel size for sensor_init_data array */
#define SENSOR_INIT_HEIGHT  480
#define SENSOR_INIT_WINSEQADR sensor_vga
#define SENSOR_INIT_PIXFMT V4L2_PIX_FMT_UYVY
#define YUV420_BUFFER_MAX_SIZE  7558272     /* 2592*1944*1.5*/ 

#define CONFIG_SENSOR_WhiteBalance	1
#define CONFIG_SENSOR_Brightness	0
#define CONFIG_SENSOR_Contrast      0
#define CONFIG_SENSOR_Saturation    0
#define CONFIG_SENSOR_Effect        1
#define CONFIG_SENSOR_Scene         1
#define CONFIG_SENSOR_DigitalZoom   0
#define CONFIG_SENSOR_Exposure      0
#define CONFIG_SENSOR_Flash         1
#define CONFIG_SENSOR_Mirror        0
#define CONFIG_SENSOR_Flip          1
#define CONFIG_SENSOR_Focus         1



#define CONFIG_SENSOR_I2C_SPEED     100000       /* Hz */
//#define CONFIG_SENSOR_I2C_SPEED   350000       /* Hz */

/* Sensor write register continues by preempt_disable/preempt_enable for current process not be scheduled */
#define CONFIG_SENSOR_I2C_NOSCHED   0
#define CONFIG_SENSOR_I2C_RDWRCHK   0


#define SENSOR_BUS_PARAM  (SOCAM_MASTER | SOCAM_PCLK_SAMPLE_RISING|\
	SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH|\
	SOCAM_DATA_ACTIVE_HIGH | SOCAM_DATAWIDTH_8  |SOCAM_MCLK_24MHZ)

#define COLOR_TEMPERATURE_CLOUDY_DN    6500
#define COLOR_TEMPERATURE_CLOUDY_UP    8000
#define COLOR_TEMPERATURE_CLEARDAY_DN  5000
#define COLOR_TEMPERATURE_CLEARDAY_UP    6500
#define COLOR_TEMPERATURE_OFFICE_DN     3500
#define COLOR_TEMPERATURE_OFFICE_UP     5000
#define COLOR_TEMPERATURE_HOME_DN       2500
#define COLOR_TEMPERATURE_HOME_UP       3500

#define SENSOR_NAME_STRING(a) STR(CONS(SENSOR_NAME, a))
#define SENSOR_NAME_VARFUN(a) CONS(SENSOR_NAME, a)

#define SENSOR_AF_IS_ERR    (0x00<<0)
#define SENSOR_AF_IS_OK		(0x01<<0)
#define SENSOR_INIT_IS_ERR   (0x00<<28)
#define SENSOR_INIT_IS_OK    (0x01<<28)



#if CONFIG_SENSOR_Focus
#define SENSOR_AF_MODE_INFINITY    0
#define SENSOR_AF_MODE_MACRO       1
#define SENSOR_AF_MODE_FIXED       2
#define SENSOR_AF_MODE_AUTO        3
#define SENSOR_AF_MODE_CONTINUOUS  4
#define SENSOR_AF_MODE_CLOSE       5
#endif



void mt9p111_init_sg(void);

#endif
