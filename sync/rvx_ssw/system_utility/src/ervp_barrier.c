#include "platform_info.h"
#include "ervp_mmio_util.h"
#include "ervp_lock.h"
#include "ervp_auto_id.h"
#include "ervp_common_peri_group_memorymap.h"
#include "ervp_core_peri_group_memorymap.h"
#include "ervp_variable_allocation.h"

typedef struct barrier_state
{
  unsigned int private_tag;
  int is_last;
} sBARRIER_STATE;

#ifdef INCLUDE_MULTICORE

#ifdef USE_HW_BARRIER

static inline unsigned int get_global_tag(int barrier_index)
{
	unsigned int addr = MMAP_CPG_MISC_GLOBAL_TAG;
	unsigned int global_tag_list = REG32(addr);
	global_tag_list = global_tag_list >> barrier_index;
	global_tag_list = global_tag_list & 1;
	return global_tag_list;
}

static inline void toggle_global_tag(int barrier_index)
{
	unsigned int addr = MMAP_CPG_MISC_GLOBAL_TAG;
	unsigned int value = (1<<barrier_index);
	init_auto_id(AUTO_ID_INDEX_FOR_HW_BARRIER+barrier_index);
	REG32(addr) = value;
}

static inline unsigned int get_rank_for_barrier(int barrier_index)
{
	return get_auto_id(AUTO_ID_INDEX_FOR_HW_BARRIER+barrier_index);
}

#endif // USE_HW_BARRIER

#ifdef USE_HW_BARRIER
#ifdef INCLUDE_CORE_PERI_GROUP

static inline void set_private_tag(int barrier_index)
{
	unsigned int addr = MMAP_MULTICORE_SUPPORTER_SET_PRIVATE_TAG;
	unsigned int value = (1<<barrier_index);
	REG32(addr) = value;
}

static inline unsigned int get_global_tag_read_only(int barrier_index)
{
	set_private_tag(barrier_index);
	return 0;
}

static inline unsigned int is_barrier_blocked(int barrier_index)
{
	unsigned int addr = MMAP_MULTICORE_SUPPORTER_WAIT_BARRIER;
	return REG32(addr);
}

static inline void wait_barrier(int barrier_index, unsigned int private_tag)
{
	while(is_barrier_blocked(barrier_index)!=0);
}

#else // not INCLUDE_CORE_PERI_GROUP

static inline unsigned int get_global_tag_read_only(int barrier_index)
{
	return get_global_tag(barrier_index);
}

static inline void wait_barrier(int barrier_index, unsigned int private_tag)
{
	while(get_global_tag_read_only(barrier_index)==private_tag);
}

#endif // INCLUDE_CORE_PERI_GROUP
#endif // USE_HW_BARRIER

#ifdef USE_SW_BARRIER

volatile static int global_tag[NUM_BARRIER] NOTCACHED_DATA = {0};
volatile static int num_reached[NUM_BARRIER] NOTCACHED_DATA = {0};

static inline unsigned int get_global_tag(int barrier_index)
{
	return global_tag[barrier_index];
}

static inline unsigned int get_global_tag_read_only(int barrier_index)
{
	return get_global_tag(barrier_index);
}

static inline void toggle_global_tag(int barrier_index)
{
	num_reached[barrier_index] = 0;
	global_tag[barrier_index] = 1 - global_tag[barrier_index];
}

static inline unsigned int get_rank_for_barrier(int barrier_index)
{
	unsigned int rank;
	acquire_lock(LOCK_INDEX_FOR_SW_BARRIER);
	rank = num_reached[barrier_index];
	num_reached[barrier_index] = rank+1;
	release_lock(LOCK_INDEX_FOR_SW_BARRIER);
	return rank;
}

static inline void wait_barrier(int barrier_index, unsigned int private_tag)
{
	while(get_global_tag_read_only(barrier_index)==private_tag);
}

#endif // USE_SW_BARRIER

// for both HW and SW

static inline sBARRIER_STATE reach_barrier(int barrier_index, int num_core)
{
  sBARRIER_STATE state;
  state.private_tag = get_global_tag_read_only(barrier_index);
  state.is_last = (get_rank_for_barrier(barrier_index)==(num_core-1))? 1 : 0;
	return state;
}

static inline void pass_barrier(int barrier_index, int num_core, sBARRIER_STATE state)
{
	if(state.is_last)
		toggle_global_tag(barrier_index);
	else
	{
		// never reach if called by release_unique_grant
		wait_barrier(barrier_index, state.private_tag);
	}
}

void reach_and_pass_barrier(int barrier_index, int num_core)
{
	sBARRIER_STATE state;
	state = reach_barrier(barrier_index, num_core);
	pass_barrier(barrier_index, num_core, state);
}

int request_unique_grant(int barrier_index, int num_core)
{
	sBARRIER_STATE state;
	state = reach_barrier(barrier_index, num_core);
  pass_barrier(barrier_index, num_core, state);
	return state.is_last;
}

void release_unique_grant(int barrier_index, int num_core)
{
  toggle_global_tag(barrier_index); // pass_barrier;
}

#endif // INCLUDE_MULTICORE
