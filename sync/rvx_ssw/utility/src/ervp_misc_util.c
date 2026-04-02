#include "ervp_misc_util.h"

void convert_value_to_bitvector(unsigned int value, char* bitvector)
{
	int i;
	for(i=0; i<32; i++)
	{
		bitvector[i] = '0' + ((value>>(32-1-i)) & 1);
	}
	bitvector[32] = 0;
}