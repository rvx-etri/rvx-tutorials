#include "ervp_matrix_op_sw.h"
#include "ervp_special_matrix_op.h"
#include "ervp_math.h"

ervp_hwtask_busy_fx_t _matrix_add_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  int datatype = a->datatype;
  assert(datatype != MATRIX_DATATYPE_UINT32);
  assert(b->datatype != MATRIX_DATATYPE_UINT32);
  assert(c->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(a, b));
  assert(matrix_is_same_size(a, c));
  if (datatype == MATRIX_DATATYPE_FLOAT32)
    matrix_add_float_sw(a, b, c, option_value);
  else
    matrix_add_fixed_sw(a, b, c, option_value);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_sub_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  int datatype = a->datatype;
  assert(datatype != MATRIX_DATATYPE_UINT32);
  assert(b->datatype != MATRIX_DATATYPE_UINT32);
  assert(c->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(a, b));
  assert(matrix_is_same_size(a, c));
  if (datatype == MATRIX_DATATYPE_FLOAT32)
    matrix_sub_float_sw(a, b, c, option_value);
  else
    matrix_sub_fixed_sw(a, b, c, option_value);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_ewmult_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  int datatype = a->datatype;
  assert(datatype != MATRIX_DATATYPE_UINT32);
  assert(b->datatype != MATRIX_DATATYPE_UINT32);
  assert(c->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(a, b));
  assert(matrix_is_same_size(a, c));
  if (datatype == MATRIX_DATATYPE_FLOAT32)
    matrix_ewmult_float_sw(a, b, c, option_value);
  else
    matrix_ewmult_fixed_sw(a, b, c, option_value);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_mult_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  int datatype = a->datatype;
  assert(datatype != MATRIX_DATATYPE_UINT32);
  assert(b->datatype != MATRIX_DATATYPE_UINT32);
  assert(c->datatype != MATRIX_DATATYPE_UINT32);
  assert(a->num_col == b->num_row);
  assert(a->num_row == c->num_row);
  assert(b->num_col == c->num_col);
  if (datatype == MATRIX_DATATYPE_FLOAT32)
    matrix_mult_float_sw(a, b, c, option_value);
  else
    matrix_mult_fixed_sw(a, b, c, option_value);
  return NULL;
}

static ervp_hwtask_busy_fx_t __matrix_copy_part_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  assert(_mop_option_check(c, option_value));
  assert(mop_option_is_acc(option_value) == 0);
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  for (int i = 0; i < num_row; i++)
    for (int j = 0; j < num_col; j++)
    {
      UNKNOWN_TYPE data = _matrix_read_element(a, i, j);
      if (mop_option_has_postprocess(option_value))
        data.value_signed = _melement_perform_rshift_and_clip(data.value_signed, mop_option.br.rshift, mop_option.br.performs_cliping, c->datatype);
      _matrix_write_element(c, i, j, data);
    }
  c->is_binary = a->is_binary;
  mop_option_free(mop_option);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_copy_part_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  _matrix_check_copy_part(a, c, num_row, num_col, option_value);
  return __matrix_copy_part_sw(a, c, num_row, num_col, option_value);
}

static ervp_hwtask_busy_fx_t __matrix_transpose_part_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  assert(_mop_option_check(c, option_value));
  assert(mop_option_is_acc(option_value) == 0);
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  for (int i = 0; i < num_row; i++)
    for (int j = 0; j < num_col; j++)
    {
      UNKNOWN_TYPE data = _matrix_read_element(a, i, j);
      if (mop_option_has_postprocess(option_value))
        data.value_signed = _melement_perform_rshift_and_clip(data.value_signed, mop_option.br.rshift, mop_option.br.performs_cliping, c->datatype);
      _matrix_write_element(c, j, i, data);
    }
  c->is_binary = a->is_binary;
  mop_option_free(mop_option);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_transpose_part_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  _matrix_check_transpose_part(a, c, num_row, num_col, option_value);
  return __matrix_transpose_part_sw(a, c, num_row, num_col, option_value);
}

ervp_hwtask_busy_fx_t _matrix_conv_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value)
{
  assert(input_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(kernel_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(output_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_conv_check_size(input_info, kernel_info, output_info, conv_option_value));
  int is_float = matrix_datatype_is_float(input_info->datatype);
  assert(is_float == matrix_datatype_is_float(output_info->datatype));

  if (is_float)
    matrix_conv_float_sw(input_info, kernel_info, output_info, conv_option_value);
  else
    matrix_conv_fixed_sw(input_info, kernel_info, output_info, conv_option_value);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_shift_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int shamount, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(!matrix_datatype_is_float(a->datatype));
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  for (int i = 0; i < a->num_row; i++)
  {
    for (int j = 0; j < a->num_col; j++)
    {
      int result = matrix_read_fixed_element(a, i, j);
      if (shamount >= 0)
        result = result << shamount;
      else
        result = result >> (-shamount);
      result = _melement_perform_rshift_and_clip(result, mop_option.br.rshift, mop_option.br.performs_cliping, c->datatype);
      if (mop_option.br.acc)
        result += matrix_read_fixed_element(c, i, j);
      matrix_write_fixed_element(c, i, j, result);
    }
  }
  c->is_binary = 0;
  mop_option_free(mop_option);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_reshape_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(a != NULL);
  assert(c != NULL);
  assert(matrix_num_elements(a) == matrix_num_elements(c));
  assert(matrix_datatype_is_float(a->datatype) == matrix_datatype_is_float(c->datatype));
  assert(_mop_option_check(c, option_value));

  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  int m = 0;
  int n = 0;
  for (int i = 0; i < a->num_row; i++)
    for (int j = 0; j < a->num_col; j++)
    {
      UNKNOWN_TYPE data = _matrix_read_element(a, i, j);
      if (!matrix_datatype_is_float(a->datatype) && option_value)
        if ((!matrix_datatype_is_float(a->datatype)) && option_value)
          data.value_signed = _melement_perform_rshift_and_clip(data.value_signed, mop_option.br.rshift, mop_option.br.performs_cliping, c->datatype);
      _matrix_write_element(c, m, n, data);
      n++;
      if (n == c->num_col)
      {
        m++;
        n = 0;
      }
    }
  c->is_binary = a->is_binary;
  mop_option_free(mop_option);
  return NULL;
}

ervp_hwtask_busy_fx_t _matrix_downsample_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value)
{
  assert(input_info != NULL);
  assert(output_info != NULL);
  int is_float = matrix_datatype_is_float(input_info->datatype);
  assert(is_float == matrix_datatype_is_float(output_info->datatype));
  assert(matrix_downsample_check_size(input_info, output_info, downsample_option_value));

  if (is_float)
    _matrix_downsample_float_sw(mop_mapping, input_info, output_info, downsample_option_value);
  else
    _matrix_downsample_fixed_sw(mop_mapping, input_info, output_info, downsample_option_value);
  return NULL;
}