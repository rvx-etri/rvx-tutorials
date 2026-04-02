#include "ov5642_regs.h"
#include "cam_spi.h"

void timer_start(void);
int timer_get_cur_ms(void);

static int time_sec;
static int time_us; 

void cam_spi_init_hw(void)
{
	// I2C Init
	configure_i2c(I2C_INDEX_FOR_SPI_CAM, I2C_FREQ_OF_SPI_CAM, I2C_ENABLE, I2C_INTERRUPT_DISABLE);

	// SPI Init
	configure_spi(SPI_BASEADDR, SPI_FREQ_OF_SPI_CAM, SPI_MODE_OF_SPI_CAM, PORT_ID_OF_SPI_CAM);
}

int cam_spi_setup(void)
{
	uint8_t temp, vid, pid;

	// Check SPI Bus
	cam_spi_write_spi(regSPICAM_TEST, 0x55);
	temp = cam_spi_read_spi(regSPICAM_TEST);
	if(temp == 0x55) { printf("Camera SPI bus is OK.\n"); }
	else             { printf("Camera SPI bus has problem.\n"); return 0; }

	// Change MCU mode
	cam_spi_write_spi(regSPICAM_MODE, 0x00);

	// Check if the camera module is ov5642
	vid = cam_spi_read_i2c(OV5642_CHIPID_HIGH);
	pid = cam_spi_read_i2c(OV5642_CHIPID_LOW);

	if ((vid != 0x56) || (pid != 0x42)) { printf("Can't find OV5642 module!\n"); return 0; }
	else		                        { printf("Find OV5642 module!\n"); }

	return 1;
}

void cam_spi_config_sensor(eOV5642_SIZE size)
{
	cam_spi_write_i2c(0x3008, 0x80); // sensor reset
	delay_ms(100);

	if(size == OV5642_RAW_VGA) {
		cam_spi_write_sensor_vals(OV5642_1280x960_RAW);
		cam_spi_write_sensor_vals(OV5642_640x480_RAW);
	}
	else if(size == OV5642_RAW_HD) {
		cam_spi_write_sensor_vals(OV5642_1280x960_RAW);
	}
	else if(size == OV5642_RAW_FHD) {
		cam_spi_write_sensor_vals(ov5642_RAW);
		cam_spi_write_sensor_vals(OV5642_1920x1080_RAW);
	}
	else if(size == OV5642_YUV_QVGA) {
		cam_spi_write_sensor_vals(OV5642_QVGA_YUV);
	}
	else if(size == OV5642_YUV_VGA) {
		cam_spi_write_sensor_vals(OV5642_VGA_YUV);
	}

	delay_ms(100);

	//cam_spi_write_spi(SENSOR_INTERFACE_TIMING_REG_ADDR, SENSOR_VSYNC_POLARITY);
	cam_spi_write_spi(regSPICAM_SENSOR_TIMING, 0x02); // Vsync : active High, Hsync : active Low

}

void cam_spi_config_sensor_size(int width)
{
	switch(width) {
	case 320: // 320 x 240
		break;
	case 640: // 640 x 480
		break;
	case 1920: // 1920 x 1080
		break;
	case 2048: // 2048 x 1536
		break;
	case 2592: // 2592 x 1944
		break;
	}
}


int cam_spi_capture_single(uint8_t *img)
{
	int i;
	uint32_t fifo_size;

	// clear (or flush) the fifo
	cam_spi_write_spi(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	cam_spi_write_spi(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (cam_spi_read_spi(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	printf("capture done\n");

	fifo_size = cam_spi_read_fifo_size();

	printf("fifo_size : %d\n", fifo_size);
	
#if (1)
	// Single Read
	for(i=0; i<fifo_size; i++) {
		img[i] = cam_spi_read_spi(regSPICAM_SINGLE_READ);
	}


#else
	// Burst Read
	// CS LOW

	cam_spi_write_spi(regSPICAM_BURST_READ);
	cam_spi_read_spi(0x00); // dummy read
	
	// 4096 bytes read
	// residual bytes read

	// CS High
#endif

	// clear the fifo done flag
	cam_spi_write_spi(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	return fifo_size;

}

int cam_spi_capture_single_yuv(uint8_t *img, int width, int height, int stride)
{
	int i, x, y;
	uint32_t fifo_size, rsize;
	uint8_t *p;

	timer_start();

	// clear (or flush) the fifo
	cam_spi_write_spi(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	// start the capture
	cam_spi_write_spi(regSPICAM_FIFO, START_CAPTURE);
	printf("capture start\n");

	// check done
	while( (cam_spi_read_spi(regSPICAM_STATUS) & CAMERA_WRITE_FIFO_DONE_FLAG) == 0) {
		delay_ms(1);
	}
	printf("capture done. %d\n", timer_get_cur_ms());

	fifo_size = cam_spi_read_fifo_size();

	printf("fifo_size : %d\n", fifo_size);

#if (0)
	// Single Read
	for(y=0; y<height; y++) {
		p = img + y*stride;
		for(x=0; x<width*2; x+=4) {
			p[x+0] = cam_spi_read_spi(regSPICAM_SINGLE_READ);
			p[x+1] = cam_spi_read_spi(regSPICAM_SINGLE_READ);
			p[x+2] = cam_spi_read_spi(regSPICAM_SINGLE_READ);
			p[x+3] = cam_spi_read_spi(regSPICAM_SINGLE_READ);
		}
	}
	printf("read done. %d\n", timer_get_cur_ms());


#else
	{
		// Burst Read
		uint32_t addr;

		addr = regSPICAM_BURST_READ;

		enable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM); // CS LOW
		__write_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &addr);

		for(y=0; y<height; y++) {
			p = img + y*stride;
			for(x=0; x<width*2; x+=4) {
				__read_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &p[x+0]);
				__read_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &p[x+1]);
				__read_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &p[x+2]);
				__read_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &p[x+3]);
			}
		}

		disable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM); // CS LOW
		printf("burst read done. %d\n", timer_get_cur_ms());
	}
