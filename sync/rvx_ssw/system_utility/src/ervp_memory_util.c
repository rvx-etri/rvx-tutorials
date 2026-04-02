#include "platform_info.h"

#include "ervp_printf.h"
#include "ervp_mmio_util.h"
#include "ervp_round_int.h"
#include "ervp_assert.h"
#include "ervp_core_id.h"

void print_memory_byte(uint8_t *array, int num)
{
	int i;
	for (i = 0; i < num; i++)
	{
		if ((i & 7) == 0)
			printf("\n");
		printf("0x%02x ", array[i]);
	}
}

void print_memory_4byte(uint32_t *array, int num)
{
	int i;
	assert_pointer_align(array,4);
	for (i = 0; i < num; i++)
	{
		if ((i & 3) == 0)
			printf("\n");
		printf("0x%08x ", (unsigned int)(array[i]));
	}
}

static void *memset_1byte(void *dst, int value, int num)
{
	unsigned int dst_addr = (unsigned int)dst;
	const unsigned int dst_addr_end = ((unsigned int)dst) + num;
	const unsigned char value_1byte = (value & 0xFF);
	while (dst_addr < dst_addr_end)
	{
		REG8(dst_addr) = value_1byte;
		dst_addr += 1;
	}
	return dst;
}

static void *memset_4byte(void *dst, uint32_t value, int num)
{
	assert_pointer_align(dst,4);
	unsigned int dst_addr = (unsigned int)dst;
	const unsigned int dst_addr_end = ((unsigned int)dst) + (num << 2);
	const uint8_t value_1byte = (value & 0xFF);
	const uint32_t value_4byte = (value_1byte << 24) + (value_1byte << 16) + (value_1byte << 8) + (value_1byte);
	while (dst_addr < dst_addr_end)
	{
		REG32(dst_addr) = value_4byte;
		dst_addr += 4;
	}
	return dst;
}

void *memset_rvx(void *dst, int value, size_t num)
{
	unsigned int dst_addr = (unsigned int)dst;
	unsigned int num_prolog, num_body;

	// prologue
	num_prolog = 4 - (dst_addr & 0x3);
	if (num_prolog != 4)
	{
		memset_1byte((void *)dst_addr, value, num_prolog);
		dst_addr += num_prolog;
		num -= num_prolog;
	}
	// body
	num_body = num >> 2;
	memset_4byte((void *)dst_addr, value, num_body);
	dst_addr += (num_body << 2);
	num -= (num_body << 2);
	// epilogue
	memset_1byte((void *)dst_addr, value, num);
	return dst;
}

void *memcpy_1byte(void *dst, const void *src, int num)
{
	unsigned int src_addr = (unsigned int)src;
	unsigned int dst_addr = (unsigned int)dst;
	const unsigned int dst_addr_end = ((unsigned int)dst) + num;
	while (dst_addr < dst_addr_end)
	{
		REG8(dst_addr) = REG8(src_addr);
		src_addr += 1;
		dst_addr += 1;
	}
	return dst;
}

static void *memcpy_4byte(void *dst, const void *src, int num)
{
	unsigned int src_addr = (unsigned int)src;
	unsigned int dst_addr = (unsigned int)dst;
	const unsigned int dst_addr_end = ((unsigned int)dst) + (num << 2);
	while (dst_addr < dst_addr_end)
	{
		REG32(dst_addr) = REG32(src_addr);
		src_addr += 4;
		dst_addr += 4;
	}
	return dst;
}

void *memcpy_rvx(void *dst, const void *src, size_t num)
{
	unsigned int src_addr = (unsigned int)src;
	unsigned int dst_addr = (unsigned int)dst;
	unsigned int num_prolog, num_body;
	if (((src_addr ^ dst_addr) & 0x3) == 0)
	{
		// prologue
		num_prolog = 4 - (dst_addr & 0x3);
		if (num_prolog != 4)
		{
			memcpy_1byte((void *)dst_addr, (void *)src_addr, num_prolog);
			dst_addr += num_prolog;
			src_addr += num_prolog;
			num -= num_prolog;
		}
		// body
		num_body = num >> 2;
		memcpy_4byte((void *)dst_addr, (void *)src_addr, num_body);
		dst_addr += (num_body << 2);
		src_addr += (num_body << 2);
		num -= (num_body << 2);
		// epilogue
		memcpy_1byte((void *)dst_addr, (void *)src_addr, num);
	}
	else
		memcpy_1byte(dst, src, num);
	return dst;
}

static int _memory_compare_byte(const void *result, const void *ref, size_t num, int prints)
{
	int all_are_equal = 1;
	const uint8_t *result_temp = (const uint8_t *)result;
	const uint8_t *ref_temp = (const uint8_t *)ref;
	for (int i = 0; i < num; i++)
	{
		uint8_t a, b;
		a = *result_temp;
		b = *ref_temp;
		if (a != b)
		{
			all_are_equal = 0;
			if (prints)
				printf("\n[0x%08x] 0x%02x 0x%02x", (const unsigned int)result_temp, a, b);
			break;
		}
		result_temp++;
		ref_temp++;
	}
	return all_are_equal;
}

static inline int _memory_compare_4byte(const void *result, const void *ref, size_t num, int prints)
{
	int all_are_equal = 1;
	const uint32_t *result_temp = (const uint32_t *)result;
	const uint32_t *ref_temp = (const uint32_t *)ref;
	for (int i = 0; i < (num >> 2); i++)
	{
		uint32_t a, b;
		a = *result_temp;
		b = *ref_temp;
		if (a != b)
		{
			all_are_equal = 0;
			if (prints)
				printf("\n[0x%08x] 0x%08x 0x%08x", (const unsigned int)result_temp, (unsigned int)a, (unsigned int)b);
			break;
		}
		result_temp++;
		ref_temp++;
	}
	return all_are_equal;
}

int memory_compare(const void *result, const void *ref, size_t num, int prints)
{
	const int num_prolog_result = (4 - (((unsigned int)(result)) & 3)) & 3;
	const int num_prolog_ref = (4 - (((unsigned int)(ref)) & 3)) & 3;
	int all_are_equal = 1;
	if (num_prolog_result != num_prolog_ref)
		all_are_equal &= _memory_compare_byte(result, ref, num, prints);
	else
	{
		const uint8_t *result_temp = (const uint8_t *)result;
		const uint8_t *ref_temp = (const uint8_t *)ref;
		const int num_prolog = num_prolog_result;
		const int num_epilog = (num - num_prolog) & 3;
		const int num_body = num - num_prolog - num_epilog;
		// prolog
		if (num_prolog > 0)
		{
			all_are_equal &= _memory_compare_byte(result_temp, ref_temp, num_prolog, prints);
			result_temp += num_prolog;
			ref_temp += num_prolog;
		}
		// body
		all_are_equal &= _memory_compare_4byte(result_temp, ref_temp, num_body, prints);
		result_temp += num_body;
		ref_temp += num_body;
		// epilog
		all_are_equal &= _memory_compare_byte(result_temp, ref_temp, num_epilog, prints);
	}
	if (prints)
	{
		if (all_are_equal)
			printf("\nall correct");
		else
			printf("\nsome incorrect");
	}
	return all_are_equal;
}