#ifndef __ERVP_MISC_UTIL_H__
#define __ERVP_MISC_UTIL_H__

#include <stddef.h>
#include "core_dependent.h"

#define NULL_FTN_POINTER ((void*)1)

#define static_assert(cond) switch(0) { case 0: case !!(long)(cond): ; }

static inline void delay_unit(unsigned int count) {
	unsigned int i;
	for(i=0; i<(count>>2); i++)
	{
		IDLE;
	}
}

void convert_value_to_bitvector(unsigned int value, char* bitvector);

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define CLIP(x, lower, upper) (((x) < (lower)) ? (lower) : (((x) > (upper)) ? (upper) : (x)))

#define CLIP_UINT08(x) CLIP(x,0,255)
#define CLIP_SINT08(x) CLIP(x,-128,127)

#define CLIP_UINT16(x) CLIP(x,0,65535)
#define CLIP_SINT16(x) CLIP(x,-65536,65535)

static inline int is_function_pointer_valid(void* p)
{
	int valid = 1;
	if(p==NULL)
		valid = 0;
	else if(p==NULL_FTN_POINTER)
		valid = 0;
	return valid;
}

#endif
