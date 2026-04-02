#include <stddef.h>

#include "ervp_printf.h"
#include "ervp_delay.h"
#include "arducam_driver.h"
#include "arducam.h"

int arducam_format = 0;
int arducam_width = 0;
int arducam_height = 0;
int arducam_stride = 0;

static SpiConfig arducam_spi_config = {SPI_DIVSOR(ARDUCAM_SPI_FREQ), ARDUCAM_SPI_MODE, (1<<ARDUCAM_SPI_PORT_ID), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};

void arducam_config_hw(void)
{
	printf_function();
	// I2C Init
	configure_i2c(ARDUCAM_I2C_INDEX, ARDUCAM_I2C_FREQ, I2C_ENABLE, I2C_INTERRUPT_DISABLE);

	// SPI Init
	spi_configure(&arducam_spi_config);
}

void arducam_config_spi()
{
	spi_configure(&arducam_spi_config);
}

ErvpImage* arducam_alloc_image(ErvpImage* prepared)
{
  ErvpImage* image = prepared;
  if(image==NULL)
    image = image_alloc(arducam_width, arducam_height, arducam_format);
  else
    image_init(image, arducam_width, arducam_height, arducam_format, 0);
  return image;
}

int arducam_check(void)
{
	printf_function();
	uint8_t temp, vid, pid;

	// Check SPI Bus
	arducam_spi_write_abyte(regSPICAM_TEST, 0x55);
	temp = arducam_spi_read_abyte(regSPICAM_TEST);
	if(temp != 0x55)	{ printf("Camera SPI bus has problem.\n"); return 0; }
	//else				{ printf("Camera SPI bus is OK.\n"); }

	// Change MCU mode
	arducam_spi_write_abyte(regSPICAM_MODE, 0x00);

	// Check if the camera module is ov5642
	vid = arducam_i2c_read(OV5642_CHIPID_HIGH);
	pid = arducam_i2c_read(OV5642_CHIPID_LOW);

	if ((vid != 0x56) || (pid != 0x42)) { printf("Can't find OV5642 module!\n"); return 0; }
	//else		                        { printf("Find OV5642 module!\n"); }

	return 1;
}

void arducam_config_sensor(ArducamFormatSize fs)
{
	printf_function();
	arducam_i2c_write(0x3008, 0x80); // sensor reset
	delay_ms(100);

	if(fs == ARDUCAM_YUV_422_QVGA) {
		arducam_write_sensor_vals(OV5642_QVGA_YUV);
		//arducam_write_sensor_vals(OV5642_QVGA_Preview);
		//arducam_i2c_write(0x4300, 0x5F);
		arducam_format = IMAGE_FMT_YUV_422_PACKED;
		arducam_width = 320;
		arducam_height = 240;
		arducam_stride = arducam_width * 2;
	}
	else if(fs == ARDUCAM_YUV_422_VGA) {
		arducam_write_sensor_vals(OV5642_VGA_YUV);
		//arducam_write_sensor_vals(OV5642_QVGA_YUV);
		arducam_write_sensor_vals(OV5642_QCIF);
		arducam_format = IMAGE_FMT_YUV_422_PACKED;
		arducam_width = 176;
		arducam_height = 144;
		arducam_stride = arducam_width * 2;
	}
	else if(fs == ARDUCAM_RGB_565_QVGA) {
		arducam_write_sensor_vals(OV5642_QVGA_YUV);
		arducam_write_sensor_vals(OV5642_RGB_565);
		arducam_format = IMAGE_FMT_RGB_565_PACKED;
		arducam_width = 320;
		arducam_height = 240;
		arducam_stride = arducam_width * 2;
	}
	else if(fs == ARDUCAM_RGB_565_VGA) {
		arducam_write_sensor_vals(OV5642_VGA_YUV);
		arducam_write_sensor_vals(OV5642_RGB_565);
		arducam_format = IMAGE_FMT_RGB_565_PACKED;
		arducam_width = 640;
		arducam_height = 480;
		arducam_stride = arducam_width * 2;
	}
	else if(fs == ARDUCAM_RGB_565_SVGA) {
		arducam_write_sensor_vals(OV5642_VGA_YUV);
		arducam_write_sensor_vals(OV5642_SVGA);
		arducam_write_sensor_vals(OV5642_RGB_565);
		arducam_format = IMAGE_FMT_RGB_565_PACKED;
		arducam_width = 800;
		arducam_height = 600;
		arducam_stride = arducam_width * 2;
	}
	else
		assert_must(0);
#if 0
	else if(fs == ARDUCAM_RGB_888_QVGA) {
		arducam_write_sensor_vals(OV5642_QVGA_YUV);
		arducam_write_sensor_vals(OV5642_RGB_565);
		arducam_i2c_write(0x4300, 0x23);
		arducam_format = IMAGE_FMT_RGB_888;
		arducam_width = 320;
		arducam_height = 240;
		arducam_stride = arducam_width * 3;
	}
	else if(fs == ARDUCAM_RGB_888_VGA) {
		arducam_write_sensor_vals(OV5642_VGA_YUV);
		arducam_write_sensor_vals(OV5642_RGB_565);
		arducam_i2c_write(0x4300, 0x23);
		arducam_format = IMAGE_FMT_RGB_888;
		arducam_width = 640;
		arducam_height = 480;
		arducam_stride = arducam_width * 3;
	}
	else if(fs == ARDUCAM_YUV_444_QVGA) {
		arducam_write_sensor_vals(OV5642_QVGA_YUV);
		arducam_i2c_write(0x4300, 0x20);
		arducam_format = IMAGE_FMT_YUV_444;
		arducam_width = 320;
		arducam_height = 240;
		arducam_stride = arducam_width * 3;
	}
	else if(fs == ARDUCAM_YUV_444_VGA) {
		arducam_write_sensor_vals(OV5642_VGA_YUV);
		arducam_i2c_write(0x4300, 0x20);
		arducam_format = IMAGE_FMT_YUV_444;
		arducam_width = 640;
		arducam_height = 480;
		arducam_stride = arducam_width * 3;
	}
#endif
	delay_ms(100);

	arducam_spi_write_abyte(regSPICAM_SENSOR_TIMING, 0x02); // Vsync : active High, Hsync : active Low
}

