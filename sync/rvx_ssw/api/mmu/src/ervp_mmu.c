#include "ervp_uart.h"
#include "ervp_printf.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_multicore_synch.h"
#include "ervp_mmio_util.h"
#include "ervp_core_id.h"

#include "core_dependent.h"
#include "platform_info.h"

#define BW_ADDR			32
#define NUM_PART		16
#define NUM_PAGE		8
#define BW_PART (BW_ADDR/NUM_PART)

#define BW_PAGE_INDEX 3

#define MIN_PAGE_SIZE pow2(BW_PART)
#define MAX_PAGE_SIZE pow2(BW_ADDR-BW_PART)

#define GET_MMU_REG_ADDR(page_id, reg_offset) (CORE_SUPPORTER_BASEADDR + (page_id<<(BW_MMAP_SUBOFFSET_MMU)) + (reg_offset))

static inline int pow2(int x)
{
	int i;
	int result = 1;
	for(i=0; i<x; i++)
		result <<= 1;
	return result;
}

static inline int log2(int x)
{
	int result = 0;
	while(1)
	{
		if(x<2) break;
		x >>= 1;
		result++;
	}
	return result;
}

static inline int get_lower_page_size(int page_size)
{
	return (page_size >> BW_PART);
}

static inline int get_upper_page_size(int page_size)
{
	return (page_size << BW_PART);
}

static int get_max_smaller_page_size(int memory_size)
{
	int page_size;
	page_size = MAX_PAGE_SIZE;
	while(page_size > memory_size)
		page_size = get_lower_page_size(page_size);
	return page_size;
}

static int get_min_bigger_page_size(int memory_size)
{
	int page_size;
	page_size = MIN_PAGE_SIZE;
	while(page_size < memory_size)
		page_size = get_upper_page_size(page_size);
	return page_size;
}

int quantize_to_page_size(int memory_size, int num_page)
{
	int page_size;
	int lower_page_size;
	if(num_page<0)
		page_size = 0;
	else
	{
		page_size = get_min_bigger_page_size(memory_size);
		if(memory_size==page_size)
			;
		else if(num_page>1)
		{
			lower_page_size = get_lower_page_size(page_size);
			if((num_page*lower_page_size) >= memory_size)
				page_size = lower_page_size + quantize_to_page_size(memory_size-lower_page_size,num_page-1);
		}
	}
	return page_size; 
}

static unsigned int get_valid_part(int page_size)
{
	unsigned int valid_part = (unsigned int)(-1);
	int size = 1;
	while(size<page_size)
	{
		size <<= BW_PART;
		valid_part = (valid_part<<1) & (-2);
	}
	return valid_part;
}

static void set_page_table_reg(int page_id, unsigned int base_addr, unsigned int valid_part, unsigned int target_addr)
{
	REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_BASEADDR)) = base_addr;
	REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_VALID_PART)) = valid_part;
	REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_TARGETADDR)) = target_addr;
}

void print_mmu(int page_id)
{
	unsigned int base_addr;
	unsigned int valid_part;
	unsigned int target_addr;

	base_addr = REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_BASEADDR));
	valid_part = REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_VALID_PART));
	target_addr = REG32(GET_MMU_REG_ADDR(page_id, MMAP_OFFSET_MMU_TARGETADDR));

	//printf("0x%x, 0x%x, 0x%x", base_addr, valid_part, target_addr);
}

static int set_page_table_general(int page_id, int num_page, int memory_size, 
				unsigned int base_addr, unsigned int target_addr)
{
	int allocated_memory_size;
	unsigned int valid_part;
	int page_size;

	//printf("@c:%d p: %d, np: %d, ms: 0x%x, ba: 0x%x, ta: 0x%x", page_id, num_page, memory_size, base_addr, target_addr);
	allocated_memory_size = quantize_to_page_size(memory_size, num_page);
	//printf("%x", allocated_memory_size);
	while(allocated_memory_size>0)
	{
		if(page_id>=NUM_PAGE)
		{
			allocated_memory_size = 0;
			break;
		}
		if(allocated_memory_size<MIN_PAGE_SIZE)
		{
			allocated_memory_size = 0;
			break;
		}

		page_size = get_max_smaller_page_size(allocated_memory_size);
		valid_part = get_valid_part(page_size);
		//printf("\npage_size: %x", page_size);
		set_page_table_reg(page_id, base_addr, valid_part, target_addr);
		//printf("\nc:%d p: %d, ba: 0x%x vp: %x, ta: %x", CORE_ID, page_id, base_addr, valid_part, target_addr);
		
		page_id++;
		allocated_memory_size -= page_size;
		base_addr += page_size;
		target_addr += page_size;
	}

	return allocated_memory_size;
}

int set_page_table(int memory_size, unsigned int base_addr, unsigned int target_addr)
{
	return set_page_table_general(0, NUM_PAGE, memory_size, base_addr, target_addr);
}