#endif

	// clear the fifo done flag
	cam_spi_write_spi(regSPICAM_FIFO, CLEAR_WRITE_FIFO_DONE_FLAG);

	return fifo_size;

}




uint32_t cam_spi_read_fifo_size(void)
{
	uint32_t fifo_size, size1, size2, size3;

	size1 = cam_spi_read_spi(regSPICAM_FIFOSIZE1);
	size2 = cam_spi_read_spi(regSPICAM_FIFOSIZE2);
	size3 = cam_spi_read_spi(regSPICAM_FIFOSIZE3) & 0x07;

	fifo_size = ((size3<<16) | (size2<<8) | (size1)) & 0x07FFFF;

	return fifo_size;
}


// I/O Interfacing
void cam_spi_write_spi(uint8_t addr, uint8_t data)
{
	addr |= (1<<7); // write command

	enable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM);
	__write_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &addr);
	__write_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &data);
	disable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM);

}

uint8_t cam_spi_read_spi(uint8_t addr)
{
	uint8_t data;

	addr &= ~(1<<7);

	enable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM);
	__write_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &addr);
	__read_spi(SPI_BASEADDR, SPI_MODE_OF_SPI_CAM, 1, &data);
	disable_spi(SPI_BASEADDR, PORT_ID_OF_SPI_CAM);

	return data;
}

void cam_spi_write_i2c(uint16_t addr, uint8_t data)
{
	start_i2c_transmisstion(I2C_INDEX_FOR_SPI_CAM, OV5642_SENSOR_ADDR);
	write_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM, addr>>8);
	write_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM, addr&0xff);
	// writing
	finish_writing_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM, data);
}

void cam_spi_write_sensor_vals(sensor_reg reglist[])
{
	uint32_t addr;
	uint8_t data;
	sensor_reg *next = reglist;

	while((addr != 0xffff) | (data != 0xff)) {
		addr = next->reg;
		data = next->val;
		cam_spi_write_i2c(addr, data);
		next++;
	}
}

uint8_t cam_spi_read_i2c(uint16_t addr)
{
	uint8_t data;

	start_i2c_transmisstion(I2C_INDEX_FOR_SPI_CAM, OV5642_SENSOR_ADDR);
	write_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM, addr>>8);
	write_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM, addr&0xff);

	start_i2c_reading(I2C_INDEX_FOR_SPI_CAM, OV5642_SENSOR_ADDR);

	// writing
	data = finish_reading_a_byte_on_i2c(I2C_INDEX_FOR_SPI_CAM);

	return data;
}

void timer_start(void)
{
	time_sec = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1SEC);
	time_us  = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1US);
}

int timer_get_cur_ms(void)
{
	int sec, us, ms;

	sec = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1SEC);
	us  = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1US);

	sec = sec - time_sec;
	us  = us - time_us;

	if(us < 0) {
		us  += 1000000;
		sec -= 1;
	}

	ms = sec*1000 + us/1000;

	// 100MHz º¸Á¤
	ms = ms;

	return ms;
}

int timer_get_cur_us(void)
{
	int sec, us;

	sec = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1SEC);
	us  = REG32(EXTERNAL_PERI_GROUP_BASEADDR + MMAP_OFFSET_PLATFORM_MISC_CLOCK_1US);

	sec = sec - time_sec;
	us  = us - time_us;

	if(us < 0) {
		us  += 1000000;
		sec -= 1;
	}

	us = sec*1000000 + us;

	// 100MHz º¸Á¤
	us = us;

	return us;
}