void arducam_init(ArducamFormatSize fs)
{
	// SPI & I2C config 
	arducam_config_hw();

	// Device check 
	if(arducam_check() == 0)
		return;

	// Sensor & ArduCam Config
	arducam_config_sensor(fs);

	arducam_set_mirror_flip(ARDUCAM_MIRROR_FLIP);
	delay_ms(100);
}

void arducam_set_mirror_flip(uint8_t mirror_flip)
{
	printf_function();
	uint8_t reg_val;
	switch(mirror_flip)
	{
		case ARDUCAM_MIRROR:
			reg_val = arducam_i2c_read(0x3818);
			reg_val = reg_val|0x00;
			reg_val = reg_val&0x9F;
			arducam_i2c_write(0x3818 ,reg_val);
			reg_val = arducam_i2c_read(0x3621);
			reg_val = reg_val|0x20;
			arducam_i2c_write(0x3621, reg_val);

			break;
		case ARDUCAM_FLIP:
			reg_val = arducam_i2c_read(0x3818);
			reg_val = reg_val|0x60;
			reg_val = reg_val&0xFF;
			arducam_i2c_write(0x3818 ,reg_val);
			reg_val = arducam_i2c_read(0x3621);
			reg_val = reg_val&0xdf;
			arducam_i2c_write(0x3621, reg_val);
			break;
		case ARDUCAM_MIRROR_FLIP:
			reg_val = arducam_i2c_read(0x3818);
			reg_val = reg_val|0x20;
			reg_val = reg_val&0xbF;
			arducam_i2c_write(0x3818 ,reg_val);
			reg_val = arducam_i2c_read(0x3621);
			reg_val = reg_val|0x20;
			arducam_i2c_write(0x3621, reg_val);
			break;
		case ARDUCAM_NORMAL:
			reg_val = arducam_i2c_read(0x3818);
			reg_val = reg_val|0x40;
			reg_val = reg_val&0xdF;
			arducam_i2c_write(0x3818 ,reg_val);
			reg_val = arducam_i2c_read(0x3621);
			reg_val = reg_val&0xdf;
			arducam_i2c_write(0x3621, reg_val);
			break;
	}
}

uint32_t arducam_read_fifo_size(void)
{
	uint32_t fifo_size, size1, size2, size3;

	size1 = arducam_spi_read_abyte(regSPICAM_FIFOSIZE1);
	size2 = arducam_spi_read_abyte(regSPICAM_FIFOSIZE2);
	size3 = arducam_spi_read_abyte(regSPICAM_FIFOSIZE3);

	fifo_size = ((size3<<16) | (size2<<8) | (size1)) & 0x007FFFFF;

	return fifo_size;
}

