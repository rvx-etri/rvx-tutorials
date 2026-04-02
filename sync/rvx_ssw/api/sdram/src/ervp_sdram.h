#ifndef __ERVP_SDRAM_H__
#define __ERVP_SDRAM_H__

#include <stdint.h>

uint32_t sdram_get_cotrol_reg(unsigned int offset);
void sdram_set_cotrol_reg(unsigned int offset, uint32_t value);

void sdram_set_clk_skew(int value);
void sdram_set_clk_pol(int value);
void sdram_set_ras(int value);

#endif

