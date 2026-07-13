#ifndef _H_VIO_DEFINE_H_
#define _H_VIO_DEFINE_H_

#define VIM_BASE        (I_EDGE_VIDEO_SYSTEM_VIDEO_CTRL_BASEADDR)
#define VOM_BASE        (I_EDGE_VIDEO_SYSTEM_VIDEO_CTRL_BASEADDR + 0x10000)

typedef enum {
  VIO_FMT_YUVPLANE = 0,
  VIO_FMT_YUVINTRV = 1,
  VIO_FMT_RGBPLANE = 2,
  VIO_FMT_RGBINTRV = 3
} eVIO_FMT;

typedef struct {
	int hsize;
	int vsize;
	unsigned int stride;

	//unsigned int addr0;
	//unsigned int addr1;
	unsigned int addr;
	unsigned int offset0;
	unsigned int offset1;

	int type;

	int en;
	int endian;

	// sync 
	// H
	unsigned int hss;
	unsigned int hfp;
	unsigned int hbp;
	// V
	unsigned int vss;
	unsigned int vfp;
	unsigned int vbp;

	unsigned int d_pol;
	unsigned int h_pol;
	unsigned int v_pol;
} sVOM;

#endif
