#include "ervp_printf.h"
#include "hdmi_rdma.h"
#include "cam_spi.h"
#include "spi_cam.h"
#include "ov5642_regs.h"

void spi_cam_check_test_reg(void)
{
	int vid, pid;
	int temp;

	printf("SPI CAM CHECK TEST REG!\n");

	// i2c test
	configure_i2c(I2C_INDEX_FOR_SPI_CAM, I2C_FREQ_OF_SPI_CAM, I2C_ENABLE, I2C_INTERRUPT_DISABLE);

	vid = cam_spi_read_i2c(OV5642_CHIPID_HIGH);
	pid = cam_spi_read_i2c(OV5642_CHIPID_LOW);

	printf("vid : %02X\n", vid);
	printf("pid : %02X\n", pid);

	// spi test
	configure_spi(SPI_BASEADDR, SPI_FREQ_OF_SPI_CAM, SPI_MODE_OF_SPI_CAM, PORT_ID_OF_SPI_CAM);

	// Check SPI Bus
	cam_spi_write_spi(regSPICAM_TEST, 0x55);
	temp = cam_spi_read_spi(regSPICAM_TEST);
	printf("temp : %02X", temp);
	if(temp == 0x55) printf("Camera SPI bus is OK.\n");
	else             printf("Camera SPI bus has problem.\n");
}

void spi_cam_test(void)
{
	uint8_t *img;
	int size;

	// SPI & I2C AE±aE­
	printf("cam_spi_init_hw()\n");
	cam_spi_init_hw();

	// Device ¿￢°a E®AI
	printf("cam_spi_setup()\n");
	if(cam_spi_setup() == 0)
		return;

	// Sensor & ArduCam AE±aE­
	printf("cam_spi_config_sensor()\n");
	cam_spi_config_sensor(OV5642_RAW_VGA);
	delay_ms(100);

	img = (uint8_t *)0x20000000; // DRAM base

	//memset(img, 0, 1024*1024); // 128KB clear

	printf("cam_spi_capture_single()\n");
	size = cam_spi_capture_single(img);

	if(size > 2048*2048*3)
		return;

	//print_mem(img, 64);
	//print_mem(img, 640*480);
	printf("\r\n");
	printf("\r\n");
	printf("Image DATA\r\n");
	print_mem(img, size);
}


void spi_cam_disp_test(void)
{
	uint8_t *img;
	int i;
	int size;

	sHRDMA hr;
	int i_width, i_height, i_type;
	int o_width, o_height;

	i_width  =  320; i_height =  240; i_type = HR_COLOR_YCbCr; // 
	o_width  = 1920; o_height = 1080; // hdmi fhd


	// HDMI TX AE±aE­
	hr.addr0 = 0x21000000;
	hr.addr1 = 0x21800000;

	hdmirdma_init_param((sHRDMA *)&hr, o_width, o_height, HR_COLOR_YCbCr);
	hdmirdma_set_param((sHRDMA *)&hr);


	//memset(hr.addr0, 0x00, hr.stride*hr.vsize);
	for(i=0; i<hr.stride*hr.vsize; i+=4) {	REG32(0x21000000 + i) = 0x80808080;	}
	for(i=0; i<hr.stride*hr.vsize; i+=4) {	REG32(0x21800000 + i) = 0x80808080;	}



	// ¼¾¼­ AO·AAI YUV AI °æ¿i
	if(i_type == 1) {
		REG32(HDMIRDMA_EN		) = 0xffffffff;

		// SPI CAM AE±aE­
		img = 0x21000000;
		printf("cam_spi_init_hw()\n");
		cam_spi_init_hw();

		printf("cam_spi_setup()\n");
		if(cam_spi_setup() == 0)
			return;

		printf("cam_spi_config_sensor()\n");
		if     (i_width == 320) 	cam_spi_config_sensor(OV5642_YUV_QVGA);
		else if(i_width == 640) 	cam_spi_config_sensor(OV5642_YUV_VGA);
		delay_ms(100);

		hr.idx = 1;
		hdmirdma_set_idx(hr.idx);

		for(i=0; i<100; i++) {

			if     (hr.idx == 1) img = hr.addr0;
			else if(hr.idx == 0) img = hr.addr1;

			printf("cam_spi_capture_single()\n");
			size = cam_spi_capture_single_yuv(img, i_width, i_height, hr.stride);

			hr.idx = 1 - hr.idx;
			hdmirdma_set_idx(hr.idx);
		}

		for(i=0; i<100; i++) {
			delay_ms(100);
		}

		REG32(HDMIRDMA_EN		) = 0;

	}
	// ¼¾¼­ AO·AAI RAW AI °æ¿i
	else {
		// SPI CAM AE±aE­
		img = 0x20000000;
		printf("cam_spi_init_hw()\n");
		cam_spi_init_hw();

		printf("cam_spi_setup()\n");
		if(cam_spi_setup() == 0)
			return;

		printf("cam_spi_config_sensor()\n");
		cam_spi_config_sensor(OV5642_RAW_VGA);
		delay_ms(100);


		REG32(HDMIRDMA_EN		) = 0xffffffff;

		for(i=0; i<10; i++) {
			printf("cam_spi_capture_single()\n");
			size = cam_spi_capture_single(img);


			printf("bayer2rgb\n");
			//bayer2rgb_bggr_sg(img, hr.addr0, 640, 480, hr.stride);
			bayer2rgb_cbycry_sg(img, hr.addr0, i_width, i_height, hr.stride);

			printf("bayer2rgb done\n");
		}

		printf("HDMIRDMA_ENDIAN	  = %08X\n", REG32(HDMIRDMA_ENDIAN		)); 

		for(i=0; i<100; i++) {
			delay_ms(100);
		}

		REG32(HDMIRDMA_EN		) = 0;
	}


	printf("done\n");

	
}

