#include "platform_info.h"
#include "core_dependent.h"
#include "ervp_caching.h"

volatile int cache_temp;

// 0xBE0 is NOT readable
static unsigned int _cacheable_start, _cacheable_last;

void init_cache(unsigned int cacheable_start, unsigned int cacheable_last)
{
#ifndef CACHING_NONE
#ifdef CACHING_SAFE
#else
	cacheable_start = FIXED_CACHEABLE_START;
	cacheable_last = FIXED_CACHEABLE_LAST;
#endif
#endif
	write_csr(0xBE0, cacheable_start);
	write_csr(0xBE8, cacheable_last);
	register_cacheable_region(0, cacheable_start, cacheable_last);
}

void set_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last)
{
	if (index == 0)
	{
		write_csr(0xBE1, cacheable_start);
		write_csr(0xBE9, cacheable_last);
	}
	else if (index == 1)
	{
		write_csr(0xBE2, cacheable_start);
		write_csr(0xBEA, cacheable_last);
	}
	else if (index == 2)
	{
		write_csr(0xBE3, cacheable_start);
		write_csr(0xBEB, cacheable_last);
	}
}