void arducam_single_capture(ErvpImage *image)
{
	uint8_t *data;
	uint32_t fifo_size;
	int stride;
	int height;
	uint8_t *line_data;
	int i,j;
	int hindex_next, sindex_next;
	int width_incr, stride_incr, height_incr;
	int temp;

	data = (uint8_t *)image->addr[0];

	// flush the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);
	// clear the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	arducam_spi_write_abyte(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (arducam_spi_read_abyte(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	printf("capture done\n");

	fifo_size = arducam_read_fifo_size();
	//printf("fifo size : %d\n", fifo_size);
	if (fifo_size >= ARDUCAM_MAX_FIFO_SIZE ) 
	{
		printf("Over size : %lu\n", fifo_size);
		arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);
		return;
	}
	if (fifo_size == 0 ) //0 kb
	{
		printf("Size is 0\n");
		arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);
		return;
	}

	stride = image->width<<1;
	if(stride > arducam_stride)
		stride = arducam_stride;
	height = image->height;
	if(height > arducam_height)
		height = arducam_height;

	if(stride==arducam_stride)
	{
		arducam_spi_read_bytes(regSPICAM_BURST_READ, data, stride*height);
	}
	else if((image->width <= arducam_width) && (image->height <= arducam_height))
	{
		width_incr = 1;
		temp = image->width<<1;
		while(temp < arducam_width)
		{
			temp += image->width;
			width_incr++;
		}
		stride_incr = width_incr<<1;
		height_incr = 1;
		temp = image->height<<1;
		while(temp < arducam_height)
		{
			temp += image->height;
			height_incr++;
		}
		//printf("%d\n", stride_incr);
		//printf("%d\n", height_incr);

		for(i=0, hindex_next=0, line_data = data; i<(image->height*height_incr); i++)
		{
			if(i==hindex_next)
			{
				hindex_next += height_incr;
				for(j=0, sindex_next=0; j<(image->width*stride_incr); j+=2)
				{
					if(j==sindex_next)
					{
						sindex_next += stride_incr;
						*line_data++ = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
						*line_data++ = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
					}
					else
					{
						arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
						arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
					}
				}
				for(j=(image->width*stride_incr); j<arducam_stride; j++)
				{
					arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
				}
			}
			else
			{
				for(j=0; j<arducam_stride; j++)
				{
					arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
				}
			}
		}
	}
	else
	{
		for(i=0, line_data = data; i<height; i++, line_data+=(image->width<<1))
		{
			for(j=0; j<stride; j++)
			{
				line_data[j] = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
			}
			for(j=stride; j<arducam_stride; j++)
			{
				arducam_spi_read_abyte(regSPICAM_SINGLE_READ);  // NOT USED
			}
		}
	}

	// clear the fifo done flag
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);
}

#if 0
int arducam_capture_single_rgb(unsigned char *rgb565, int width, int height, int sample)
{
	int x, y;

	int start_row, start_col;
	uint32_t addr;
	int rgb_image_index;
	int rgb_image_row, rgb_image_col;
	uint32_t fifo_size;

	start_col = ((arducam_width - width*sample) >> 1);
	start_row = ((arducam_height - height*sample) >> 1);

	if( !((start_col>=0) && (start_row>=0)) )
	{
		printf("can't sampling image - %d %d\n", start_col, start_row);
		return 0;
	}

	//printf("Number of frames to be captured: %d\n", arducam_spi_read_abyte(regSPICAM_CAPTURE));
	// clear (or flush) the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	arducam_spi_write_abyte(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (arducam_spi_read_abyte(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	printf("capture done\n");

	//fifo_size = arducam_read_fifo_size();
	//printf("fifo_size : %d\n", fifo_size);
	
	addr = regSPICAM_BURST_READ;

	spi_start(); // CS LOW
	spi_write(1, &addr);

	rgb_image_row = 0;
	for(y=0; y<arducam_height; y++) {
		spi_read(arducam_stride, line_data);

		if(y>=start_row)
		{
			if(((y-start_row)%sample)==0)
			{
				rgb_image_col=0;
				rgb_image_index = rgb_image_row*width;
				for(x=start_col; x<arducam_width; x+=sample)
				{
					rgb565[2*rgb_image_index] = line_data[(x<<1)];
					rgb565[2*rgb_image_index+1] = line_data[(x<<1)+1];

					rgb_image_col++;
					rgb_image_index++;
					if(rgb_image_col >= width)
						break;
				}
				rgb_image_row++;
				if(rgb_image_row >= height)
					break;
			}
		}
	}
	//printf("%d %d\n", rgb_image_col, rgb_image_row);

	spi_end(); // CS LOW

	// clear the fifo done flag
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	return 1;
}

