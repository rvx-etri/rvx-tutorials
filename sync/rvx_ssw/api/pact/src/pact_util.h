#ifndef __PACT_UTIL_H__
#define __PACT_UTIL_H__

#include "ervp_matrix.h"

#define GET_PARTED_NUM_ROW_COL(i_end, cur_i)  ( ((i_end-cur_i)>=PACT_MATRIX_SIZE)? PACT_MATRIX_SIZE: (i_end-cur_i) )

#endif
