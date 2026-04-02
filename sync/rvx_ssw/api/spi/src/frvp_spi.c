// ****************************************************************************
// Copyright SoC Design Research Group, All rights reserved.    
// Electronics and Telecommunications Research Institute (ETRI)
//
// THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
// WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
// TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
// REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
// SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
// IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
// COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
// ****************************************************************************
// 13.Apr.2017 
// Sukho Lee (shle99@etri.re.kr)
// ****************************************************************************
// ****************************************************************************
/*
 ** SPI Setup
 *
 * SPI_REG_SCKDIV(SPI_BASEADDR)
 *   Fsck = Fin/(2*(div+1))   Fin=tlclk=cpuclk=16 MHz
 *   So at least 1, which puts the clock rate at 8 MHz (of max 10 MHz)
 *
 * SPI_REG_CSMODE(SPI_BASEADDR)
 *   mode
 *     3 OFF Disable hardware control of the CS pin
 * SPI_REG_SCKMODE(SPI_BASEADDR)
 *   pol
 *     0 Inactive state of SCLK is logical 0
 *   pha
 *     0 Value of SDIN is sampled at SCLK's rising edge so shift out a new bit at falling edge
 * SPI_REG_FMT(SPI_BASEADDR)
 *   proto
 *     0 single
 *   endian
 *     0 Data is clocked from bit 7 (MSB) to bit 0 (LSB)
 *   dir
 *     1 Tx: the receive fifo is not populated
 *   len
 *     8 bits per frame
 */

#include "ervp_misc_util.h"
#include "ervp_mmio_util.h"
#include "ervp_uart.h"
#include "frvp_spi.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"
#include "ervp_external_peri_group_memorymap.h"

#define SPI_BASEADDR            (EXTERNAL_PERI_GROUP_BASEADDR + SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_SPI)

#define SPI_REG_SCKDIV          (0x00 + SPI_BASEADDR)
#define SPI_REG_SCKMODE         (0x04 + SPI_BASEADDR)
#define SPI_REG_CSID            (0x10 + SPI_BASEADDR)
#define SPI_REG_CSDEF           (0x14 + SPI_BASEADDR)
#define SPI_REG_CSMODE          (0x18 + SPI_BASEADDR)

#define SPI_REG_DCSSCK          (0x28 + SPI_BASEADDR)
#define SPI_REG_DSCKCS          (0x2a + SPI_BASEADDR)
#define SPI_REG_DINTERCS        (0x2c + SPI_BASEADDR)
#define SPI_REG_DINTERXFR       (0x2e + SPI_BASEADDR)

#define SPI_REG_FMT             (0x40 + SPI_BASEADDR)
#define SPI_REG_TXFIFO          (0x48 + SPI_BASEADDR)
#define SPI_REG_RXFIFO          (0x4c + SPI_BASEADDR)
#define SPI_REG_TXCTRL          (0x50 + SPI_BASEADDR)
#define SPI_REG_RXCTRL          (0x54 + SPI_BASEADDR)

#define SPI_REG_FCTRL           (0x60 + SPI_BASEADDR)
#define SPI_REG_FFMT            (0x64 + SPI_BASEADDR)

#define SPI_REG_IE              (0x70 + SPI_BASEADDR)
#define SPI_REG_IP              (0x74 + SPI_BASEADDR)

// DMA
#define SPI_DMA_EN              (0x100 + SPI_BASEADDR)
#define SPI_DMA_ADDR            (0x104 + SPI_BASEADDR)
#define SPI_DMA_VSIZE           (0x108 + SPI_BASEADDR)
#define SPI_DMA_HSIZE           (0x10C + SPI_BASEADDR)
#define SPI_DMA_STRIDE          (0x110 + SPI_BASEADDR)
#define SPI_DMA_ENDIAN          (0x114 + SPI_BASEADDR)
#define SPI_DMA_TXMODE          (0x118 + SPI_BASEADDR)
#define SPI_DMA_INTR            (0x120 + SPI_BASEADDR)

void print_spi_config(const SpiConfig *config)
{
	char bitvector[33];
	printf("\ndivisor: %u", config->divisor);
	printf("\nsck_mode: %u", config->sck_mode);
	convert_value_to_bitvector(config->select, bitvector);
	printf("\nselect: %s", bitvector);
	printf("\ncs_mode: %u", config->cs_mode);
	printf("\nframe_format: %u", config->frame_format);
	printf("\ncs_active_low: %u", config->cs_active_low);
	printf("\n");
}