int get_yuv(YUV *yuv_image, int width, int height, int sample)
{
	int x, y;
	unsigned short rgb;

	int start_row, start_col;
	uint32_t addr;
	int yuv_image_index;
	int yuv_image_row, yuv_image_col;

	start_col = ((arducam_width - width*sample) >> 1);
	start_row = ((arducam_height - height*sample) >> 1);

	//printf("%d %d\n", start_col, start_row);
	addr = regSPICAM_BURST_READ;

	spi_start(); // CS LOW
	spi_write(1, &addr);

	yuv_image_row = 0;
	for(y=0; y<arducam_height; y++) {
		spi_read(arducam_stride, line_data);

		if(y>=start_row)
		{
			if(((y-start_row)%sample)==0)
			{
				yuv_image_col = 0;
				yuv_image_index = yuv_image_row*width;
				for(x=start_col; x<arducam_width; x+=sample)
				{
					yuv_image[yuv_image_index].Y = line_data[(x<<1) + 1];
					yuv_image[yuv_image_index].U = line_data[(x>>1)<<2];
					yuv_image[yuv_image_index].V = line_data[((x>>1)<<2) + 2];

					yuv_image_col++;
					yuv_image_index++;
					if(yuv_image_col >= width)
						break;
				}
				yuv_image_row++;
				if(yuv_image_row >= height)
					break;
			}
		}
	}
	//printf("%d %d\n", yuv_image_col, yuv_image_row);

	spi_end(); // CS LOW
}


