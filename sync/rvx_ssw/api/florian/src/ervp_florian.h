#ifndef __ERVP_FLORIAN_H__
#define __ERVP_FLORIAN_H__

#include "platform_info.h"

#if defined(INCLUDE_FLORIAN)

float florian_convert_si32_f32 (int i);
float florian_convert_ui32_f32 (unsigned int i);

float florian_addsf3 (float a, float b);
float florian_subsf3 (float a, float b);
float florian_mulsf3 (float a, float b);
float florian_divsf3 (float a, float b);

#endif // defined(INCLUDE_FLORIAN)

#if defined(INCLUDE_FLORIAN_DP)

double florian_convert_si32_f64 (int i);
double florian_convert_ui32_f64 (unsigned int i);

#endif // defined(INCLUDE_FLORIAN_DP)

// https://gcc.gnu.org/onlinedocs/gccint/Soft-float-library-routines.html

#if defined(USE_FLORIAN_SP) && defined(INCLUDE_FLORIAN)

float __floatsisf (int i);
float __floatunsisf (unsigned int i);

float __addsf3 (float a, float b);
float __subsf3 (float a, float b);
float __mulsf3 (float a, float b);
float __divsf3 (float a, float b);

#endif // defined(USE_FLORIAN_SP) && defined(INCLUDE_FLORIAN)

#if defined(USE_FLORIAN_DP) && defined(INCLUDE_FLORIAN_DP)

double __floatsidf (int i);
double __floatunsidf (unsigned int i);

#endif // defined(USE_FLORIAN_DP) && defined(INCLUDE_FLORIAN_DP)

#endif
