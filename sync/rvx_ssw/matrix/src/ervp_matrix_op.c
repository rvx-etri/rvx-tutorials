#include <stddef.h>
#include "ervp_misc_util.h" // due to NULL_FTN_POINTER
#include "ervp_matrix_op.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_matrix_op_transform.h"
#include "ervp_special_matrix_op.h"
#include "ervp_misc_util.h"
#include "ervp_math.h"
#include "core_dependent.h"

int _melement_perform_rshift_and_clip(int value, int rshift, int performs_cliping, int datatype)
{
  int result = math_div_by_shift(value, rshift);
  if (performs_cliping)
  {
    int lower_bound = matrix_datatype_get_lower_bound(datatype);
    int upper_bound = matrix_datatype_get_upper_bound(datatype);
    if (result < lower_bound)
      result = lower_bound;
    if (result > upper_bound)
      result = upper_bound;
  }
  return result;
}

ErvpMatrixInfo *matrix_conv_alloc_output(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, unsigned int conv_option_value)
{
  ErvpMatrixInfo *result;
  int o_num_row = matrix_conv_output_rows(input_info->num_row, kernel_info->num_row, conv_option_value);
  int o_num_col = matrix_conv_output_cols(input_info->num_col, kernel_info->num_col, conv_option_value);
  int datatype;
  datatype = matrix_datatype_is_float(input_info->datatype) ? MATRIX_DATATYPE_FLOAT32 : MATRIX_DATATYPE_SINT32;
  result = matrix_alloc(datatype, o_num_row, o_num_col, NULL);
  return result;
}

int matrix_conv_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, const ErvpMatrixInfo *output_info, unsigned int conv_option_value)
{
  int o_num_row = matrix_conv_output_rows(input_info->num_row, kernel_info->num_row, conv_option_value);
  int o_num_col = matrix_conv_output_cols(input_info->num_col, kernel_info->num_col, conv_option_value);
  return (output_info->num_row == o_num_row) && (output_info->num_col == o_num_col);
}

unsigned int matrix_pad_gen_option(unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  ervp_mpad_option_t pad_option;
  pad_option.value = 0;
  if (conv_option.br.pad_amount > 0)
  {
    if (conv_option.br.pad_has_rowd)
      pad_option.br.num_rowd = conv_option.br.pad_amount;
    if (conv_option.br.pad_has_rowu)
      pad_option.br.num_rowu = conv_option.br.pad_amount;
    if (conv_option.br.pad_has_cold)
      pad_option.br.num_cold = conv_option.br.pad_amount;
    if (conv_option.br.pad_has_colu)
      pad_option.br.num_colu = conv_option.br.pad_amount;
    pad_option.br.mode = conv_option.br.pad_mode;
  }
  return pad_option.value;
}

ErvpMatrixInfo *matrix_pad_alloc_output(const ErvpMatrixInfo *input_info, unsigned int pad_option_value)
{
  ErvpMatrixInfo *result;
  int o_num_row = matrix_pad_output_rows(input_info->num_row, pad_option_value);
  int o_num_col = matrix_pad_output_cols(input_info->num_col, pad_option_value);
  result = matrix_alloc(input_info->datatype, o_num_row, o_num_col, NULL);
  return result;
}

int matrix_pad_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *output_info, unsigned int pad_option_value)
{
  int o_num_row = matrix_pad_output_rows(input_info->num_row, pad_option_value);
  int o_num_col = matrix_pad_output_cols(input_info->num_col, pad_option_value);
  return (output_info->num_row == o_num_row) && (output_info->num_col == o_num_col);
}

int matrix_downsample_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *output_info, unsigned int downsample_option_value)
{
  ervp_mdownsample_option_t downsample_option;
  downsample_option.value = downsample_option_value;
  const int stride = downsample_option.br.stride_m1 + 1;
  return ((input_info->num_row / stride) == output_info->num_row) && ((input_info->num_col / stride) == output_info->num_col);
}

ervp_mop_mapping_t *matrix_op_mapping_alloc()
{
  ervp_mop_mapping_t *mapping = calloc(sizeof(ervp_mop_mapping_t), 1);
  assert(mapping);

  mapping->matrix_copy_part = _matrix_copy_part_sw;
  mapping->matrix_transpose_part = _matrix_transpose_part_sw;
  mapping->matrix_reshape = _matrix_reshape_sw;

  mapping->matrix_add = _matrix_add_sw;
  mapping->matrix_sub = _matrix_sub_sw;
  mapping->matrix_ewmult = _matrix_ewmult_sw;
  mapping->matrix_mult = _matrix_mult_sw;
  mapping->matrix_conv = _matrix_conv_sw;
  mapping->matrix_conv_sharedinput = matrix_conv_sharedinput_tf;
  mapping->matrix_conv_sharedoutput = matrix_conv_sharedoutput_tf;
  mapping->matrix_downsample = _matrix_downsample_sw;

  mapping->matrix_scalar_mult_fixed = _matrix_scalar_mult_fixed_sw;
  mapping->matrix_scalar_mult_float = _matrix_scalar_mult_float_sw;
  mapping->matrix_shift_fixed = _matrix_shift_fixed_sw;
  mapping->matrix_pad = matrix_pad2copy_tf;

  mapping->matrix_fill_fixed = _matrix_fill_fixed_sw;
  mapping->matrix_fill_float = _matrix_fill_float_sw;
  mapping->matrix_identity = _matrix_identity_sw;

  mapping->matrix_copy = matrix_copy2copy_part_tf;
  mapping->matrix_transpose = matrix_transpose2transpose_part_tf;
  mapping->matrix_zero = matrix_zero2fill_tf;
  mapping->matrix_one = matrix_one2fill_tf;

  return mapping;
}