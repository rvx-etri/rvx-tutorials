#ifndef __ERVP_CORE_ID_H__
#define __ERVP_CORE_ID_H__

#include "ervp_reg_util.h"
#include "platform_info.h"

#include "core_dependent.h"
#include "ervp_core_peri_group_memorymap_offset.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_printf.h"
#include "ervp_assert.h"

static inline int get_process_id()
{
	int result;
#ifdef INCLUDE_MULTICORE
	result = REG32(CORE_PERI_GROUP_BASEADDR + MMAP_OFFSET_MULTICORE_SUPPORTER_PROCESS_ID);
#else
	assert(NUM_CORE_PHYSICAL == 1);
	result = 0;
#endif
	return result;
}

static inline int concat_id(int process_id, int local_id)
{
	int result = process_id;
	switch (NUM_CORE_PER_CORECLUSTER)
	{
	case 1:
		break;
	case 2:
		result <<= 1;
		result += local_id;
		break;
	case 4:
		result <<= 2;
		result += local_id;
		break;
	default:
		while (1)
			;
	}
	return result;
}

static inline int get_exclusive_id()
{
#ifdef VP_ENV
	return get_internal_core_id();
#else
	return concat_id(get_process_id(), get_local_id());
#endif
}

static inline int get_manager_id()
{
	int result;
	if (NUM_CORE_MANAGER == 1)
		result = 0;
	else
		result = get_exclusive_id();
	return result;
}

#define EXCLUSIVE_ID get_exclusive_id()
#define LOCAL_ID get_local_id()
#define PROCESS_ID get_process_id()
#define MANAGER_ID get_manager_id()

#define CORE_ID(pid, local_id) concat_id(pid, local_id)

static inline void print_core_id()
{
	printf("\n%d", EXCLUSIVE_ID);
}

#endif
