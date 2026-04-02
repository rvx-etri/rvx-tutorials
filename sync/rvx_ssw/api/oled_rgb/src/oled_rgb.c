#include "oled_rgb.h"

#include "platform_info.h"
#include "ervp_gpio.h"
#include "ervp_printf.h"
#include "ervp_delay.h"
#include "ervp_external_peri_group_memorymap.h"
#include "frvp_spi.h"
#include "ervp_assert.h"

//#define SPI_FREQ_OF_OLED        5000000
#define SPI_FREQ_OF_OLED        10000000
#define SPI_MODE_OF_OLED        SPI_SCKMODE_3

static const SpiConfig oled_spi_config = {SPI_DIVSOR(SPI_FREQ_OF_OLED), SPI_MODE_OF_OLED, (1<<SPI_INDEX_FOR_OLED), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};

static unsigned char cmds[13];
static uint16_t oled_buffer[OLEDRGB_WIDTH*OLEDRGB_HEIGHT];

unsigned short BuildRGB(unsigned char R,unsigned char G,unsigned char B){return ((R>>3)<<11) | ((G>>2)<<5) | (B>>3);};     
unsigned char ExtractRFromRGB(unsigned short wRGB){return (unsigned char)((wRGB>>11)&0x1F);};
unsigned char ExtractGFromRGB(unsigned short wRGB){return (unsigned char)((wRGB>>5)&0x3F);};
unsigned char ExtractBFromRGB(unsigned short wRGB){return (unsigned char)(wRGB&0x1F);};

void OledrgbDevInit();
void OledrgbHostTerm();
void OledrgbHostInit();

void write_cmd1(unsigned char bVal);
void write_cmd2(unsigned char bVal1, unsigned char bVal2);
void write_cmd4(unsigned char *pCmd, int nCmd, const unsigned char *pData, int nData);
void write_bitmap(unsigned char *pCmd, int nCmd, const unsigned char *pData, int nData);

void oled_rgb_start(void)
{
	OledrgbHostInit();
	OledrgbDevInit();
}

void oled_rgb_end()
{
	OledrgbHostTerm();
}

void oled_rgb_config_spi()
{
	spi_configure(&oled_spi_config);
}

static inline void OledReset()
{
	gpio_set_output(MMAP_SPIO_OLED_RSTNN, 1);
	gpio_set_output(MMAP_SPIO_OLED_RSTNN, 0);
	delay_ms(1);
	gpio_set_output(MMAP_SPIO_OLED_RSTNN, 1);
	delay_ms(1);
}

static inline void OledPowerOn()
{
	gpio_set_output(MMAP_SPIO_OLED_VDD, 1);
	delay_ms(20);
}

void OledrgbHostInit()
{
	oled_rgb_config_spi();
	gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_COMMAND);
	OledReset();
	gpio_set_output(MMAP_SPIO_OLED_VBAT, 0);
	OledPowerOn();
}

void OledrgbHostTerm()
{

	// Make the signal pins be inputs.
	gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_DATA);
	OledReset();

	// Make power control pins be inputs. 
	gpio_set_output(MMAP_SPIO_OLED_VBAT, 0);
	OledPowerOn();
}