void ClearBorders(unsigned char *rgb, int sx, int sy, int w)
{
	int i, j;
	// black edges are added with a width w:
	i = 4 * sx * w - 1;
	j = 4 * sx * sy - 1;
	while (i >= 0) {
		rgb[i--] = 0;
		rgb[j--] = 0;
	}

	int low = sx * (w - 1) * 4 - 1 + w * 4;
	i = low + sx * (sy - w * 3 + 1) * 4;
	while (i > low) {
		j = 8 * w;
		while (j > 0) {
			rgb[i--] = 0;
			j--;
		}
		i -= (sx - 3 * w) * 4;
	}
}

void bayer2rgb_bggr(unsigned char *bayer, unsigned char *rgb, int width, int height, int ow, int oh, int stride)
{
	typedef unsigned char uint8_t;

	int bayerStep = width;
	//int rgbStep   = 4*width;
	int rgbStep = stride;

	int blue = -1;
	int start_with_green = 0;


	// ClearBorders
	//ClearBorders(rgb, width, height, 1);
	rgb += rgbStep + 4 + 1;
	height -= 2;
	width -= 2;

	for( ; height--; bayer += bayerStep, rgb += rgbStep) {
		int t0, t1;
		unsigned char *bayerEnd = bayer + width;

		printf("height : %d\n", height);

		if(start_with_green) {
			t0 = (bayer[1] + bayer[bayerStep * 2 + 1] + 1) >> 1;
			t1 = (bayer[bayerStep] + bayer[bayerStep + 2] + 1) >> 1;
			rgb[-blue] = (uint8_t) t0;
			rgb[0] = bayer[bayerStep + 1];
			rgb[blue] = (uint8_t) t1;
			bayer++;
			rgb += 4;
		}

		if(blue > 0) {
			for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 8) {
				t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +	bayer[bayerStep * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[bayerStep] + bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] + 2) >> 2;
				rgb[-1] = (uint8_t) t0;
				rgb[0] = (uint8_t) t1;
				rgb[1] = bayer[bayerStep + 1];

				t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
				t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +	1) >> 1;
				rgb[3] = (uint8_t) t0;
				rgb[4] = bayer[bayerStep + 2];
				rgb[5] = (uint8_t) t1;
			}
		}
		else {
			for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 8) {
				t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] + bayer[bayerStep * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[bayerStep] +	bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] + 2) >> 2;
				rgb[1] = (uint8_t) t0;
				rgb[0] = (uint8_t) t1;
				rgb[-1] = bayer[bayerStep + 1];

				t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
				t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
					1) >> 1;
				rgb[5] = (uint8_t) t0;
				rgb[4] = bayer[bayerStep + 2];
				rgb[3] = (uint8_t) t1;
			}
		}

		if (bayer < bayerEnd) {
			t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] + bayer[bayerStep * 2 + 2] + 2) >> 2;
			t1 = (bayer[1] + bayer[bayerStep] +	bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] + 2) >> 2;
			rgb[-blue] = (uint8_t) t0;
			rgb[0] = (uint8_t) t1;
			rgb[blue] = bayer[bayerStep + 1];
			bayer++;
			rgb += 4;
		}


		bayer -= width;
		rgb -= width * 4;

		blue = -blue;
		start_with_green = !start_with_green;
	}

}


