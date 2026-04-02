#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "frvp_spi.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "orvp_rtcc_module.h"
#include "orvp_i2c.h"

#define I2C_INDEX_FOR_RTCC_MODULE	0
#define I2C_FREQ_OF_RTCC_MODULE		500

#define RTCC_MODULE_ADDRESS		0x6f
#define REG_ADDRESS			0x0

#define RTCSEC_SEC(sec)     ((int)(sec/10)<<4) | (sec%10)
#define RTCSEC_MIN(min)     ((int)(min/10)<<4) | (min%10)
#define RTCSEC_HOUR_24(hour)        ((int)(hour/10)<<4) | (hour%10)
#define RTCSEC_DATE(date)       ((int)(date/10)<<4) | (date%10)
#define RTCSEC_MONTH(month)     ((int)(month/10)<<4) | (month%10)
#define RTCSEC_YEAR(year)       ((int)(year/10)<<4) | (year%10)

/* SECONDS VALUE REGISTER */
#define START_BIT                       (0x1<<7)

/* WEEKDAY VALUE REGISTER */
#define OSCRUN_BIT      (0x1<<5)    // Oscillator Status bit
#define PWRFAIL_BIT     (0x1<<4)    // Power Failure Status bit
#define VBATEN_BIT      (0x1<<3)    // External Battery Backup Supply (VBAT) Enable bit

/* MONTH VALUE REGISTER */
#define LPYR_BIT_OF_MONTH_REG           (0x1<<5)    // Leap Year bit
#define NUM_BYTES_OF_DATA		7

void configure_i2c_for_rtcc_module()
{
    configure_i2c(I2C_INDEX_FOR_RTCC_MODULE, I2C_FREQ_OF_RTCC_MODULE, I2C_ENABLE, I2C_INTERRUPT_DISABLE);
}

void configure_rtcc_module(unsigned int sec, unsigned int min, unsigned int hour, unsigned int day, unsigned int date, unsigned int month, unsigned int year)
{
	unsigned char data[NUM_BYTES_OF_DATA];

	data[0] = START_BIT | RTCSEC_SEC(sec);		// 0x00,start, sec
	data[1] = RTCSEC_MIN(min);            		// 0x01,min
	data[2] = RTCSEC_HOUR_24(hour);       		// 0x02,hour
	data[3] = VBATEN_BIT | day;           		// 0x03,day with Battery
	data[4] = RTCSEC_DATE(date);          		// 0x04,date
	data[5] = RTCSEC_MONTH(month);        		// 0x05,month
	data[6] = RTCSEC_YEAR(year);          		// 0x06,year

	write_i2c_r1dn(I2C_INDEX_FOR_RTCC_MODULE, RTCC_MODULE_ADDRESS, REG_ADDRESS, data, NUM_BYTES_OF_DATA);

	//printf("OSC start and Time Setting Done.\n");
}

void read_time(unsigned int *sec, unsigned int *min, unsigned int *hour, unsigned int *day, unsigned int *date, unsigned int *month, unsigned int *year)
{
	unsigned char data[NUM_BYTES_OF_DATA];

	read_i2c_r1dn(I2C_INDEX_FOR_RTCC_MODULE, RTCC_MODULE_ADDRESS, REG_ADDRESS, data, NUM_BYTES_OF_DATA);

	// 0x00,sec
	*sec = ((data[0] & 0x70) >> 4) * 10 + (data[0] & 0x0f);

	// 0x01,min
	*min = ((data[1] & 0x70) >> 4) * 10 + (data[1] & 0x0f);

	// 0x02,hour
	*hour = ((data[2] & 0x30) >> 4) * 10 + (data[2] & 0x0f);

	// 0x03,day
	*day = data[3] & 0x07;

	// 0x04,date
	*date = ((data[4] & 0x30) >> 4) * 10 + (data[4] & 0x0f);

	// 0x05,month
	*month = ((data[5] & 0x10) >> 4) * 10 + (data[5] & 0x0f);

	// 0x06,year
	*year = ((data[6] & 0xf0) >> 4) * 10 + (data[6] & 0x0f);
}