void OledrgbDevInit()
{	
	/* 
		 Bring PmodEn HIGH
		 */

	OledPowerOn();	
	OledReset();

	/* command un-lock
	*/
	write_cmd1(0xFD);
	write_cmd1(0x12);

	/* 5. Univision Initialization Steps
	*/

	// 5a) Set Display Off
	write_cmd1(CMD_DISPLAYOFF);

	// 5b) Set Remap and Data Format
	write_cmd2(CMD_SETREMAP, 0x72);

	// 5c) Set Display Start Line
	write_cmd2(CMD_SETDISPLAYSTARTLINE, 0x00);

	// 5d) Set Display Offset
	write_cmd2(CMD_SETDISPLAYOFFSET, 0x00); //no offset

	// 5e)
	write_cmd1(CMD_NORMALDISPLAY);

	// 5f) Set Multiplex Ratio
	write_cmd2(CMD_SETMULTIPLEXRATIO, 0x3F); //64MUX    

	// 5g)Set Master Configuration
	write_cmd2(CMD_SETMASTERCONFIGURE, 0x8E);

	// 5h)Set Power Saving Mode
	write_cmd2(CMD_POWERSAVEMODE, 0x0B);    

	// 5i) Set Phase Length
	write_cmd2(CMD_PHASEPERIODADJUSTMENT, 0x31); //phase 2 = 14 DCLKs, phase 1 = 15 DCLKS            

	// 5j) Send Clock Divide Ratio and Oscillator Frequency
	write_cmd2(CMD_DISPLAYCLOCKDIV, 0xF0); //mid high oscillator frequency, DCLK = FpbCllk/2

	// 5k) Set Second Pre-charge Speed of Color A
	write_cmd2(CMD_SETPRECHARGESPEEDA, 0x64); //Set Second Pre-change Speed For ColorA

	// 5l) Set Set Second Pre-charge Speed of Color B
	write_cmd2(CMD_SETPRECHARGESPEEDB, 0x78); //Set Second Pre-change Speed For ColorB

	// 5m) Set Second Pre-charge Speed of Color C
	write_cmd2(CMD_SETPRECHARGESPEEDC, 0x64); //Set Second Pre-change Speed For ColorC

	// 5n) Set Pre-Charge Voltage
	write_cmd2(CMD_SETPRECHARGEVOLTAGE, 0x3A); // Pre-charge voltage =...Vcc    

	// 50) Set VCOMH Deselect Level
	write_cmd2(CMD_SETVVOLTAGE, 0x3E); // Vcomh = ...*Vcc

	// 5p) Set Master Current
	write_cmd2(CMD_MASTERCURRENTCONTROL, 0x06); 

	// 5q) Set Contrast for Color A
	write_cmd2(CMD_SETCONTRASTA, 0x91); //Set contrast for color A

	// 5r) Set Contrast for Color B
	write_cmd2(CMD_SETCONTRASTB, 0x50); //Set contrast for color B

	// 5s) Set Contrast for Color C
	write_cmd2(CMD_SETCONTRASTC, 0x7D); //Set contrast for color C

	write_cmd1(CMD_DEACTIVESCROLLING);   //disable scrolling

	// 5t) Set display ON

	// 5u) Clear Screen
	oled_rgb_clear();

	/* Turn on VCCEN and wait 25ms
	*/
	gpio_set_output(MMAP_SPIO_OLED_VBAT, 1);
	delay_ms(100);

	/* Send Display On command
	*/
	write_cmd1(CMD_DISPLAYON);

	delay_ms(300);
}

void oled_rgb_clear()
{
	cmds[0] = CMD_CLEARWINDOW; 		// Enter the “clear mode”
	cmds[1] = 0x00;					// Set the starting column coordinates
	cmds[2] = 0x00;					// Set the starting row coordinates
	cmds[3] = OLEDRGB_WIDTH - 1;	// Set the finishing column coordinates;
	cmds[4] = OLEDRGB_HEIGHT - 1;	// Set the finishing row coordinates;
	write_cmd4(cmds, 5, NULL, 0);
	delay_ms(5);
}

void oled_rgb_draw_rectangle(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned short lineColor, unsigned char bFill, unsigned short fillColor)
{
	cmds[0] = CMD_FILLWINDOW;		//fill window
	cmds[1] = (bFill ? ENABLE_FILL: DISABLE_FILL);
	cmds[2] = CMD_DRAWRECTANGLE;	//draw rectangle
	cmds[3] = c1;					// start column
	cmds[4] = r1;					// start row
	cmds[5] = c2;					// end column
	cmds[6] = r2;					//end row

	cmds[7] = ExtractRFromRGB(lineColor);	//R				
	cmds[8] = ExtractGFromRGB(lineColor);	//G
	cmds[9] = ExtractBFromRGB(lineColor);	//B


	if(bFill)
	{
		cmds[10] = ExtractRFromRGB(fillColor);	//R			
		cmds[11] = ExtractGFromRGB(fillColor);	//G
		cmds[12] = ExtractBFromRGB(fillColor);	//B
	}
	else
	{
		cmds[10] = 0;	//R			
		cmds[11] = 0;	//G
		cmds[12] = 0;	//B
	}
	//write_cmd4(cmds, bFill ? 13: 10, NULL, 0); // wrong working
	write_cmd4(cmds, 13, NULL, 0);
	delay_ms(5);
}

