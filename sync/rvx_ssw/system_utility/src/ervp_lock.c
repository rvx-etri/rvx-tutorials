#include "platform_info.h"
#include "ervp_lock.h"

#ifdef INCLUDE_MULTICORE

void acquire_lock(int lock_index)
{
	const unsigned int addr = get_lock_addr(lock_index);
	const unsigned int wait_addr = get_wait_lock_addr(lock_index);
	while(1)
	{
		__wait_lock(wait_addr);
		if(__try_to_acquire_lock(addr)!=0)
			break;
	}
}

// API
int acquire_shared_resource(int lock_index)
{
	int interrupt_status;
	interrupt_status = get_interrupt();
	if(interrupt_status)
	{
		const unsigned int addr = get_lock_addr(lock_index);
		const unsigned int wait_addr = get_wait_lock_addr(lock_index);
		while(1)
		{
			__wait_lock(wait_addr);
			disable_interrupt();
			if(__try_to_acquire_lock(addr)!=0)
				break;
			enable_interrupt();
		}
	}
	else
	{
		acquire_lock(lock_index);
	}
	return interrupt_status;
}

void release_shared_resource(int lock_index, int interrupt_status)
{
	release_lock(lock_index);
	if(interrupt_status)
		enable_interrupt();
}

#endif // INCLUDE_MULTICORE
