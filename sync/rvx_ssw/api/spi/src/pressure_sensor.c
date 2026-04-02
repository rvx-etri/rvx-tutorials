#include "frvp_spi.h"
#include "pressure_sensor.h"
#include "platform_info.h"

#define SPI_FREQ_OF_PRESSURE_SENSOR				300000
#define SPI_MODE_OF_PRESSURE_SENSOR				SPI_SCKMODE_0
#define SPI_DIRECTION_OF_PRESSURE_SENSOR		SPI_DIR_RX

#define PORT_ID_OF_PRESSURE_SENSOR				SPI_INDEX_FOR_READYMADE
#define NUM_BYTES_OF_PRESSURE_SENSOR_VALUE		2

static SpiConfig spiconfig = {SPI_DIVSOR(SPI_FREQ_OF_PRESSURE_SENSOR), SPI_MODE_OF_PRESSURE_SENSOR, (1<<PORT_ID_OF_PRESSURE_SENSOR), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};

void configure_pressure_sensor()
{
    spi_configure(&spiconfig);
}

unsigned int read_pressure_sensor_value()
{
    unsigned int result;
    unsigned char value[NUM_BYTES_OF_PRESSURE_SENSOR_VALUE];

    spi_start();
    spi_read(NUM_BYTES_OF_PRESSURE_SENSOR_VALUE, value);
    spi_end();

    result = (0xf00 & ((unsigned int)value[0])<<8)  |  (0xff & value[1]);

    return result;
}