int arducam_capture_single(YUV *img, int width, int height, int sample)
{
	uint32_t fifo_size;
	//unsigned short RGB;

	// clear (or flush) the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	arducam_spi_write_abyte(regSPICAM_FIFO, START_CAPTURE);
	//printf("capture start\n");

	// check done
	while( (arducam_spi_read_abyte(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	//printf("capture done\n");

	fifo_size = arducam_read_fifo_size();

	//printf("fifo_size : %d\n", fifo_size);

	get_yuv(img, width, height, sample);

	// clear the fifo done flag
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	return fifo_size;
}

int arducam_capture_single_yuv(uint8_t *img, int width, int height, int stride)
{
	int x, y;
	uint32_t fifo_size, rsize;
	uint8_t *p;

	//timer_start();

	// clear (or flush) the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	arducam_spi_write_abyte(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (arducam_spi_read_abyte(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	//printf("capture done. %d\n", timer_get_cur_ms());

	fifo_size = arducam_read_fifo_size();

	printf("fifo_size : %d\n", fifo_size);

#if (0)
	// Single Read
	for(y=0; y<height; y++) {
		p = img + y*stride;
		for(x=0; x<width*2; x+=4) {
			p[x+0] = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
			p[x+1] = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
			p[x+2] = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
			p[x+3] = arducam_spi_read_abyte(regSPICAM_SINGLE_READ);
		}
	}
	//printf("read done. %d\n", timer_get_cur_ms());


#else
	{
		// Burst Read
		uint32_t addr;

		addr = regSPICAM_BURST_READ;

		spi_start(); // CS LOW
		spi_write(1, &addr);

		for(y=0; y<height; y++) {
			p = img + y*stride;
			for(x=0; x<width*2; x+=4) {
				spi_read(1, &p[x+0]);
				spi_read(1, &p[x+1]);
				spi_read(1, &p[x+2]);
				spi_read(1, &p[x+3]);
			}
		}

		spi_end(); // CS LOW
		//printf("burst read done. %d\n", timer_get_cur_ms());
	}
#endif

	// clear the fifo done flag
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	return fifo_size;

}

/***** JPEG *****/

void InitCAM(uint8_t m_fmt)
{
	arducam_i2c_write(0x3008, 0x80);
	if (m_fmt == ARDUCAM_RAW){
		//Init and set 640x480;
		arducam_write_sensor_vals(OV5642_1280x960_RAW);	
		arducam_write_sensor_vals(OV5642_640x480_RAW);	
	}else{	
		arducam_write_sensor_vals(OV5642_QVGA_Preview);
		delay_ms(100);
		if (m_fmt == ARDUCAM_JPEG)
		{
			delay_ms(100);
			arducam_write_sensor_vals(OV5642_JPEG_Capture_QSXGA);
			arducam_write_sensor_vals(OV5642_320x240);
			delay_ms(100);
			arducam_i2c_write(0x3818, 0xa8);
			arducam_i2c_write(0x3621, 0x10);
			arducam_i2c_write(0x3801, 0xb0);
			arducam_i2c_write(0x4407, 0x04);
			//arducam_i2c_write(0x4407, 0x0C);
			arducam_i2c_write(0x5888, 0x00);
			arducam_i2c_write(0x5000, 0xFF); 
		}
		else
		{
			uint8_t reg_val;
			arducam_i2c_write(0x4740, 0x21);
			arducam_i2c_write(0x501e, 0x2a);
			arducam_i2c_write(0x5002, 0xf8);
			arducam_i2c_write(0x501f, 0x01);
			arducam_i2c_write(0x4300, 0x61);
			reg_val = arducam_i2c_read(0x3818);
			arducam_i2c_write(0x3818, (reg_val | 0x60) & 0xff);
			reg_val = arducam_i2c_read(0x3621);
			arducam_i2c_write(0x3621, reg_val & 0xdf);
		}
	}
}

void arducam_set_jpeg_size(uint8_t size)
{
	uint8_t reg_val;

	switch (size)
	{
		case ARDUCAM_320x240:
			arducam_write_sensor_vals(OV5642_320x240);
			break;
		case ARDUCAM_640x480:
			arducam_write_sensor_vals(OV5642_640x480);
			break;
		case ARDUCAM_1024x768:
			arducam_write_sensor_vals(OV5642_1024x768);
			break;
		case ARDUCAM_1280x960:
			arducam_write_sensor_vals(OV5642_1280x960);
			break;
		case ARDUCAM_1600x1200:
			arducam_write_sensor_vals(OV5642_1600x1200);
			break;
		case ARDUCAM_2048x1536:
			arducam_write_sensor_vals(OV5642_2048x1536);
			break;
		case ARDUCAM_2592x1944:
			arducam_write_sensor_vals(OV5642_2592x1944);
			break;
		default:
			arducam_write_sensor_vals(OV5642_320x240);
			break;
	}
}

uint32_t get_jpeg(unsigned char *jpeg)
{
	int x, y;

	uint32_t addr;
	int fifo_size;
	uint8_t temp = 0, temp_last = 0;
	int i=0;
	uint8_t is_header = 0;

	//VSYNC is active HIGH   
	arducam_spi_write_abyte(regSPICAM_SENSOR_TIMING, VSYNC_LEVEL_MASK);

	//printf("Number of frames to be captured: %d\n", arducam_spi_read_abyte(regSPICAM_CAPTURE));
	
	// clear (or flush) the fifo
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	arducam_spi_write_abyte(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (arducam_spi_read_abyte(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	printf("capture done\n");

	fifo_size = arducam_read_fifo_size();
	if(fifo_size >= ARDUCAM_MAX_FIFO_SIZE)
	{
		printf("Over size.");
		return -1;
	}else if (fifo_size == 0 ){
		printf("Size is 0.");
		return 0;
	} 
	printf("fifo_size : %d\n", fifo_size);
	
	addr = regSPICAM_BURST_READ;


	spi_start(); // CS LOW
	spi_write(1, &addr);
	//spi_read(fifo_size, jpeg);

	while ( fifo_size-- )
	{
		temp_last = temp;
		spi_read(1, &temp);
		//printf("0x%02x ", temp);
			//printf("fifo_size : %d\n", fifo_size);

		//Read JPEG data from FIFO
		if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		{
			jpeg[i++] = temp;  //save the last  0XD9     
			//Write the remain bytes in the buffer
			spi_end(); // CS HIGH
			printf("fifo_size : %d\n", fifo_size);
			printf("this is end: %d\n", i);
			printf("IMG save OK !\n"); 
			break;
			is_header = 0;
			i = 0;
		}  
		if (is_header == 1)
		{ 
			//Write image data to buffer if not full
			//if (i < BUF_SIZE)
			jpeg[i++] = temp;
			//	else
			//	{
			//		//Write BUF_SIZE bytes image data to file
			//		spi_end(); // CS HIGH
			//		i = 0;
			//		jpeg[i++] = temp;
			//		spi_start(); // CS LOW
			//		spi_write(1, &addr);
			//	}        
		}
		else if ((temp == 0xD8) & (temp_last == 0xFF))
		{
			printf("fifo_size : %d\n", fifo_size);
			printf("this is header: %d\n", i);
			is_header = 1;
			jpeg[i++] = temp_last;
			jpeg[i++] = temp;   
		} 
	}

	// clear the fifo done flag
	arducam_spi_write_abyte(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);
	printf("fifo_size : %d\n", fifo_size);

	return fifo_size;
}
#endif

void arducam_set_test_color_bar()
{
	arducam_i2c_write(0x503D, 0x80);
}
