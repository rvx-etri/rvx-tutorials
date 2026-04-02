#include "ervp_tick.h"
#include "ervp_common_peri_group_memorymap.h"
#include "ervp_mmio_util.h"

#include "platform_info.h"

void init_system_tick()
{
	const unsigned int target_value = ((SYSTEM_CLK_HZ-1)/TICK_HZ) + 1;
	REG32(MMAP_CPG_MISC_SYSTEM_TICK_CONFIG) = (target_value<<1) | 1;
}

void init_core_tick()
{
	const unsigned int target_value = ((CORE_CLK_HZ-1)/TICK_HZ) + 1;
	REG32(MMAP_CPG_MISC_CORE_TICK_CONFIG) = (target_value<<1) | 1;
}
