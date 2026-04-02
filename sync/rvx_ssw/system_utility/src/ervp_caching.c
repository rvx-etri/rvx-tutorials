#include "ervp_caching.h"
#include "ervp_printf.h"

#include <stdarg.h>

static unsigned int _cacheable_start = 0;
static unsigned int _cacheable_last = 0;

void cache_flush_smart(int region, ...)
{
#if defined(CACHING_NONE) || defined(CACHING_SAFE)
  va_list args;
  va_start(args, region);

  int cacheable = 1;
  for (int i = 0; i < region; i++)
  {
    unsigned int addr = va_arg(args, unsigned int);
    cacheable = check_cacheable(addr);
    // debug_printd(cacheable);
    // debug_printx(_cacheable_start);
    // debug_printx(_cacheable_last);
    // debug_printx(addr);
    if (cacheable)
      break;
  }

  va_end(args);
  if (cacheable)
    flush_cache();
#endif
#if defined(CACHING_MOST) || defined(CACHING_ALL)
  flush_cache();
#endif
}

void register_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last)
{
  _cacheable_start = cacheable_start;
  _cacheable_last = cacheable_last;
}

void print_cacheable_region()
{
  debug_printx(_cacheable_start);
  debug_printx(_cacheable_last);
}

int check_cacheable(unsigned int addr)
{
  int cacheable = 1;
  if (addr < _cacheable_start)
    cacheable = 0;
  else if (addr > _cacheable_last)
    cacheable = 0;
  return cacheable;
}