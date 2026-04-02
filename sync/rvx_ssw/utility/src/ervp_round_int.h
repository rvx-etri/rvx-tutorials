#ifndef __ERVP_ROUND_INT_H__
#define __ERVP_ROUND_INT_H__

static inline int round_up_int(int value, int step)
{
  int result;
  int residue;
  result = value;
  if(step>1)
  {
    residue = value % step;
    if(residue!=0)
      result += (step-residue);
  }
  return result;
};

static inline int round_down_int(int value, int step)
{
  int result;
  int residue;
  result = value;
  if(step>1)
  {
    residue = value % step;
    result -= residue;
  }
  return result;
};

static inline int round_up_power2(unsigned int value)
{
  int result = 1;
  while(result < value)
    result <<= 1;
  return result;
};

static inline int round_down_power2(unsigned int value)
{
  int result = round_up_power2(value);
  if(result!=value)
    result >>=1;
  return result;
};

static inline int rshift_ru(unsigned int value, int amount)
{
  return ((value-1)>>amount)+1;
};

#endif
