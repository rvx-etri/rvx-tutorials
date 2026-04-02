
#include "ervp_matrix_element.h"
#include "ervp_assert.h"
#include "ervp_printf.h"

#include "ervp_special_matrix_op.h"

static void _matrix_constant(ErvpMatrixInfo *result, const UNKNOWN_TYPE data)
{
  assert(result);
  for (int i = 0; i < result->num_row; i = i + 1)
    for (int j = 0; j < result->num_col; j = j + 1)
      _matrix_write_element(result, i, j, data);
}

ervp_hwtask_busy_fx_t _matrix_fill_fixed_sw(ervp_mop_mapping_t* mop_mapping, ErvpMatrixInfo *result, int32_t value)
{
  UNKNOWN_TYPE data;
  data.value_signed = value;
  _matrix_constant(result, data);
  result->is_binary = (value == 0) || (value == 1) || (value == (-1));
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_fill_float_sw(ervp_mop_mapping_t* mop_mapping, ErvpMatrixInfo *result, float value)
{
  UNKNOWN_TYPE data;
  data.value_f32 = value;
  _matrix_constant(result, data);
  result->is_binary = (value == 0) || (value == 1) || (value == (-1));
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_identity_sw(ervp_mop_mapping_t* mop_mapping, ErvpMatrixInfo *result)
{
  UNKNOWN_TYPE value;
  int min_size;

  assert(result);
  matrix_zero_sw(result);
  if (result->datatype == MATRIX_DATATYPE_FLOAT32)
    value.value_f32 = 1;
  else
    value.hex = 1;

  min_size = (result->num_row < result->num_col) ? result->num_row : result->num_col;
  for (int i = 0; i < min_size; i = i + 1)
    _matrix_write_element(result, i, i, value);
  result->is_binary = 1;
  return NULL;
}