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
// 8.May.2017 
// Sukho Lee (shle99@etri.re.kr)
// ****************************************************************************
// ****************************************************************************
/*
 ** SPI Setup
 *
 * SPI_REG_SCKDIV(SPI_FLASH_BASEADDR)
 *   Fsck = Fin/(2*(div+1))   Fin=tlclk=cpuclk=16 MHz
 *   So at least 1, which puts the clock rate at 8 MHz (of max 10 MHz)
 *
 * SPI_REG_CSMODE(SPI_FLASH_BASEADDR)
 *   mode
 *     3 OFF Disable hardware control of the CS pin
 * SPI_REG_SCKMODE(SPI_FLASH_BASEADDR)
 *   pol
 *     0 Inactive state of SCLK is logical 0
 *   pha
 *     0 Value of SDIN is sampled at SCLK's rising edge so shift out a new bit at falling edge
 * SPI_REG_FMT(SPI_FLASH_BASEADDR)
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
#include "ervp_printf.h"
#include "ervp_delay.h"
#include "ervp_external_peri_group_memorymap_offset.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"

#include "platform_info.h"
#include "frvp_spi_flash.h"
#include "frvp_spi.h"

#define SPI_FREQ_OF_FLASH		1000000
#define SPI_MODE_OF_FLASH			SPI_SCKMODE_3

#ifdef SPI_INDEX_FOR_SPI_FLASH
static const SpiConfig spi_flash_spi_config = {SPI_DIVSOR(SPI_FREQ_OF_FLASH), SPI_MODE_OF_FLASH, (1<<SPI_INDEX_FOR_SPI_FLASH), SPI_CSMODE_OFF, (SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_LEN(8)), 1};
#else
static SpiConfig spi_flash_spi_config = {0, 0, 0, 0, 0, 0};
#endif

void configure_spi_flash(void)
{
	spi_configure(&spi_flash_spi_config);
}

unsigned char read_status_reg()
{
	unsigned char buffer[4];
	buffer[0] = SPI_FLASH_INST_RDSR;

	spi_start();
	spi_write(1, buffer);

	spi_read(1, buffer);
	spi_end();

	return buffer[0];
}

unsigned char read_flag_status_reg()
{
	unsigned char buffer[4];
	buffer[0] = SPI_FLASH_INST_RFSR;

	spi_start();
	spi_write(1, buffer);

	spi_read(1, buffer);
	spi_end();

	return buffer[0];
}

unsigned char read_lock_reg(unsigned int addr)
{
	unsigned char buffer[4];
	buffer[0] = SPI_FLASH_INST_RDLR;
	buffer[1] = (addr>>16) & 0xFF;
	buffer[2] = (addr>>8) & 0xFF;
	buffer[3] = addr & 0xFF;

	spi_start();
	spi_write(4, buffer);

	spi_read(1, buffer);
	spi_end();

	return buffer[0];
}

void suspend_program_or_erase()
{
	unsigned char buffer[4];

	buffer[0] = SPI_FLASH_INST_PES;	// Program/Erase Suspend
	spi_start();
	spi_write(1, buffer);
	spi_end();
	while((read_flag_status_reg() & SPI_FLASH_FLAG_STATUS_ES) == SPI_FLASH_FLAG_STATUS_ES)
		delay_unit(1000);
}

void read_flash(unsigned int addr, int num_bytes, unsigned char *rdata) 
{
	unsigned char buffer[4];
	int num_reads;
	
	while(num_bytes>0)
	{
		if(num_bytes > SPI_FLASH_PAGE_SIZE)
			num_reads = SPI_FLASH_PAGE_SIZE;
		else
			num_reads = num_bytes;

		buffer[0] = SPI_FLASH_INST_READ;	// read data
		buffer[1] = (addr>>16) & 0xFF;
		buffer[2] = (addr>>8) & 0xFF;
		buffer[3] = addr & 0xFF;

		spi_start();
		spi_write(4, buffer);

		spi_read(num_reads, rdata);
		spi_end();

		num_bytes -= num_reads;
		addr += num_reads;
		rdata += num_reads;
	}
}

void write_flash_wo_erase(unsigned int addr, int num_bytes, unsigned char *wdata) 
{
	unsigned char buffer[4];
	int num_writes;

	while(num_bytes>0)
	{
		if(num_bytes > SPI_FLASH_PAGE_SIZE)
			num_writes = SPI_FLASH_PAGE_SIZE;
		else
			num_writes = num_bytes;
		buffer[0] = SPI_FLASH_INST_WREN;
		spi_start();
		spi_write(1, buffer);	// write enable
		spi_end();

		buffer[0] = SPI_FLASH_INST_PP;	// page program	(max write size: 256 bytes(page size))
		buffer[1] = (addr>>16) & 0xFF;
		buffer[2] = (addr>>8) & 0xFF;
		buffer[3] = addr & 0xFF;

		spi_start();
		spi_write(4, buffer);

		spi_write(num_writes, wdata);
		spi_end();

		while((read_status_reg() & SPI_FLASH_STAUTS_WIP) == SPI_FLASH_STAUTS_WIP)
			delay_unit(100);
		num_bytes -= num_writes;
		addr += num_writes;
		wdata += num_writes;
	}
}

void erase_sector(unsigned int addr)
{
	// erase flash
	unsigned char buffer[4];
	buffer[0] = SPI_FLASH_INST_WREN;
	spi_start();
	spi_write(1, buffer);	// Write enable
	spi_end();

	addr = align_sector_addr(addr);
	printf("\n[RVPAPI|INFO] flash erase 0x%8x\n", addr);
	buffer[0] = SPI_FLASH_INST_SE;	// sector erase	(subsector: 4 Kbytes, sector: 64 Kbytes)
	buffer[1] = (addr>>16) & 0xFF;
	buffer[2] = (addr>>8) & 0xFF;
	buffer[3] = addr & 0xFF;
	spi_start();
	spi_write(4, buffer);
	spi_end();

	if(is_sim())
		delay_ms(7);
	else
		delay_ms(700);

	while((read_status_reg() & SPI_FLASH_STAUTS_WIP) == SPI_FLASH_STAUTS_WIP)
		delay_unit(1000);
	printf("\n[RVPAPI|INFO] flash erase complete");
}

void write_flash(unsigned int addr, int num_bytes, unsigned char *wdata)
{
	int i, j;
	int num_sectors, write_size_at_last_sector, num_pages, remainder;

	if( (addr % SPI_FLASH_SECTOR_SIZE) != 0 )
	{
		printf("can't dump data to flash!\n");
		printf("must align flash address!\n");
	}
	else
	{
		num_sectors = num_bytes / SPI_FLASH_SECTOR_SIZE;
		write_size_at_last_sector = num_bytes % SPI_FLASH_SECTOR_SIZE;

		for(i=0; i<num_sectors; i++)
		{
			erase_sector(addr);
			for(j=0; j<SPI_FLASH_NUM_PAGES_PER_SECTOR; j++)
			{
				//printf("0x%x 0x%x\n", wdata, addr);
				write_flash_wo_erase(addr, SPI_FLASH_PAGE_SIZE, wdata);
				wdata += SPI_FLASH_PAGE_SIZE;
				addr += SPI_FLASH_PAGE_SIZE;
			}
		}

		if(write_size_at_last_sector != 0)
		{
			erase_sector(addr);

			num_pages = write_size_at_last_sector / SPI_FLASH_PAGE_SIZE;
			remainder = write_size_at_last_sector % SPI_FLASH_PAGE_SIZE;

			for(i=0; i<num_pages; i++)
			{
				//printf("0x%x 0x%x\n", wdata, addr);
				write_flash_wo_erase(addr, SPI_FLASH_PAGE_SIZE, wdata);
				wdata += SPI_FLASH_PAGE_SIZE;
				addr += SPI_FLASH_PAGE_SIZE;
			}

			if(remainder != 0)
			{
				//printf("0x%x 0x%x\n", wdata, addr);
				write_flash_wo_erase(addr, remainder, wdata);
			}
		}
	}

}

void verify_flash_id(void)
{
	int i;
	unsigned char buffer[4];
	buffer[0] = SPI_FLASH_INST_RDID; 
	spi_start();
	spi_write(1, buffer);
	spi_read(3, buffer);
	spi_end();

#if 1
	for(i=0;i<3;i++)
		printf("flash id [%d]: 0x%x\n", i, buffer[i]);
	// Manufacturer ID : 0x20, Memory Type: 0xba, 18, 10
#endif
};







#if 0
void set_flash_mode(void)
{
	// Quad mode setting
	unsigned char buffer[3];
	buffer[0] = SPI_FLASH_INST_WREN;
	spi_start();
	spi_write(1, buffer);  // Write enable
	spi_end();

	// The Non Volatile Configuration Register bits determine the device memory configuration after power-on.
	buffer[0] = SPI_FLASH_INST_WRNVCR; // Write Non Volatile Configuration register
	buffer[1] = 0xef;	// NVCR LS Byte
	buffer[2] = 0xff;	// NVCR MS Byte
	// 1111 : 111 : 111 : 1 : 0 : 1 : 1 : 11 <- all disable
	spi_start();
	spi_write(3, buffer);
	spi_end();
}
#endif


#if 0
void command_flash(unsigned char data1, unsigned char data2, unsigned char data3)  
{
	REG32(SPI_REG_FCTRL(SPI_FLASH_BASEADDR)) = SPI_FCTRL_EN;
	REG32(SPI_REG_FFMT(SPI_FLASH_BASEADDR))  = SPI_INSN_CMD_EN            |   
		SPI_INSN_ADDR_LEN(0x0)     |   
		SPI_INSN_PAD_CNT(0x0)      |   
		SPI_INSN_CMD_PROTO(data1)  |   // 0:8, 1:4, 2:2, 3:0 (0~3)
		SPI_INSN_ADDR_PROTO(0x0)   |   
		SPI_INSN_DATA_PROTO(data2) |   // 0:8, 1:4, 2:2, 3:0 (0~3)
		SPI_INSN_CMD_CODE(data3)   |   
		SPI_INSN_PAD_CODE(0x00);       
}

void read_flash(void)  // Quad Output Fast Read (0x6B)
{
	// SPI Flash Memory Enable
	REG32(SPI_REG_FCTRL(SPI_FLASH_BASEADDR)) = SPI_FCTRL_EN;                  // flash interface control
	REG32(SPI_REG_FFMT(SPI_FLASH_BASEADDR))  = SPI_INSN_CMD_EN            |   // Enable sending if command
		SPI_INSN_ADDR_LEN(0x3)     |   // Number of addr bytes (0 to 4)
		SPI_INSN_PAD_CNT(0x8)      |   // Number of dummy cycles (0x0)
		SPI_INSN_CMD_PROTO(0x00)   |   // Protocol for transmitting command,  2 cycle : 2 (default, extended cycle)
		SPI_INSN_ADDR_PROTO(0x0)   |   // Protocol for transmitting adderss and padding (0x0)
		SPI_INSN_DATA_PROTO(0x2)   |   // Protocol for receiveing data bytes read 4 cycle: 1 0x0
		SPI_INSN_CMD_CODE(0x6B)    |   // Value of command byte, Quad Read
		SPI_INSN_PAD_CODE(0x00);       // First 8 bits to transmit during dummy cycles
	//printf("2.Flash Read Configuration Done.\n"); //, Check(0x30007)=%x\n",REG32(SPI_REG_FFMT(SPI_FLASH_BASEADDR)));
}

void disable_flash()
{
	REG32(SPI_REG_FCTRL(SPI_FLASH_BASEADDR))   = !SPI_FCTRL_EN;
}
#endif
