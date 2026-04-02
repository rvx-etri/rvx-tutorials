//#include <stdio.h>
//#include <ctype.h>

#include "ervp_stdlib.h"

int atoi_rvx(const char *str)
{
  int result = 0;
  int sign = 1;

  // Skip leading whitespaces
  while (isspace(*str))
  {
    str++;
  }

  // Handle sign
  if (*str == '-' || *str == '+')
  {
    if (*str == '-')
    {
      sign = -1;
    }
    str++;
  }

  // Process integer part
  while (isdigit(*str))
  {
    result = result * 10 + (*str - '0');
    str++;
  }

  return sign * result;
}

double atof_rvx(const char *str)
{
  double result = 0.0;
  double fraction = 1.0;
  int sign = 1;
  int exponent = 0;
  int exp_sign = 1;

  // Skip leading whitespaces
  while (isspace(*str))
  {
    str++;
  }

  // Handle sign
  if (*str == '-' || *str == '+')
  {
    if (*str == '-')
    {
      sign = -1;
    }
    str++;
  }

  // Process integer part
  while (isdigit(*str))
  {
    result = result * 10.0 + (*str - '0');
    str++;
  }

  // Process fractional part
  if (*str == '.')
  {
    str++;
    while (isdigit(*str))
    {
      fraction *= 0.1;
      result += (*str - '0') * fraction;
      str++;
    }
  }

  // Process scientific notation (e.g., 1.23e4 or 5.67E-2)
  if (*str == 'e' || *str == 'E')
  {
    str++;
    if (*str == '-' || *str == '+')
    {
      if (*str == '-')
      {
        exp_sign = -1;
      }
      str++;
    }
    while (isdigit(*str))
    {
      exponent = exponent * 10 + (*str - '0');
      str++;
    }

    // Apply exponent
    while (exponent > 0)
    {
      if (exp_sign == 1)
      {
        result *= 10.0;
      }
      else
      {
        result *= 0.1;
      }
      exponent--;
    }
  }

  return sign * result;
}