void oled_rgb_draw_line(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned short lineColor)
{
	cmds[0] = CMD_DRAWLINE; 		//draw line
	cmds[1] = c1;					// start column
	cmds[2] = r1;					// start row
	cmds[3] = c2;					// end column
	cmds[4] = r2;					//end row
	cmds[5] = ExtractRFromRGB(lineColor);	//R					
	cmds[6] = ExtractGFromRGB(lineColor);	//G
	cmds[7] = ExtractBFromRGB(lineColor);	//R

	write_cmd4(cmds, 8, NULL, 0);
	delay_ms(5);
}

void oled_rgb_draw_pixel(unsigned char c, unsigned char r, unsigned short pixelColor)
{
#if 1
	oled_rgb_draw_rectangle(c, r, c, r, pixelColor, 0, 0);
#else
	/*
		 cmds[0] = CMD_DRAWRECTANGLE;	//draw rectangle
		 cmds[1] = c;					// start column
		 cmds[2] = r;					// start row
		 cmds[3] = c;					// end column
		 cmds[4] = r;					//end row

		 cmds[5] = ExtractRFromRGB(pixelColor);	//R					
		 cmds[6] = ExtractGFromRGB(pixelColor);	//G
		 cmds[7] = ExtractBFromRGB(pixelColor);	//R

		 write_cmd4(cmds, 8, NULL, 0);
		 delay_ms(5);
		 */
	unsigned char data[2];
	//set column start and end
	cmds[0] = CMD_SETCOLUMNADDRESS; 		
	cmds[1] = c;					// Set the starting column coordinates
	cmds[2] = OLEDRGB_WIDTH - 1;					// Set the finishing column coordinates

	//set row start and end
	cmds[3] = CMD_SETROWADDRESS; 		
	cmds[4] = r;					// Set the starting row coordinates
	cmds[5] = OLEDRGB_HEIGHT - 1;					// Set the finishing row coordinates

	data[0] = pixelColor >> 8;
	data[1] = pixelColor;

	write_cmd4(cmds, 6, data, 2);
	delay_ms(5);
#endif	
}

void oled_rgb_draw_bitmap(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, const unsigned char *pBmp)
{
	//set column start and end
	cmds[0] = CMD_SETCOLUMNADDRESS; 		
	cmds[1] = c1;			// Set the starting column coordinates
	cmds[2] = c2;			// Set the finishing column coordinates

	//set row start and end
	cmds[3] = CMD_SETROWADDRESS; 		
	cmds[4] = r1;			// Set the starting row coordinates
	cmds[5] = r2;			// Set the finishing row coordinates

	write_cmd4(cmds, 6, pBmp, (((c2 - c1 + 1)  * (r2 - r1 + 1)) << 1));
	delay_ms(5);
}


