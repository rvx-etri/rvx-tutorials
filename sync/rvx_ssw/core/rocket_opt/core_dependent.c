#include "platform_info.h"
#include "core_dependent.h"
#include "ervp_caching.h"

void init_cache(unsigned int cacheable_start, unsigned int cacheable_last)
{
	register_cacheable_region(0, 0x10000000, 0xC0000000-1);
}

void set_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last)
{
}