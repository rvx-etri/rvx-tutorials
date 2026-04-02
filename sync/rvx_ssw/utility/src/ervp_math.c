#include "ervp_math.h"
#include "ervp_printf.h"

unsigned int math_exp_uint(unsigned int base, unsigned int power)
{
  unsigned int result = 1;
  for(int i=0; i<power; i++)
    result *= base;
  return result;
}

unsigned int math_root_uint(unsigned int radicand, unsigned int index)
{
  unsigned int lower_success;
	unsigned int upper_fail;
	if(radicand<=1)
		lower_success = radicand;
	else
	{
    lower_success = 1;
    upper_fail = radicand;
    while(1)
    {
      unsigned int candicate = (lower_success+upper_fail)>>1;
      if(lower_success>=candicate)
        break;
      unsigned int exp = math_exp_uint(candicate,index);
      if(exp<=radicand)
        lower_success = candicate;
      else
        upper_fail = candicate;
    }
	}
	return lower_success;
}

float apmath_root(float radicand , float index)
{
	int i;
	for(i=0; i<9; i++)
		index = (index+(radicand /index))/2;
	return index;
}