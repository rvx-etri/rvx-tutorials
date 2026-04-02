#include "ervp_external_peri_group_api.h"
#include "ervp_fuse_box.h"

static inline int set_clock_cmd(unsigned int status)
{
	return (status<<(32-BW_CLOCK_CMD));
}
void wakeup_core(const int core_id)
{
	REG32(MMAP_FUSE_BOX_CORE_CLOCK_CMD) = (1<<core_id) | set_clock_cmd(CLOCK_CMD_WAKEUP);
}

void sleep_core(const int core_id)
{
	REG32(MMAP_FUSE_BOX_CORE_CLOCK_CMD) = (1<<core_id) | set_clock_cmd(CLOCK_CMD_SLEEP);
}

void disable_core()
{
	const int core_id = EXCLUSIVE_ID;
	REG32(MMAP_FUSE_BOX_CORE_CLOCK_CMD) = (1<<core_id) | set_clock_cmd(CLOCK_CMD_DISABLE);
}

static inline void __wfi_core()
{
	const int core_id = EXCLUSIVE_ID;
	REG32(MMAP_FUSE_BOX_CORE_CLOCK_CMD) = (1<<core_id) | set_clock_cmd(CLOCK_CMD_WFI);
}

void wfi_core()
{
	__wfi_core();
}

void wakeup_user_clk(const int user_clk_select)
{
	REG32(MMAP_FUSE_BOX_USER_CLOCK_CMD) = user_clk_select | set_clock_cmd(CLOCK_CMD_WAKEUP);
}

void sleep_user_clk(const int user_clk_select)
{
	REG32(MMAP_FUSE_BOX_USER_CLOCK_CMD) = user_clk_select | set_clock_cmd(CLOCK_CMD_SLEEP);
}

void disable_user_clk(const int user_clk_select)
{
	REG32(MMAP_FUSE_BOX_USER_CLOCK_CMD) = user_clk_select | set_clock_cmd(CLOCK_CMD_DISABLE);
}

static inline void __wfi_system_force(const int user_clk_not_to_sleep, const int predefined_clk_not_to_sleep)
{
	REG32(MMAP_FUSE_BOX_SYSTEM_CMD) = (set_clock_cmd(CLOCK_CMD_WFI) | (user_clk_not_to_sleep<<NUM_PREDEFINED_CLOCK) | predefined_clk_not_to_sleep);
}

void wfi_system_force(const int user_clk_not_to_sleep, const int predefined_clk_not_to_sleep)
{
	__wfi_system_force(user_clk_not_to_sleep, predefined_clk_not_to_sleep);
}

void wfi_system(const int user_clk_not_to_sleep, const int predefined_clk_not_to_sleep)
{
	/* BUG
	int id = get_dynamic_id(NUM_CORE_USER);
	if(id==(NUM_CORE_USER-1))
		__wfi_system_force(user_clk_not_to_sleep, predefined_clk_not_to_sleep);
	else
		__wfi_core();
	*/
	while(1);
}
