#include <string.h>
#include "ervp_matrix_op.h"

static inline void _matrix_conv_fixed_each_sw(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, int o_row_index, int o_col_index, ervp_mconv_option_t conv_option)
{
  int result = 0;
  int k_row_index, k_col_index;
  int i_row_index, i_col_index;
  int i_value;
  int k_value;

  const int row_index_dd = _matrix_conv_cal_start_row_index_of_input_matrix(kernel_info->num_row, o_row_index, conv_option);
  const int col_index_dd = _matrix_conv_cal_start_col_index_of_input_matrix(kernel_info->num_col, o_col_index, conv_option);

  for(k_row_index = 0; k_row_index < kernel_info->num_row; k_row_index++)
  {
    for(k_col_index = 0; k_col_index < kernel_info->num_col; k_col_index++)
    {
      int is_pad = 0;
      i_row_index = k_row_index + row_index_dd;
      i_col_index = k_col_index + col_index_dd;
      
      if(i_row_index < 0)
      {
        assert(conv_option.br.pad_has_rowd);
        assert(i_row_index >= (0- conv_option.br.pad_amount));
        i_row_index = 0;
        is_pad = 1;
      }
      else if(i_row_index >= input_info->num_row)
      {
        assert(conv_option.br.pad_has_rowu);
        assert(i_row_index < (input_info->num_row + conv_option.br.pad_amount));
        i_row_index = input_info->num_row - 1;
        is_pad = 1;
      }

      if(i_col_index < 0)
      {
        assert(conv_option.br.pad_has_cold);
        assert(i_col_index >= (0- conv_option.br.pad_amount));
        i_col_index = 0;
        is_pad = 1;
      }
      else if(i_col_index >= input_info->num_col)
      {
        assert(conv_option.br.pad_has_colu);
        assert(i_col_index < (input_info->num_col + conv_option.br.pad_amount));
        i_col_index = input_info->num_col - 1;
        is_pad = 1;
      }

      if(is_pad && (conv_option.br.pad_mode==PADMODE_ZEROS))
        ;
      else
      {
        i_value = matrix_read_fixed_element(input_info, i_row_index, i_col_index);
        k_value = matrix_read_fixed_element(kernel_info, k_row_index, k_col_index);
        result += i_value * k_value;
      }
    }
  }
  result = _melement_perform_rshift_and_clip(result,conv_option.br.rshift,conv_option.br.performs_cliping,output_info->datatype);
  if(conv_option.br.acc)
    result += matrix_read_fixed_element(output_info, o_row_index, o_col_index);
  matrix_write_fixed_element(output_info, o_row_index, o_col_index, result);
}

ervp_hwtask_busy_fx_t _matrix_conv_fixed_sw(ervp_mop_mapping_t* mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  int o_row_index, o_col_index;
  conv_option.value = conv_option_value;
  for(o_row_index = 0; o_row_index < output_info->num_row; o_row_index++)
  {
    for(o_col_index = 0; o_col_index < output_info->num_col; o_col_index++)
    {
      _matrix_conv_fixed_each_sw(input_info, kernel_info, output_info, o_row_index, o_col_index, conv_option);
    }
  }
  output_info->is_binary = 0;
  return NULL;
}

