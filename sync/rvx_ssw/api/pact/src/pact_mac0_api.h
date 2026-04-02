#ifndef __PACT_MAC0_API_H__
#define __PACT_MAC0_API_H__

#include "ervp_matrix.h"

void pact_matrix_add(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
void pact_matrix_sub(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
void pact_matrix_ewmult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
void pact_matrix_mult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);

#endif