void bayer2rgb_bggr_sg(unsigned char *bayer, unsigned char *rgb, int width, int height, int stride)
{
	// boundayr 2pixelAº A³¸® ¾ECO. ¼Oμμ
	// Bayer Format : BGGR 
	int x, y;
	unsigned char  *pb, *po;
	int t0, t1;


	for(y=2; y<height-2; y++) {


		// even line
		pb = bayer + y*width  + 2;
		po = rgb   + y*stride + 8;
		for(x=2; x<width-2; x+=2) {

			// B
			t0 = (pb[-width-1] + pb[-width+1] + pb[width-1] + pb[width+1] + 2) >> 2;
			t1 = (pb[-width] + pb[-1] + pb[+1] + pb[width] + 2) >> 2;

			po[0] = 0;     // a
			po[1] = pb[0]; // b
			po[2] = t1;    // g
			po[3] = t0;    // r

			pb += 1;

			// G
			t0 = (pb[-width] + pb[width] + 1) >> 1;
			t1 = (pb[-1] + pb[+1] + 1) >> 1;

			po[4] = 0;     // a
			po[5] = t1; // b
			po[6] = pb[0]; // g
			po[7] = t0;    // r

			pb += 1;
			po += 8;
		}

		// even line
		pb = bayer + (y+1)*width  + 2;
		po = rgb   + (y+1)*stride + 8;
		for(x=2; x<width-2; x+=2) {
			// G
			t0 = (pb[-width] + pb[width] + 1) >> 1;
			t1 = (pb[-1] + pb[+1] + 1) >> 1;

			po[0] = 0;     // a
			po[1] = t0; // b
			po[2] = pb[0]; // g
			po[3] = t1;    // r

			pb += 1;

			// R
			t0 = (pb[-width-1] + pb[-width+1] + pb[width-1] + pb[width+1] + 2) >> 2;
			t1 = (pb[-width] + pb[-1] + pb[+1] + pb[width] + 2) >> 2;

			po[4] = 0;     // a
			po[5] = t0;    // b
			po[6] = t1;    // g
			po[7] = pb[0]; // r

			pb += 1;
			po += 8;
		}
	}

}

void bayer2rgb_cbycry_sg(unsigned char *bayer, unsigned char *yuv, int width, int height, int stride)
{
	// boundayr 2pixelAº A³¸® ¾ECO. ¼Oμμ
	// Bayer Format : BGGR 
	int x, y;
	unsigned char  *pb, *po;
	int r0, g0, b0, r1, g1, b1;
	int y0, y1, cb, cr;


	for(y=2; y<height-2; y++) {

		printf("y : %d\r\n", y);

		// even line
		pb = bayer + y*width  + 2;
		po = yuv   + y*stride + 4; // 1pixel 2byte
		for(x=2; x<width-2; x+=2) {

			// B
			r0 = (pb[-width-1] + pb[-width+1] + pb[width-1] + pb[width+1] + 2) >> 2;
			g0 = (pb[-width] + pb[-1] + pb[+1] + pb[width] + 2) >> 2;
			b0 = pb[0];

			pb += 1;

			// G
			r1 = (pb[-width] + pb[width] + 1) >> 1;
			b1 = (pb[-1] + pb[+1] + 1) >> 1;
			g1 = pb[0];

			// CbYCrY
			y0 = (77*r0 + 150*g0 + 28*b0) >> 8; if(y0<0) y0=0; else if(y0>255) y0 = 255;
			y1 = (77*r1 + 150*g1 + 28*b1) >> 8; if(y1<0) y1=0; else if(y1>255) y1 = 255;
			cb = ((73*(b0 - y0 + b1 - y1))>>8) + 128; if(cb<0) cb=0; else if(cb>255) cb = 255;
			cr = ((91*(r0 - y0 + r1 - y1))>>8) + 128; if(cr<0) cr=0; else if(cr>255) cr = 255;

			po[0] = cb;
			po[1] = y0;
			po[2] = cr;
			po[3] = y1;


			pb += 1;
			po += 4;
		}

		// even line
		pb = bayer + (y+1)*width  + 2;
		po = yuv   + (y+1)*stride + 4;
		for(x=2; x<width-2; x+=2) {
			// G
			b0 = (pb[-width] + pb[width] + 1) >> 1;
			r0 = (pb[-1] + pb[+1] + 1) >> 1;
			g0 = pb[0];

			pb += 1;

			// R
			b1 = (pb[-width-1] + pb[-width+1] + pb[width-1] + pb[width+1] + 2) >> 2;
			g1 = (pb[-width] + pb[-1] + pb[+1] + pb[width] + 2) >> 2;
			r1 = pb[0];

			y0 = (77*r0 + 150*g0 + 28*b0) >> 8; if(y0<0) y0=0; else if(y0>255) y0 = 255;
			y1 = (77*r1 + 150*g1 + 28*b1) >> 8; if(y1<0) y1=0; else if(y1>255) y1 = 255;
			cb = ((73*(b0 - y0 + b1 - y1))>>8) + 128; if(cb<0) cb=0; else if(cb>255) cb = 255;
			cr = ((91*(r0 - y0 + r1 - y1))>>8) + 128; if(cr<0) cr=0; else if(cr>255) cr = 255;

			po[0] = cb;
			po[1] = y0;
			po[2] = cr;
			po[3] = y1;

			pb += 1;
			po += 4;
		}
	}

}

void print_mem(void *addr, int size)
{
	int i;

#if (0)
	for(i=0; i<size; i++) {
		if((i%16)==0) printf("%08X : ", addr+i);
		printf("%02X ", REG8(addr+i));
		if((i%16)==15) printf("\n");
	}
	printf("\n");
#else
	for(i=0; i<size; i++) {
		//if((i%16)==0) printf("%08X : ", addr+i);
		printf("%02X ", REG8(addr+i));
		if((i%16)==15) printf("\r\n");
	}
	printf("\r\n");
#endif
}
