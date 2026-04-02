#ifndef __ERVP_PSUEDO_RANDOM_H__
#define __ERVP_PSUEDO_RANDOM_H__

#include <stdint.h>

#define srand srand_rvx
void srand_rvx(uint32_t s);

#define rand_range rand_range_rvx
int32_t rand_range_rvx(int32_t min, int32_t max);

#endif