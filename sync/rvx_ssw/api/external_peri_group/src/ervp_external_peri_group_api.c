#include "ervp_external_peri_group_api.h"
#include "ervp_gpio.h"
#include "ervp_user_gpio.h"
#include "ervp_bit_util.h"
#include "ervp_lock.h"

static volatile unsigned int spi_user_output_is_occupied = 0;
static volatile unsigned int spi_cs_backup;
static volatile unsigned int spi_select_backup;

static inline unsigned int get_extreg_addr(int i)
{
	unsigned int addr;
	addr = MMAP_EPG_MISC_EXTREG00;
	addr += (i<<BW_UNUSED_EPG_MISC);
	return addr;
}

int get_gpreg(int i)
{
	return REG32(get_extreg_addr(i));
}

void set_gpreg(int i, int data)
{
	REG32(get_extreg_addr(i)) = data;
}

void gpio_tick_enable(int value)
{
  mmio_write_data(MMAP_EPG_MISC_GPIO_TICK_CFG, (value<<1)|1);
}

void gpio_tick_disable()
{
  mmio_write_data(MMAP_EPG_MISC_GPIO_TICK_CFG, 0);
}

void init_switch()
{
#ifdef INCLUDE_SWITCH_SLIDE_READYMADE
	int i;
	for(i=0; i<NUM_SWITCH_SLIDE_READYMADE; i++)
		user_gpio_set_input_cfg(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0+i, GPIO_CONFIG_DEBOUNCE_ENABLE);
#endif
}

unsigned int get_slide_switch(int index)
{
	unsigned int result = -1;
#ifdef INCLUDE_SWITCH_SLIDE_READYMADE
	result = user_gpio_get_input(GPIO_INDEX_FOR_SWITCH_SLIDE_READYMADE_0+index);
#endif
	return result;
}

void spi_user_output_acquire_lock()
{
#ifdef LOCK_INDEX_FOR_SYSTEM_VARIABLE
	const int lock_index = LOCK_INDEX_FOR_SYSTEM_VARIABLE;
#else
	const int lock_index = -1;
#endif

	while(1)
	{
		acquire_lock(lock_index);
		if(spi_user_output_is_occupied)
			release_lock(lock_index);
		else
		{
			spi_user_output_is_occupied = 1;
			spi_cs_backup = REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW);
			spi_select_backup = REG32(MMAP_SPIO_SPI_SELECT);
			release_lock(lock_index);
			break;
		}
	}
}

void spi_user_output_release_lock()
{
	REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW) = spi_cs_backup;
	REG32(MMAP_SPIO_SPI_SELECT) = spi_select_backup;
	spi_user_output_is_occupied = 0;
}

void spi_cs_config_user_output(unsigned int spi_index, unsigned int enable, unsigned int value)
{
	if(enable)
	{
		unsigned int spi_cs_active_low_list = REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW);
		if(value)
			spi_cs_active_low_list = set_bits_by_index(spi_cs_active_low_list,spi_index,spi_index);
		else
			spi_cs_active_low_list = clear_bits_by_index(spi_cs_active_low_list,spi_index,spi_index);
		REG32(MMAP_SPIO_SPI_CS_ACTIVE_LOW) = spi_cs_active_low_list;
		REG32(MMAP_SPIO_SPI_SELECT) = 0;
	}
}

void spi_clk_config_user_output(unsigned int spi_index, unsigned int enable, unsigned int value)
{
	unsigned int control = REG32(MMAP_SPIO_SERIAL_COMM_CONTROL);
	if(enable)
	{
		control = set_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_OE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_OE);
		if(value)
			control = set_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_VALUE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_VALUE);
		else
			control = clear_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_VALUE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_VALUE);
	}
	else
	{
		control = clear_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_OE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_CLK_USER_OE);
	}
	REG32(MMAP_SPIO_SERIAL_COMM_CONTROL) = control;
}

void spi_dq0_config_user_output(unsigned int spi_index, unsigned int enable, unsigned int value)
{
	unsigned int control = REG32(MMAP_SPIO_SERIAL_COMM_CONTROL);
	if(enable)
	{
		control = set_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_OE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_OE);
		if(value)
			control = set_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_VALUE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_VALUE);
		else
			control = clear_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_VALUE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_VALUE);
	}
	else
	{
		control = clear_bits_by_index(control, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_OE, SPIO_SERIAL_COMM_CONTROL_INDEX_SPI_DQ0_USER_OE);
	}
	REG32(MMAP_SPIO_SERIAL_COMM_CONTROL) = control;
}
