#ifndef __ERVP_SPECIAL_MATRIX_OP_H__
#define __ERVP_SPECIAL_MATRIX_OP_H__

#include "ervp_matrix_op.h"

// cal by sw
ervp_hwtask_busy_fx_t _matrix_fill_fixed_sw(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result, int32_t value);
ervp_hwtask_busy_fx_t _matrix_fill_float_sw(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result, float value);
ervp_hwtask_busy_fx_t _matrix_identity_sw(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);

static inline ervp_hwtask_busy_fx_t _matrix_zero_sw(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result)
{
  return _matrix_fill_fixed_sw(mop_mapping, result, 0);
}

static inline ervp_hwtask_busy_fx_t _matrix_one_sw(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result)
{
  return _matrix_fill_fixed_sw(mop_mapping, result, 1);
}

// redefine
static inline void matrix_fill_fixed_sw(ErvpMatrixInfo *result, int32_t value) { _matrix_fill_fixed_sw(NULL, result, value); }
static inline void matrix_fill_float_sw(ErvpMatrixInfo *result, float value) { _matrix_fill_float_sw(NULL, result, value); }
static inline void matrix_identity_sw(ErvpMatrixInfo *result) { _matrix_identity_sw(NULL, result); }
static inline void matrix_zero_sw(ErvpMatrixInfo *result) { _matrix_zero_sw(NULL, result); }
static inline void matrix_one_sw(ErvpMatrixInfo *result) { _matrix_one_sw(NULL, result); }

#endif
