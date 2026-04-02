#include "ervp_core_peri_group_memorymap_offset.h"
#include "ervp_tcu_memorymap.h"

#include "ervp_mmio_util.h"

void tcu_set_region_valid(int index, unsigned int value)
{
	unsigned int valid_list = REG32(MMAP_TCU_REGION_VALID_LIST);
	if(value==0)
	{
		value = 1 << index;
		value = ~value;
		valid_list = valid_list & value;
	}
	else
	{
		value = 1 << index;
		valid_list = valid_list | value;
	}
	REG32(MMAP_TCU_REGION_VALID_LIST) = valid_list;
}

unsigned int tcu_get_region_valid_list()
{
	return REG32(MMAP_TCU_REGION_VALID_LIST);
}

void tcu_clear_all_region()
{
	REG32(MMAP_TCU_REGION_VALID_LIST) = 0;
}

void tcu_set_region_start(int index, unsigned int value)
{
	unsigned int addr = MMAP_TCU_REGION_START00 + (ERVP_TCU_ADDR_INTERVAL*index);
	REG32(addr) = value;
}

unsigned int tcu_get_region_start(int index)
{
	unsigned int addr = MMAP_TCU_REGION_START00 + (ERVP_TCU_ADDR_INTERVAL*index);
	return REG32(addr);
}

void tcu_set_region_last(int index, unsigned int value)
{
	unsigned int addr = MMAP_TCU_REGION_LAST00 + (ERVP_TCU_ADDR_INTERVAL*index);
	REG32(addr) = value;
}

unsigned int tcu_get_region_last(int index)
{
	unsigned int addr = MMAP_TCU_REGION_LAST00 + (ERVP_TCU_ADDR_INTERVAL*index);
	return REG32(addr);
}

void tcu_set_region_offset(int index, unsigned int value)
{
	unsigned int addr = MMAP_TCU_REGION_OFFSET00 + (ERVP_TCU_ADDR_INTERVAL*index);
	REG32(addr) = value;
}

unsigned int tcu_get_region_offset(int index)
{
	unsigned int addr = MMAP_TCU_REGION_OFFSET00 + (ERVP_TCU_ADDR_INTERVAL*index);
	return REG32(addr);
}