void print_spi_status()
{
	char bitvector[33];
	SpiConfig config;
	spi_store_config(&config);
	printf("\ndivisor: %u", config.divisor);
	printf("\nsck_mode: %u", config.sck_mode);
	convert_value_to_bitvector(config.select, bitvector);
	printf("\nselect: %s", bitvector);
	printf("\ncs_mode: %u", config.cs_mode);
	printf("\nframe_format: %u", config.frame_format);
	convert_value_to_bitvector(REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW), bitvector);
	printf("\ncs_active_low_list: %s", bitvector);
	printf("\n");
}

static int spi_is_initialized = 0;

static inline void __spi_init_once()
{
	REG32(SPI_REG_CSID)    = 0x0;
	REG32(SPI_REG_CSDEF)   = 0xffff;
	REG32(SPI_REG_TXCTRL) = 1;
	REG32(SPI_REG_RXCTRL) = 0;
	// SPI Flash Memory Disable 
	REG32(SPI_REG_FCTRL) = 0x0;
	REG32(SPI_REG_FFMT)  = 0x0;
	REG32(SPI_REG_IE) = 0x0;
}

void spi_configure(const SpiConfig *config)
{
	unsigned int value, temp;
	if(spi_is_initialized==0)
	{
		__spi_init_once();
		spi_is_initialized = 1;
	}
	if(config->cs_active_low)
	{
		temp = config->select;
		value = REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW);
		value = value | temp;
	}
	else
	{
		temp = ~config->select;
		value = REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW);
		value = value & temp;
	}
	REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW) = value;
	spi_restore_config(config);
}

void spi_store_config(SpiConfig *config)
{
	config->divisor = REG32(SPI_REG_SCKDIV);
	config->sck_mode = REG32(SPI_REG_SCKMODE);
	config->select = REG32(MMAP_SPIO_SPI_SELECT);
	config->cs_mode = REG32(SPI_REG_CSMODE);
	config->frame_format = REG32(SPI_REG_FMT);
}

void spi_restore_config(const SpiConfig *config)
{
	REG32(SPI_REG_SCKDIV) = config->divisor;
	REG32(SPI_REG_SCKMODE) = config->sck_mode;
	REG32(MMAP_SPIO_SPI_SELECT) = config->select;
	REG32(SPI_REG_CSMODE) = config->cs_mode;
	REG32(SPI_REG_FMT) = config->frame_format;
}

void spi_start()
{
	//disable_interrupt();
	REG32(SPI_REG_CSMODE)  = SPI_CSMODE_HOLD;
}

void spi_end()
{
	REG32(SPI_REG_CSMODE)  = SPI_CSMODE_OFF;
	//enable_interrupt();
}

void spi_set_direction(int direction)
{
	if(direction == SPI_DIR_RX)
		REG32(SPI_REG_FMT) &= ~(SPI_FMT_DIR_BIT);
	else
		REG32(SPI_REG_FMT) |= SPI_FMT_DIR_BIT;
}

void spi_read(int num_bytes, unsigned char* data)
{
	int i;

	spi_set_direction(SPI_DIR_RX);

	for(i=0; i<num_bytes; i++)
	{
		while (REG32(SPI_REG_TXFIFO) & SPI_TXFIFO_FULL)
			IDLE;
		REG32(SPI_REG_TXFIFO) = 0x0;
		while (!(REG32(SPI_REG_IP) & SPI_IP_RXWM));	// wait for reading
		//data[i] = 0xff & REG32(SPI_REG_RXFIFO);
		data[i] = REG8(SPI_REG_RXFIFO);
	}
}

// not verified yet
void spi_write(int num_bytes, const unsigned char* data)
{
	int i;

	spi_set_direction(SPI_DIR_TX);

	for(i=0; i<num_bytes; i++)
	{
		while (REG32(SPI_REG_TXFIFO) & SPI_TXFIFO_FULL)
			IDLE;
		REG32(SPI_REG_TXFIFO) = data[i];
	}
	// wait until FIFO becomes empty
	while ((REG32(SPI_REG_IP) & SPI_IP_TXWM)==0);
}


void spi_read_and_write(int num_bytes, const unsigned char* tx_data, unsigned char* rx_data)
{
	int i;

	spi_set_direction(SPI_DIR_RX);

	for(i=0; i<num_bytes; i++)
	{
		while (REG32(SPI_REG_TXFIFO) & SPI_TXFIFO_FULL)
			IDLE;
		if(tx_data == 0)
			REG32(SPI_REG_TXFIFO) = 0x0;
		else
			REG32(SPI_REG_TXFIFO) = tx_data[i];

		while (!(REG32(SPI_REG_IP) & SPI_IP_RXWM));   // wait for reading
		if(rx_data == 0)
			REG8(SPI_REG_RXFIFO);
		else
			rx_data[i] = REG8(SPI_REG_RXFIFO);
	}
}
