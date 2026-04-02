#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_misc_util.h"
#include "ervp_blocked_matrix_op.h"
#include "ervp_matrix_op.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_special_matrix_op.h"
#include "ervp_matrix_element.h"

ervp_hwtask_busy_fx_t blocked_matrix_mult(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  // printf_function();
  //  matrix_mult_size_print(a, b, c);

  // matrix_print_brief(a);
  // matrix_print_brief(b);
  // matrix_print_brief(c);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  ErvpMatrixInfo block_a = *a;
  ErvpMatrixInfo block_b = *b;
  ErvpMatrixInfo block_c = *c;

  const int HAS_POST_PROCESS = mop_option_has_postprocess(option_value);

  const int block_size = blocked_info->block_size;
  ErvpMatrixInfo *temp_buffer = NULL; // due to overflow
  if (HAS_POST_PROCESS)
  {
    int datatype = matrix_datatype_is_float(c->datatype) ? MATRIX_DATATYPE_FLOAT32 : MATRIX_DATATYPE_SINT32;
    temp_buffer = matrix_alloc(datatype, block_size, block_size, NULL);
  }

  for (int m = 0; m < a->num_row; m += block_size)
  {
    int remain_row = a->num_row - m;
    if (remain_row >= block_size)
      remain_row = block_size;
    block_a.num_row = remain_row;
    block_c.num_row = remain_row;

    for (int n = 0; n < b->num_col; n += block_size)
    {
      int remain_col = b->num_col - n;
      if (remain_col >= block_size)
        remain_col = block_size;
      block_b.num_col = remain_col;
      block_c.num_col = remain_col;

      block_c.addr = matrix_get_element_addr(c, m, n);

      if (HAS_POST_PROCESS)
      {
        temp_buffer->num_row = block_c.num_row;
        temp_buffer->num_col = block_c.num_col;
        // matrix_zero_sw(temp_buffer);
        hwtask_wait_complete(hwtask_busy_fx);
        hwtask_busy_fx = blocked_info->subop_mapping->matrix_zero(blocked_info->subop_mapping, temp_buffer);
      }
      else if (!mop_option_is_acc(option_value))
      {
        // matrix_zero_sw(&block_c);
        hwtask_busy_fx = blocked_info->subop_mapping->matrix_zero(blocked_info->subop_mapping, &block_c);
      }

      for (int k = 0; k < a->num_col; k += block_size)
      {
        int remain_product = a->num_col - k;
        if (remain_product >= block_size)
          remain_product = block_size;
        block_a.num_col = remain_product;
        block_b.num_row = remain_product;

        block_a.addr = matrix_get_element_addr(a, m, k);
        block_b.addr = matrix_get_element_addr(b, k, n);

        hwtask_wait_complete(hwtask_busy_fx);
        if (HAS_POST_PROCESS)
          hwtask_busy_fx = blocked_info->subop_mapping->matrix_mult(blocked_info->subop_mapping, &block_a, &block_b, temp_buffer, mop_option_acc_only());
        else
          hwtask_busy_fx = blocked_info->subop_mapping->matrix_mult(blocked_info->subop_mapping, &block_a, &block_b, &block_c, mop_option_acc_only());
      }
      if (HAS_POST_PROCESS)
      {
        hwtask_wait_complete(hwtask_busy_fx);
        hwtask_busy_fx = matrix_perform_postprocess_tf(blocked_info->subop_mapping, temp_buffer, &block_c, option_value);
      }
    }
  }
  if (temp_buffer)
  {
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(temp_buffer);
  }
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_add(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  ErvpMatrixInfo block_a = *a;
  ErvpMatrixInfo block_b = *b;
  ErvpMatrixInfo block_c = *c;
  int block_size = blocked_info->block_size;

  for (int m = 0; m < a->num_row; m += block_size)
  {
    int remain_row = a->num_row - m;
    if (remain_row >= block_size)
      remain_row = block_size;
    block_a.num_row = remain_row;
    block_b.num_row = remain_row;
    block_c.num_row = remain_row;

    for (int n = 0; n < a->num_col; n += block_size)
    {
      int remain_col = a->num_col - n;
      if (remain_col >= block_size)
        remain_col = block_size;
      block_a.num_col = remain_col;
      block_b.num_col = remain_col;
      block_c.num_col = remain_col;

      block_a.addr = matrix_get_element_addr(a, m, n);
      block_b.addr = matrix_get_element_addr(b, m, n);
      block_c.addr = matrix_get_element_addr(c, m, n);

      hwtask_busy_fx = blocked_info->subop_mapping->matrix_add(blocked_info->subop_mapping, &block_a, &block_b, &block_c, option_value);
    }
  }
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_sub(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  ErvpMatrixInfo block_a = *a;
  ErvpMatrixInfo block_b = *b;
  ErvpMatrixInfo block_c = *c;
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  int block_size = blocked_info->block_size;

  for (int m = 0; m < a->num_row; m += block_size)
  {
    int remain_row = a->num_row - m;
    if (remain_row >= block_size)
      remain_row = block_size;
    block_a.num_row = remain_row;
    block_b.num_row = remain_row;
    block_c.num_row = remain_row;

    for (int n = 0; n < a->num_col; n += block_size)
    {
      int remain_col = a->num_col - n;
      if (remain_col >= block_size)
        remain_col = block_size;
      block_a.num_col = remain_col;
      block_b.num_col = remain_col;
      block_c.num_col = remain_col;

      block_a.addr = matrix_get_element_addr(a, m, n);
      block_b.addr = matrix_get_element_addr(b, m, n);
      block_c.addr = matrix_get_element_addr(c, m, n);

      hwtask_busy_fx = blocked_info->subop_mapping->matrix_sub(blocked_info->subop_mapping, &block_a, &block_b, &block_c, option_value);
    }
  }
  mop_option_free(mop_option);
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_ewmult(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  ErvpMatrixInfo block_a = *a;
  ErvpMatrixInfo block_b = *b;
  ErvpMatrixInfo block_c = *c;
  int block_size = blocked_info->block_size;

  for (int m = 0; m < a->num_row; m += block_size)
  {
    int remain_row = a->num_row - m;
    if (remain_row >= block_size)
      remain_row = block_size;
    block_a.num_row = remain_row;
    block_b.num_row = remain_row;
    block_c.num_row = remain_row;

    for (int n = 0; n < a->num_col; n += block_size)
    {
      int remain_col = a->num_col - n;
      if (remain_col >= block_size)
        remain_col = block_size;
      block_a.num_col = remain_col;
      block_b.num_col = remain_col;
      block_c.num_col = remain_col;

      block_a.addr = matrix_get_element_addr(a, m, n);
      block_b.addr = matrix_get_element_addr(b, m, n);
      block_c.addr = matrix_get_element_addr(c, m, n);

      hwtask_busy_fx = blocked_info->subop_mapping->matrix_ewmult(blocked_info->subop_mapping, &block_a, &block_b, &block_c, option_value);
    }
  }
  return hwtask_busy_fx;
}

static inline int _matrix_conv_output_rows_per_block(int block_row, int kernel_row, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  return (((block_row - kernel_row) / (conv_option.br.stride_m1 + 1))) + 1;
}

static inline int col_matrix_conv_required_input_size(int output_row, int kernel_row, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  return ((output_row - 1) * (conv_option.br.stride_m1 + 1)) + kernel_row;
}

static ervp_hwtask_busy_fx_t _blocked_matrix_conv_nopad(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  assert(kernel_info->num_row == kernel_info->num_col);
  assert(!matrix_conv_has_pad(conv_option_value));

  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;

  const int8_t output_block_size = _matrix_conv_output_rows_per_block(blocked_info->block_size, kernel_info->num_row, conv_option_value);
  const int output_block_addr_offset = matrix_get_element_addr_offset_in_row(output_info, output_block_size);
  const uint8_t output_block_bit_offset = _matrix_get_element_addr_bit_offset(output_info, output_block_size);
  const int8_t input_block_size = col_matrix_conv_required_input_size(output_block_size, kernel_info->num_row, conv_option_value);
  const int input_block_addr_offset = matrix_get_element_addr_offset_in_row(input_info, output_block_size * (conv_option.br.stride_m1 + 1));
  const uint8_t input_block_bit_offset = _matrix_get_element_addr_bit_offset(input_info, output_block_size * (conv_option.br.stride_m1 + 1));

  ErvpMatrixInfo block_o;
  block_o.stride_ls3 = output_info->stride_ls3;
  block_o.datatype = output_info->datatype;
  block_o.num_row = output_block_size;
  block_o.is_binary = output_info->is_binary;
  ErvpMatrixInfo block_i;
  block_i.stride_ls3 = input_info->stride_ls3;
  block_i.datatype = input_info->datatype;
  block_i.num_row = input_block_size;
  block_i.is_binary = input_info->is_binary;

  // m, n are indexes of the output_matrix
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  for (int m = 0; m < output_info->num_row; m += output_block_size)
  {
    const int remain_row = output_info->num_row - m;
    if (remain_row < output_block_size)
    {
      block_o.num_row = remain_row;
      block_i.num_row = col_matrix_conv_required_input_size(remain_row, kernel_info->num_row, conv_option.value);
    }
    block_o.num_col = output_block_size;
    block_o.addr = matrix_get_element_addr(output_info, m, 0);
    block_o.bit_offset = 0;
    block_i.num_col = input_block_size;
    block_i.addr = matrix_get_element_addr(input_info, m * (conv_option.br.stride_m1 + 1), 0);
    block_i.bit_offset = 0;

    for (int n = 0; n < output_info->num_col; n += output_block_size)
    {
      int remain_col = output_info->num_col - n;
      if (remain_col < output_block_size)
      {
        block_o.num_col = remain_col;
        block_i.num_col = col_matrix_conv_required_input_size(remain_col, kernel_info->num_row, conv_option.value);
      }

      assert(block_o.bit_offset == 0);
      hwtask_busy_fx = blocked_info->subop_mapping->matrix_conv(blocked_info->subop_mapping, &block_i, kernel_info, &block_o, conv_option.value);
      block_o.addr += output_block_addr_offset;
      if (output_block_bit_offset)
      {
        block_o.bit_offset += output_block_bit_offset;
        // block_o.bit_offset &= 7;
      }
      block_i.addr += input_block_addr_offset;
      if (input_block_bit_offset)
      {
        block_i.bit_offset += input_block_bit_offset;
        // block_i.bit_offset &= 7;
      }
    }
  }
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_conv(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value)
{
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  if (matrix_conv_has_pad(conv_option_value))
  {
    ervp_mpad_option_t pad_option;
    pad_option.value = matrix_pad_gen_option(conv_option_value);
    ErvpMatrixInfo *padded_input_info = matrix_pad_alloc_output(input_info, pad_option.value);
    hwtask_busy_fx = blocked_info->subop_mapping->matrix_pad(blocked_info->subop_mapping, input_info, padded_input_info, pad_option.value);
    ervp_mconv_option_t conv_option;
    conv_option.value = conv_option_value;
    conv_option.br.pad_amount = 0;
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = _blocked_matrix_conv_nopad(blocked_info, padded_input_info, kernel_info, output_info, conv_option.value);
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(padded_input_info);
  }
  else
    hwtask_busy_fx = _blocked_matrix_conv_nopad(blocked_info, input_info, kernel_info, output_info, conv_option_value);
  return hwtask_busy_fx;
}

static ervp_hwtask_busy_fx_t _blocked_matrix_conv_sharedinput_nopad(ervp_blocked_matrix_info_t *blocked_info, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  assert(kernel_info_list[0]->num_row == kernel_info_list[0]->num_col);
  assert(!matrix_conv_has_pad(conv_option_value));

  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;

  const int8_t output_block_size = _matrix_conv_output_rows_per_block(blocked_info->block_size, kernel_info_list[0]->num_row, conv_option_value);
  const int output_block_addr_offset = matrix_get_element_addr_offset_in_row(output_info_list[0], output_block_size);
  const uint8_t output_block_bit_offset = _matrix_get_element_addr_bit_offset(output_info_list[0], output_block_size);
  const int8_t input_block_size = col_matrix_conv_required_input_size(output_block_size, kernel_info_list[0]->num_row, conv_option_value);
  const int input_block_addr_offset = matrix_get_element_addr_offset_in_row(input_info, output_block_size * (conv_option.br.stride_m1 + 1));
  const uint8_t input_block_bit_offset = _matrix_get_element_addr_bit_offset(input_info, output_block_size * (conv_option.br.stride_m1 + 1));

  for (int i = 0; i < num_output; i++)
    assert(output_block_addr_offset == matrix_get_element_addr_offset_in_row(output_info_list[i], output_block_size));

  ErvpMatrixInfo *p_block_o_list[num_output];
  ErvpMatrixInfo block_o_list[num_output];
  for (int i = 0; i < num_output; i++)
  {
    p_block_o_list[i] = &(block_o_list[i]);
    // block_o_list[i] = *(output_info_list[i]);
    block_o_list[i].stride_ls3 = output_info_list[i]->stride_ls3;
    block_o_list[i].datatype = output_info_list[i]->datatype;
    block_o_list[i].num_row = output_block_size;
    block_o_list[i].is_binary = output_info_list[i]->is_binary;
  }
  ErvpMatrixInfo block_i;
  block_i.stride_ls3 = input_info->stride_ls3;
  block_i.datatype = input_info->datatype;
  block_i.num_row = input_block_size;
  block_i.is_binary = input_info->is_binary;

  // m, n are indexes of the output_matrix
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  for (int m = 0; m < output_info_list[0]->num_row; m += output_block_size)
  {
    const int remain_row = output_info_list[0]->num_row - m;
    if (remain_row < output_block_size)
    {
      for (int i = 0; i < num_output; i++)
        block_o_list[i].num_row = remain_row;
      block_i.num_row = col_matrix_conv_required_input_size(remain_row, kernel_info_list[0]->num_row, conv_option.value);
    }
    for (int i = 0; i < num_output; i++)
    {
      block_o_list[i].num_col = output_block_size;
      block_o_list[i].addr = matrix_get_element_addr(output_info_list[i], m, 0);
      block_o_list[i].bit_offset = 0;
    }
    block_i.num_col = input_block_size;
    block_i.addr = matrix_get_element_addr(input_info, m * (conv_option.br.stride_m1 + 1), 0);
    block_i.bit_offset = 0;

    for (int n = 0; n < output_info_list[0]->num_col; n += output_block_size)
    {
      int remain_col = output_info_list[0]->num_col - n;
      if (remain_col < output_block_size)
      {
        for (int i = 0; i < num_output; i++)
          block_o_list[i].num_col = remain_col;
        block_i.num_col = col_matrix_conv_required_input_size(remain_col, kernel_info_list[0]->num_row, conv_option.value);
      }

      assert(block_o_list[0].bit_offset == 0);
      hwtask_busy_fx = blocked_info->subop_mapping->matrix_conv_sharedinput(blocked_info->subop_mapping, num_output, &block_i, kernel_info_list, p_block_o_list, conv_option.value);
      for (int i = 0; i < num_output; i++)
      {
        block_o_list[i].addr += output_block_addr_offset;
        if (output_block_bit_offset)
        {
          block_o_list[i].bit_offset += output_block_bit_offset;
          // block_o_list[i].bit_offset &= 7; // NOT needed
        }
      }
      block_i.addr += input_block_addr_offset;
      if (input_block_bit_offset)
      {
        block_i.bit_offset += input_block_bit_offset;
        // block_i.bit_offset &= 7; // NOT needed
      }
    }
  }

  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_conv_sharedinput(ervp_blocked_matrix_info_t *blocked_info, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value)
{
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  if (matrix_conv_has_pad(conv_option_value))
  {
    ervp_mpad_option_t pad_option;
    pad_option.value = matrix_pad_gen_option(conv_option_value);
    ErvpMatrixInfo *padded_input_info = matrix_pad_alloc_output(input_info, pad_option.value);
    hwtask_busy_fx = blocked_info->subop_mapping->matrix_pad(blocked_info->subop_mapping, input_info, padded_input_info, pad_option.value);
    ervp_mconv_option_t conv_option;
    conv_option.value = conv_option_value;
    conv_option.br.pad_amount = 0;
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = _blocked_matrix_conv_sharedinput_nopad(blocked_info, num_output, padded_input_info, kernel_info_list, output_info_list, conv_option.value);
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    matrix_free(padded_input_info);
  }
  else
    hwtask_busy_fx = _blocked_matrix_conv_sharedinput_nopad(blocked_info, num_output, input_info, kernel_info_list, output_info_list, conv_option_value);
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t _blocked_matrix_conv_sharedoutput_nopad(ervp_blocked_matrix_info_t *blocked_info, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  assert(kernel_info_list[0]->num_row == kernel_info_list[0]->num_col);
  assert(!matrix_conv_has_pad(conv_option_value));

  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;

  const int8_t output_block_size = _matrix_conv_output_rows_per_block(blocked_info->block_size, kernel_info_list[0]->num_row, conv_option_value);
  const int output_block_addr_offset = matrix_get_element_addr_offset_in_row(output_info, output_block_size);
  const uint8_t output_block_bit_offset = _matrix_get_element_addr_bit_offset(output_info, output_block_size);
  const int8_t input_block_size = col_matrix_conv_required_input_size(output_block_size, kernel_info_list[0]->num_row, conv_option_value);
  const int input_block_addr_offset = matrix_get_element_addr_offset_in_row(input_info_list[0], output_block_size * (conv_option.br.stride_m1 + 1));
  const uint8_t input_block_bit_offset = _matrix_get_element_addr_bit_offset(input_info_list[0], output_block_size * (conv_option.br.stride_m1 + 1));

  for (int i = 0; i < num_input; i++)
    assert(input_block_addr_offset == matrix_get_element_addr_offset_in_row(input_info_list[i], output_block_size * (conv_option.br.stride_m1 + 1)));

  ErvpMatrixInfo block_o;
  block_o.stride_ls3 = output_info->stride_ls3;
  block_o.datatype = output_info->datatype;
  block_o.num_row = output_block_size;
  block_o.is_binary = output_info->is_binary;
  ErvpMatrixInfo *p_block_i_list[num_input];
  ErvpMatrixInfo block_i_list[num_input];
  for (int i = 0; i < num_input; i++)
  {
    p_block_i_list[i] = &(block_i_list[i]);
    // block_i_list[i] = *(input_info_list[i]);
    block_i_list[i].stride_ls3 = input_info_list[i]->stride_ls3;
    block_i_list[i].datatype = input_info_list[i]->datatype;
    block_i_list[i].num_row = input_block_size;
    block_i_list[i].is_binary = input_info_list[i]->is_binary;
  }

  // m, n are indexes of the output_matrix
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  for (int m = 0; m < output_info->num_row; m += output_block_size)
  {
    const int remain_row = output_info->num_row - m;
    if (remain_row < output_block_size)
    {
      block_o.num_row = remain_row;
      for (int i = 0; i < num_input; i++)
        block_i_list[i].num_row = col_matrix_conv_required_input_size(remain_row, kernel_info_list[0]->num_row, conv_option.value);
    }
    block_o.num_col = output_block_size;
    block_o.addr = matrix_get_element_addr(output_info, m, 0);
    block_o.bit_offset = 0;
    for (int i = 0; i < num_input; i++)
    {
      block_i_list[i].num_col = input_block_size;
      block_i_list[i].addr = matrix_get_element_addr(input_info_list[i], m * (conv_option.br.stride_m1 + 1), 0);
      block_i_list[i].bit_offset = 0;
    }

    for (int n = 0; n < output_info->num_col; n += output_block_size)
    {
      int remain_col = output_info->num_col - n;
      if (remain_col < output_block_size)
      {
        block_o.num_col = remain_col;
        for (int i = 0; i < num_input; i++)
          block_i_list[i].num_col = col_matrix_conv_required_input_size(remain_col, kernel_info_list[0]->num_row, conv_option.value);
      }

      assert(block_o.bit_offset == 0);
      hwtask_busy_fx = blocked_info->subop_mapping->matrix_conv_sharedoutput(blocked_info->subop_mapping, num_input, p_block_i_list, kernel_info_list, &block_o, conv_option_value, init_ouptut);
      block_o.addr += output_block_addr_offset;
      if (output_block_bit_offset)
      {
        block_o.bit_offset += output_block_bit_offset;
        // block_o.bit_offset &= 7;
      }
      for (int i = 0; i < num_input; i++)
      {
        block_i_list[i].addr += input_block_addr_offset;
        if (input_block_bit_offset)
        {
          block_i_list[i].bit_offset += input_block_bit_offset;
          // block_i_list[i].bit_offset &= 7;
        }
      }
    }
  }
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_conv_sharedoutput(ervp_blocked_matrix_info_t *blocked_info, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut)
{
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  if (matrix_conv_has_pad(conv_option_value))
  {
    ervp_mpad_option_t pad_option;
    pad_option.value = matrix_pad_gen_option(conv_option_value);
    ErvpMatrixInfo *padded_input_info_list[num_input];
    for (int i = 0; i < num_input; i++)
    {
      padded_input_info_list[i] = matrix_pad_alloc_output(input_info_list[i], pad_option.value);
      hwtask_busy_fx = blocked_info->subop_mapping->matrix_pad(blocked_info->subop_mapping, input_info_list[i], padded_input_info_list[i], pad_option.value);
    }
    ervp_mconv_option_t conv_option;
    conv_option.value = conv_option_value;
    conv_option.br.pad_amount = 0;
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = _blocked_matrix_conv_sharedoutput_nopad(blocked_info, num_input, padded_input_info_list, kernel_info_list, output_info, conv_option.value, init_ouptut);
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = NULL;
    for (int i = 0; i < num_input; i++)
      matrix_free(padded_input_info_list[i]);
  }
  else
    hwtask_busy_fx = _blocked_matrix_conv_sharedoutput_nopad(blocked_info, num_input, input_info_list, kernel_info_list, output_info, conv_option_value, init_ouptut);
  return hwtask_busy_fx;
}

ervp_hwtask_busy_fx_t blocked_matrix_conv_sharedoutput_old(ervp_blocked_matrix_info_t *blocked_info, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut)
{
  assert(blocked_info);
  assert(blocked_info->subop_mapping);

  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  assert(conv_option.br.acc);
  ervp_hwtask_busy_fx_t hwtask_busy_fx = NULL;
  if (init_ouptut)
    hwtask_busy_fx = blocked_info->subop_mapping->matrix_zero(blocked_info->subop_mapping, output_info);
  for (int i = 0; i < num_input; i++)
  {
    hwtask_wait_complete(hwtask_busy_fx);
    hwtask_busy_fx = blocked_matrix_conv(blocked_info, input_info_list[i], kernel_info_list[i], output_info, conv_option_value);
  }
  return hwtask_busy_fx;
}