#include "ervp_float.h"
#include "ervp_printf.h"

const unsigned int FLOAT32_EXPONENT_MAX = 256-1;
const unsigned int FLOAT64_EXPONENT_MAX = 2048-1;

unsigned int get_float32_significand(float a)
{
  const unsigned int hidden = (1<<23);
  Float32Binary temp;
  unsigned int significand;
  temp.value = a;
  if(temp.br.exponent==FLOAT32_EXPONENT_MAX)
    significand = 0;
  else
  {
    significand = temp.br.mantissa;
    if(temp.br.exponent==0)
    {
      if(significand!=0)
        while(1)
        {
          significand <<= 1;
          if(significand>=hidden)
            break;
        }
    }
    else
      significand |= hidden;
  }
  return significand;
}

void print_float32_detail(float value)
{
  Float32Binary temp;
  temp.value = value;
  printf("\n%f 0x%08x", value, temp.hex);
  printf("\ne:%x m:%x", temp.br.exponent, temp.br.mantissa);
}

int is_float32_nan(float value)
{
  Float32Binary temp;
  temp.value = value;
  return (temp.br.exponent==FLOAT32_EXPONENT_MAX) && (temp.br.mantissa!=0);
}

int get_float32_similarity(float a, float b)
{
  Float32Binary a_bin, b_bin, diff;
  int result;
  unsigned int bigger_exponent;
  a_bin.value = a;
  b_bin.value = b;
  if(a_bin.hex==b_bin.hex)
    result = 32;
  else if(a==b)
    result = 32;
  else if(is_float32_nan(a) && is_float32_nan(b))
    result = 32;
  else if(a_bin.br.sign!=b_bin.br.sign)
    result = 0;
  else if(a_bin.br.exponent!=b_bin.br.exponent)
    result = 0;
  else
  {
    diff.value = a - b;
    bigger_exponent = (a_bin.br.exponent >= b_bin.br.exponent)? a_bin.br.exponent : b_bin.br.exponent;
    result = bigger_exponent - diff.br.exponent;
  }
  return result;
}

void print_float64_detail(double value)
{
  Float64Binary temp;
  temp.value = value;
  printf("\n%lf %16x", value, temp.hex);
  printf("\n%x %x", temp.br.exponent, temp.br.mantissa);
}

int is_float64_nan(float value)
{
  Float64Binary temp;
  temp.value = value;
  return (temp.br.exponent==FLOAT64_EXPONENT_MAX) && (temp.br.mantissa!=0);
}

int get_float64_similarity(double a, double b)
{
  Float64Binary a_bin, b_bin, diff;
  int result;
  a_bin.value = a;
  b_bin.value = b;
  if(a_bin.hex==b_bin.hex)
    result = 64;
  else if(a==b)
    result = 64;
  else if(a_bin.br.sign!=b_bin.br.sign)
    result = 0;
  else if(a_bin.br.exponent!=b_bin.br.exponent)
    result = 0;
  else
  {
    diff.value = a - b;
    result = a_bin.br.exponent - diff.br.exponent;
  }
  return result;
}
