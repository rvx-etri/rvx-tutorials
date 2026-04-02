#include "ervp_sdram.h"
#include "ervp_mmio_util.h"
#include "ervp_bit_util.h"

uint32_t sdram_get_cotrol_reg(unsigned int offset)
{
  return mmio_read_data(I_SYSTEM_SDRAM_CTRL_BASEADDR+offset);
}

void sdram_set_cotrol_reg(unsigned int offset, uint32_t value)
{
  mmio_write_data(I_SYSTEM_SDRAM_CTRL_BASEADDR+offset, value);
}

void sdram_set_clk_skew(int value)
{
	uint32_t reg;
	reg = sdram_get_cotrol_reg(0x0);
	reg = replace_partial_bits(reg, 10, 5, value);
	sdram_set_cotrol_reg(0x0, reg);
}

void sdram_set_clk_pol(int value)
{
	uint32_t reg;
	reg = sdram_get_cotrol_reg(0x0);
	reg = replace_partial_bits(reg, 11, 11, value);
	sdram_set_cotrol_reg(0x0, reg);
}

void sdram_set_ras(int value)
{
	uint32_t reg;
	reg = sdram_get_cotrol_reg(0x8);
	reg = replace_partial_bits(reg, 5, 3, value);
	sdram_set_cotrol_reg(0x8, reg);
}
