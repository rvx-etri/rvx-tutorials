#include "frvp_spi.h"
#include "light_sensor.h"
#include "ervp_uart.h"
#include "ervp_printf.h"
#include "platform_info.h"

#define SPI_FREQ_OF_LIGHT_SENSOR            2500000
#define SPI_MODE_OF_LIGHT_SENSOR            SPI_SCKMODE_3

#define PORT_ID_OF_LIGHT_SENSOR				SPI_INDEX_FOR_READYMADE
#define NUM_BYTES_OF_LIGHT_SENSOR_VALUE		2

static const SpiConfig spiconfig = {SPI_DIVSOR(SPI_FREQ_OF_LIGHT_SENSOR), SPI_MODE_OF_LIGHT_SENSOR, (1<<PORT_ID_OF_LIGHT_SENSOR), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};

void configure_light_sensor()
{
	spi_configure(&spiconfig);
}

unsigned int read_light_sensor_value()
{
	unsigned int result;
	unsigned char value[NUM_BYTES_OF_LIGHT_SENSOR_VALUE];

	spi_start();
	spi_read(NUM_BYTES_OF_LIGHT_SENSOR_VALUE, value);
	spi_end();
	
	result = (0xf8 & value[0]<<3)  |  (0x07 & value[1]>>5);
	
	return result;
}
