#ifndef __ERVP_FUSE_BOX_H__
#define __ERVP_FUSE_BOX_H__

#include "ervp_core_id.h"
#include "ervp_fuse_box_memorymap.h"

void wakeup_core(const int core_id);
void sleep_core(const int core_id);
void disable_core();
void wfi_core();

void wakeup_user_clk(const int user_clk_select);
void sleep_user_clk(const int user_clk_select);
void disable_user_clk(const int user_clk_select);

void wfi_system(const int user_clk_not_to_sleep, const int predefined_clk_not_to_sleep);
void wfi_system_force(const int user_clk_not_to_sleep, const int predefined_clk_not_to_sleep);

static inline void sleep_self()
{
	sleep_core(EXCLUSIVE_ID);
};

#endif