void oled_rgb_draw_rvx_image(const ErvpImage *image)
{
	int i, j, x, y, org_index, index;
	uint16_t *rgb565;
	uint8_t *oled_image;

	assert(image->format==IMAGE_FMT_RGB_565_PACKED);

	//set column start and end
	cmds[0] = CMD_SETCOLUMNADDRESS; 		
	cmds[1] = 0;			// Set the starting column coordinates
	cmds[2] = OLEDRGB_WIDTH - 1;			// Set the finishing column coordinates

	//set row start and end
	cmds[3] = CMD_SETROWADDRESS; 		
	cmds[4] = 0;			// Set the starting row coordinates
	cmds[5] = OLEDRGB_HEIGHT - 1;			// Set the finishing row coordinates

	rgb565 = (uint16_t *)image->addr[0];
	if( (image->width != OLEDRGB_WIDTH) || (image->height != OLEDRGB_HEIGHT))
	{
		for(j = 0; j < OLEDRGB_HEIGHT; j++)
		{
			for(i = 0; i < OLEDRGB_WIDTH; i++)
			{
				index = j * OLEDRGB_WIDTH + i;
				x = i * image->width / OLEDRGB_WIDTH;
				y = j * image->height / OLEDRGB_HEIGHT;
				org_index = y * image->width + x;
				oled_buffer[index] = rgb565[org_index];
			}
		}
		oled_image = (uint8_t *)oled_buffer;
	}
	else
	{
		oled_image = (uint8_t *)rgb565;
	}

	write_bitmap(cmds, 6, oled_image, ((OLEDRGB_WIDTH  * OLEDRGB_HEIGHT) << 1));

	delay_ms(5);
}

void oled_rgb_copy(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2, unsigned char c_new, unsigned char r_new)
{
	cmds[0] = CMD_COPYWINDOW;
	cmds[1] = c1;
	cmds[2] = r1;
	cmds[3] = c2;
	cmds[4] = r2;
	cmds[5] = c_new;
	cmds[6] = r_new;

	write_cmd4(cmds, 7, NULL, 0);
	delay_ms(5);
}

void oled_rgb_dim_window(unsigned char c1, unsigned char r1, unsigned char c2, unsigned char r2)
{
	cmds[0] = CMD_DIMWINDOW;
	cmds[1] = c1;
	cmds[2] = r1;
	cmds[3] = c2;
	cmds[4] = r2;

	write_cmd4(cmds, 5, NULL, 0);
	delay_ms(5);
}

void oled_rgb_start_scrolling(unsigned char c_shift_size, unsigned char start_row_addr, unsigned char r_length, unsigned char r_shift_size, unsigned char time_interval)
{
	cmds[0] = CMD_CONTINUOUSSCROLLINGSETUP; 		
	cmds[1] = c_shift_size;
	cmds[2] = start_row_addr;
	cmds[3] = r_length;
	cmds[4] = r_shift_size;
	cmds[5] = time_interval;
	cmds[6] = CMD_ACTIVESCROLLING;

	write_cmd4(cmds, 7, NULL, 0);
	delay_ms(5);
}

void oled_rgb_stop_scrolling()
{
	cmds[0] = CMD_DEACTIVESCROLLING; 		
	write_cmd4(cmds, 1, NULL, 0);
	delay_ms(5);
}

void write_cmd1(unsigned char bVal)
{
	spi_start();
	spi_write(1, &bVal);
	spi_end();
}

void write_cmd2(unsigned char bVal1, unsigned char bVal2)
{
	unsigned char cmd[2];
	cmd[0] = bVal1;
	cmd[1] = bVal2;

	spi_start();
	spi_write(2, cmd);
	spi_end();
}

void write_cmd4(unsigned char *pCmd, int nCmd, const unsigned char *pData, int nData)
{
	spi_start();
	spi_write(nCmd, pCmd);

	if(pData != NULL)
	{
		gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_DATA);
		spi_write(nData, pData);
		gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_COMMAND);
	}
	spi_end();
}

void write_bitmap(unsigned char *pCmd, int nCmd, const unsigned char *pData, int nData)
{
	int i;
	spi_start();
	spi_write(nCmd, pCmd);

	if(pData != NULL)
	{
		gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_DATA);
		for(i=0; i<nData; i+=2)
		{
			spi_write(1, &pData[1]);
			spi_write(1, &pData[0]);
			pData += 2;
		}
		gpio_set_output(MMAP_SPIO_OLED_DCSEL, OLED_DC_SEL_COMMAND);
	}
	spi_end();
}
