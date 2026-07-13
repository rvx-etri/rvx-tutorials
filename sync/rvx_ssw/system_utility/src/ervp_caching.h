#ifndef __ERVP_CACHING_H__
#define __ERVP_CACHING_H__

#include "platform_info.h"
#include "core_dependent.h"

extern unsigned int _cacheable_start;
extern unsigned int _cacheable_last;

void register_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last);
void print_cacheable_region();

#if defined(CACHING_NONE)

static inline int is_cacheable_region(unsigned int addr)
{
  return 0;
}

#elif defined(CACHING_SAFE)

static inline int is_cacheable_region(unsigned int addr)
{
  return (addr >= _cacheable_start) && (addr <= _cacheable_last);
}

#elif defined(CACHING_MOST) || defined(CACHING_ALL)

static inline int is_cacheable_region(unsigned int addr)
{
  return (addr >= FIXED_CACHEABLE_START) && (addr <= FIXED_CACHEABLE_LAST);
}

#endif

#endif
