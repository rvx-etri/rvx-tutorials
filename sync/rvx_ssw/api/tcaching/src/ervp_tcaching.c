#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_tcu.h"
#include "ervp_tcaching.h"
#include "ervp_core_id.h"
#include "ervp_round_int.h"
#include "core_dependent.h"
#include "ervp_multicore_synch.h"

#define TCACHING_ADDR_OFFSET_MASK     (CACHE_LINE_SIZE-1)
#define TCACHING_NUM_ENTRY			(8)

static unsigned int __tcaching_heap_addr[NUM_CORE_USER] = {0};

static void tcaching_heap_init()
{
	__tcaching_heap_addr[EXCLUSIVE_ID] = TEMPORARY_CACHING_HEAP_START;
}

void tcaching_init()
{
	tcu_clear_all_region();
	tcaching_heap_init();
}

static int tcaching_get_usable_entry_index()
{
	int i;
	int usable_entry_index = -1;
	unsigned int valid_list = tcu_get_region_valid_list();
	
	for(i = 0; i < TCACHING_NUM_ENTRY; i++)
	{
		unsigned int valid = (valid_list >> i) & 1;
		if(valid == 0)
		{
			usable_entry_index = i;
			break;
		}
	}
	return usable_entry_index;
}

static unsigned int tcaching_alloc_heap(size_t size)
{
	unsigned int result = 0;
	if((__tcaching_heap_addr[EXCLUSIVE_ID]-1+size) <= TEMPORARY_CACHING_HEAP_LAST)
	{
		result = __tcaching_heap_addr[EXCLUSIVE_ID];
		__tcaching_heap_addr[EXCLUSIVE_ID] += size;
		__tcaching_heap_addr[EXCLUSIVE_ID] = round_up_int(__tcaching_heap_addr[EXCLUSIVE_ID], CACHE_LINE_SIZE);
		//__tcaching_heap_addr[EXCLUSIVE_ID] = align_cache_line(__tcaching_heap_addr[EXCLUSIVE_ID]);
	}
	else
	{
		printf("The tc heap is full and cannot be allocated.");
		while(1);
	}
	return result;
}

void *tcaching_malloc(void *pt, size_t size)
{
	unsigned int tcaching_addr;
	unsigned int vstart_addr;
	unsigned int vlast_addr;
	unsigned int vstart_addr_offset;
	int offset;
	int tcaching_id;

	vstart_addr_offset = ((unsigned int)pt & TCACHING_ADDR_OFFSET_MASK);
	tcaching_addr = tcaching_alloc_heap(size + vstart_addr_offset);

	tcaching_id = tcaching_get_usable_entry_index();
	if(tcaching_id == -1)
	{
		printf("There is no usable tc entry!\n");
		while(1);
	}

	vstart_addr = tcaching_addr + vstart_addr_offset;
	vlast_addr = vstart_addr + (unsigned int)size - 1;
	offset = (unsigned int)pt - vstart_addr;

	tcu_set_region_start(tcaching_id, vstart_addr);
	tcu_set_region_last(tcaching_id, vlast_addr);
	tcu_set_region_offset(tcaching_id, offset);
	tcu_set_region_valid(tcaching_id, 1);

#if 0
	printf("0x%x\n",tcu_get_region_start(tcaching_id));
	printf("0x%x\n",tcu_get_region_last(tcaching_id));
	printf("0x%x\n",tcu_get_region_offset(tcaching_id));
	printf("0x%x\n",tcu_get_region_valid_list(tcaching_id));
#endif
#if 0
	printf("size: 0x%08x\n", size);
	printf("pt: 0x%08x\n", pt);
	printf("vs: 0x%08x\n", vstart_addr);
	printf("reg vs: 0x%08x\n", tcu_get_region_start(tcaching_id));
	printf("vl: 0x%08x\n", vlast_addr);
	printf("cs: 0x%08x\n", offset);
	printf("__tcaching_heap_addr[EXCLUSIVE_ID]: 0x%08x\n", __tcaching_heap_addr[EXCLUSIVE_ID]);
#endif

	return (void*)vstart_addr;
}

void tcaching_free(void *tcaching_pt)
{
	int i;
	unsigned int valid_list;
	int detected = 0;
	valid_list = tcu_get_region_valid_list();
	for(i = 0; i < TCACHING_NUM_ENTRY; i++)
  {
		unsigned int valid = (valid_list >> i) & 1;
		if(valid)
		{
			if((unsigned int)tcaching_pt == tcu_get_region_start(i))
			{
				detected = 1;
				//printf("temprary caching end / tcaching_pt: 0x%08x\n", tcaching_pt); 
				flush_cache();
				tcu_set_region_valid(i, 0);
				break;
			}
		}
	}
	while(!detected);

	valid_list = tcu_get_region_valid_list();
	if(valid_list == 0)
	{
		tcaching_heap_init();
		//printf("temprary caching end / valid_list 0\n"); 
	}
}

void tcaching_flush()
{
	flush_cache();
	tcu_clear_all_region();
	tcaching_heap_init();
}
