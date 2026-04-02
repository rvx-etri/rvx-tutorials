#ifndef __ERVP_MATH_H__
#define __ERVP_MATH_H__

unsigned int math_exp_uint(unsigned int base, unsigned int power);
unsigned int math_root_uint(unsigned int radicand, unsigned int index);
float apmath_root(float radicand, float index);

static inline int math_div_by_shift(const int value, unsigned int rshift)
{
  int result = value;
  if (rshift > 0)
  {
    const int is_minus = (value < 0);
    if (is_minus)
      result = -result;
    result = result >> rshift;
    if (is_minus)
      result = -result;
  }
  return result;
}

static inline unsigned int math_gcd(int a, int b)
{
  while (b != 0)
  {
    int temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

static inline unsigned int math_lcm(unsigned int a, unsigned int b)
{
  return ((unsigned int)(a / math_gcd(a, b))) * b; // divide first due to overflow
}

#endif
