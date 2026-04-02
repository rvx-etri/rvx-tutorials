#include <limits.h>
#include "ervp_assert.h"
#include "ervp_matrix_op.h"

static inline void _matrix_downsample_fixed_each_sw(const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, int o_row_index, int o_col_index, ervp_mdownsample_option_t downsample_option)
{
  int result = 0;

  const int row_index_dd = _matrix_downsample_cal_start_row_index_of_input_matrix(o_row_index, downsample_option);
  const int col_index_dd = _matrix_downsample_cal_start_col_index_of_input_matrix(o_col_index, downsample_option);
  const int stride = downsample_option.br.stride_m1 + 1;

  switch(downsample_option.br.downsample_mode)
  {
    case DOWNSAMPLE_TOPLEFT:
      result = matrix_read_fixed_element(input_info, row_index_dd, col_index_dd);
      break;
    case DOWNSAMPLE_MAX:
      {
        result = INT32_MIN;
        for(int k_row_index = 0; k_row_index < stride; k_row_index++)
        {
          for(int k_col_index = 0; k_col_index < stride; k_col_index++)
          {
            int is_pad = 0;
            int i_row_index = k_row_index + row_index_dd;
            int i_col_index = k_col_index + col_index_dd;

            if(i_row_index < 0)
            {
              assert(downsample_option.br.pad_has_rowd);
              assert(i_row_index >= (0- downsample_option.br.pad_amount));
              is_pad = 1;
            }
            else if(i_row_index >= input_info->num_row)
            {
              assert(downsample_option.br.pad_has_rowu);
              assert(i_row_index < (input_info->num_row + downsample_option.br.pad_amount));
              is_pad = 1;
            }

            if(i_col_index < 0)
            {
              assert(downsample_option.br.pad_has_cold);
              assert(i_col_index >= (0- downsample_option.br.pad_amount));
              is_pad = 1;
            }
            else if(i_col_index >= input_info->num_col)
            {
              assert(downsample_option.br.pad_has_colu);
              assert(i_col_index < (input_info->num_col + downsample_option.br.pad_amount));
              is_pad = 1;
            }

            if(!is_pad)
            {
              int temp = matrix_read_fixed_element(input_info, i_row_index, i_col_index);
              if(temp > result)
                result = temp;
            }
          }
        }
        matrix_write_fixed_element(output_info, o_row_index, o_col_index, result);
      }
      break;
    case DOWNSAMPLE_AVERAGE:
    case DOWNSAMPLE_SUM:
      {
        for(int k_row_index = 0; k_row_index < stride; k_row_index++)
        {
          for(int k_col_index = 0; k_col_index < stride; k_col_index++)
          {
            int is_pad = 0;
            int i_row_index = k_row_index + row_index_dd;
            int i_col_index = k_col_index + col_index_dd;

            if(i_row_index < 0)
            {
              assert(downsample_option.br.pad_has_rowd);
              assert(i_row_index >= (0- downsample_option.br.pad_amount));
              i_row_index = 0;
              is_pad = 1;
            }
            else if(i_row_index >= input_info->num_row)
            {
              assert(downsample_option.br.pad_has_rowu);
              assert(i_row_index < (input_info->num_row + downsample_option.br.pad_amount));
              i_row_index = input_info->num_row - 1;
              is_pad = 1;
            }

            if(i_col_index < 0)
            {
              assert(downsample_option.br.pad_has_cold);
              assert(i_col_index >= (0- downsample_option.br.pad_amount));
              i_col_index = 0;
              is_pad = 1;
            }
            else if(i_col_index >= input_info->num_col)
            {
              assert(downsample_option.br.pad_has_colu);
              assert(i_col_index < (input_info->num_col + downsample_option.br.pad_amount));
              i_col_index = input_info->num_col - 1;
              is_pad = 1;
            }

            int i_value = matrix_read_fixed_element(input_info, i_row_index, i_col_index);
            result += i_value;
          }
        }
        if(downsample_option.br.downsample_mode==DOWNSAMPLE_AVERAGE)
          result = result / (stride*stride);
        matrix_write_fixed_element(output_info, o_row_index, o_col_index, result);
      }
      break;
    default:
      assert(0);
  }
}

ervp_hwtask_busy_fx_t _matrix_downsample_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value)
{
  ervp_mdownsample_option_t downsample_option;
  downsample_option.value = downsample_option_value;
  for(int o_row_index = 0; o_row_index < output_info->num_row; o_row_index++)
  {
    for(int o_col_index = 0; o_col_index < output_info->num_col; o_col_index++)
    {
      _matrix_downsample_fixed_each_sw(input_info, output_info, o_row_index, o_col_index, downsample_option);
    }
  }
  switch(downsample_option.br.downsample_mode)
  {
    case DOWNSAMPLE_TOPLEFT:
    case DOWNSAMPLE_MAX:
      output_info->is_binary = input_info->is_binary;
      break;
    case DOWNSAMPLE_AVERAGE:
    case DOWNSAMPLE_SUM:
      output_info->is_binary = 0;
      break;
    default:
      assert(0);
  }
  return NULL;
}

