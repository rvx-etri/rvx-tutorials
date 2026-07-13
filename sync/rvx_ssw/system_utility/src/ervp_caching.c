#include "ervp_caching.h"
#include "ervp_printf.h"

unsigned int _cacheable_start = 0;
unsigned int _cacheable_last = 0;

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