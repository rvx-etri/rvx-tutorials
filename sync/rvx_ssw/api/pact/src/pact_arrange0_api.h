#ifndef __PACT_ARRANGE0_API_H__
#define __PACT_ARRANGE0_API_H__

#include "ervp_matrix.h"

void pact_matrix_transpose(const ErvpMatrixInfo *a, ErvpMatrixInfo *c);
void pact_matrix_scalar_mult_int(const ErvpMatrixInfo *a, int value, ErvpMatrixInfo *c);
void pact_matrix_scalar_mult_float(const ErvpMatrixInfo *a, float value, ErvpMatrixInfo *c);

#endif
