#ifndef __ERVP_CACHING_H__
#define __ERVP_CACHING_H__

#include "platform_info.h"
#include "core_dependent.h"

void cache_flush_smart(int region, ...);
void register_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last);
void print_cacheable_region();
int check_cacheable(unsigned int addr);

#endif
