#include <stdint.h>

#include "ervp_printf.h"

static uint32_t seed = 1;

__attribute__((weak)) void srand_rvx(uint32_t s)
{
  seed = s;
}

static int32_t rand_rvx_lcg()
{
  seed = seed * 1664525 + 1013904223;
  return (int32_t)seed;
}

static int32_t rand_rvx_xorshift()
{
  uint32_t x = seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  seed = x;
  return (int32_t)x;
}

// min ~ max 범위 난수
__attribute__((weak)) int32_t rand_range_rvx(int32_t min, int32_t max)
{
  int32_t r = rand_rvx_xorshift();
  if (max == min)
    return min;
  return min + (r % (max - min));
}