#ifndef __ERVP_MATRIX_OP_H__
#define __ERVP_MATRIX_OP_H__

#include "ervp_printf.h"
#include "ervp_malloc.h"
#include "ervp_matrix.h"
#include "ervp_matrix_element.h"
#include "ervp_hwtask.h"

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int rshift : 8;
    unsigned int performs_cliping : 1;
    unsigned int acc : 1;
    unsigned int stride_m1 : 4;
  } br;
} ervp_mop_option_t;

static const unsigned int PADMODE_NONE = 0;
static const unsigned int PADMODE_ZEROS = 1;
static const unsigned int PADMODE_REPLICATE = 2;

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int rshift : 8;
    unsigned int performs_cliping : 1;
    unsigned int acc : 1;
    unsigned int stride_m1 : 4; // to make its default value 0
    unsigned int pad_amount : 4;
    unsigned int pad_has_rowu : 1;
    unsigned int pad_has_rowd : 1;
    unsigned int pad_has_colu : 1;
    unsigned int pad_has_cold : 1;
    unsigned int pad_mode : 3;
    // unsigned int boundary_to_mult : 1;
    // unsigned int pretty_to_mult : 8; // number of col in left matrix
  } br;
} ervp_mconv_option_t;

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int num_rowu : 6;
    unsigned int num_rowd : 6;
    unsigned int num_colu : 6;
    unsigned int num_cold : 6;
    unsigned int mode : 3;
    unsigned int is_regular : 1;
  } br;
} ervp_mpad_option_t;

static const unsigned int DOWNSAMPLE_NONE = 0;
static const unsigned int DOWNSAMPLE_TOPLEFT = 1;
static const unsigned int DOWNSAMPLE_MAX = 2;
static const unsigned int DOWNSAMPLE_AVERAGE = 3;
static const unsigned int DOWNSAMPLE_SUM = 4;

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int stride_m1 : 16;
    unsigned int downsample_mode : 5;
    unsigned int pad_amount : 4;
    unsigned int pad_has_rowu : 1;
    unsigned int pad_has_rowd : 1;
    unsigned int pad_has_colu : 1;
    unsigned int pad_has_cold : 1;
    unsigned int pad_mode : 3;
  } br;
} ervp_mdownsample_option_t;

typedef struct _ervp_mop_mapping ervp_mop_mapping_t;

typedef struct _ervp_mop_mapping
{
  ervp_hwtask_busy_fx_t (*matrix_copy_part)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_transpose_part)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_reshape)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);

  ervp_hwtask_busy_fx_t (*matrix_add)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_sub)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_ewmult)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_mult)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_conv)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int conv_option_value);
  ervp_hwtask_busy_fx_t (*matrix_conv_sharedinput)(ervp_mop_mapping_t *mop_mapping, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value);
  ervp_hwtask_busy_fx_t (*matrix_conv_sharedoutput)(ervp_mop_mapping_t *mop_mapping, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_output);
  ervp_hwtask_busy_fx_t (*matrix_downsample)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value);

  ervp_hwtask_busy_fx_t (*matrix_scalar_mult_fixed)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int scalar_value, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_scalar_mult_float)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, float scalar_value, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_shift_fixed)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int shamount, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_pad)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int pad_option_value);

  ervp_hwtask_busy_fx_t (*matrix_fill_fixed)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result, int32_t value);
  ervp_hwtask_busy_fx_t (*matrix_fill_float)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result, float value);
  ervp_hwtask_busy_fx_t (*matrix_identity)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);

  ervp_hwtask_busy_fx_t (*matrix_copy)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_transpose)(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);
  ervp_hwtask_busy_fx_t (*matrix_zero)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);
  ervp_hwtask_busy_fx_t (*matrix_one)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);

} ervp_mop_mapping_t;

// ervp_mop_option_t
int _melement_perform_rshift_and_clip(int value, int rshift, int performs_cliping, int datatype);

static inline ervp_mop_option_t mop_option_alloc(unsigned int value)
{
  ervp_mop_option_t mop_option;
  mop_option.value = value;
  return mop_option;
}
static inline void mop_option_free(ervp_mop_option_t option) {}
static inline int _mop_option_check(const ErvpMatrixInfo *minfo, unsigned int option_value)
{
  int option_is_okay = 1;
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  if (matrix_datatype_is_float(minfo->datatype))
  {
    if (mop_option.br.rshift != 0)
      option_is_okay = 0;
    else if (mop_option.br.performs_cliping != 0)
      option_is_okay = 0;
  }
  mop_option_free(mop_option);
  return option_is_okay;
}

static inline int mop_option_has_postprocess(unsigned int value)
{
  ervp_mop_option_t mop_option;
  mop_option.value = value;
  return (mop_option.br.performs_cliping | mop_option.br.rshift | mop_option.br.stride_m1) != 0;
}

static inline int mop_option_is_acc(unsigned int value)
{
  ervp_mop_option_t mop_option;
  mop_option.value = value;
  return mop_option.br.acc;
}

static inline unsigned int mop_option_acc_only()
{
  ervp_mop_option_t mop_option;
  mop_option.value = 0;
  mop_option.br.acc = 1;
  return mop_option.value;
}

// ervp_mconv_option_t
static inline unsigned int matrix_conv_set_pad(unsigned int conv_option_value, int pad_amount, int pad_mode)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  conv_option.br.pad_amount = pad_amount;
  conv_option.br.pad_has_rowd = 1;
  conv_option.br.pad_has_rowu = 1;
  conv_option.br.pad_has_cold = 1;
  conv_option.br.pad_has_colu = 1;
  conv_option.br.pad_mode = pad_mode;
  return conv_option.value;
}

static inline unsigned int matrix_conv_has_pad(unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  int result = (conv_option.br.pad_amount > 0);
  if (result)
  {
    assert(conv_option.br.pad_has_rowd || conv_option.br.pad_has_rowu || conv_option.br.pad_has_cold || conv_option.br.pad_has_colu);
    assert(conv_option.br.pad_mode != PADMODE_NONE);
  }
  return result;
}

static inline int matrix_conv_output_rows(int input_row, int kernel_row, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  int output_rows = ((input_row + (conv_option.br.pad_amount * (conv_option.br.pad_has_rowd + conv_option.br.pad_has_rowu)) - kernel_row) / (conv_option.br.stride_m1 + 1)) + 1;
  assert(output_rows > 0);
  return output_rows;
}

static inline int matrix_conv_output_cols(int input_col, int kernel_col, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  int output_cols = ((input_col + (conv_option.br.pad_amount * (conv_option.br.pad_has_cold + conv_option.br.pad_has_colu)) - kernel_col) / (conv_option.br.stride_m1 + 1)) + 1;
  assert(output_cols > 0);
  return output_cols;
}

/*
static inline int matrix_conv_input_rows(int output_row, int kernel_row, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  return ((output_row - 1) * (conv_option.br.stride_m1 + 1)) + kernel_row;
}

static inline int matrix_conv_input_cols(int output_col, int kernel_col, unsigned int conv_option_value)
{
  ervp_mconv_option_t conv_option;
  conv_option.value = conv_option_value;
  return ((output_col - 1) * (conv_option.br.stride_m1 + 1)) + kernel_col;
}
*/

ErvpMatrixInfo *matrix_conv_alloc_output(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, unsigned int conv_option_value);
int matrix_conv_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, const ErvpMatrixInfo *output_info, unsigned int conv_option_value);

static inline int _matrix_conv_cal_start_row_index_of_input_matrix(int kernel_size, int output_row_index, ervp_mconv_option_t conv_option)
{
  int input_index;
  input_index = output_row_index * (conv_option.br.stride_m1 + 1);
  if (conv_option.br.pad_has_rowd)
    input_index -= conv_option.br.pad_amount;
  return input_index;
}

static inline int _matrix_conv_cal_start_col_index_of_input_matrix(int kernel_size, int output_col_index, ervp_mconv_option_t conv_option)
{
  int input_index;
  input_index = output_col_index * (conv_option.br.stride_m1 + 1);
  if (conv_option.br.pad_has_cold)
    input_index -= conv_option.br.pad_amount;
  return input_index;
}

// ervp_mpad_option_t
unsigned int matrix_pad_gen_option(unsigned int conv_option_value);

static inline int matrix_pad_output_rows(int input_row, unsigned int pad_option_value)
{
  ervp_mpad_option_t pad_option;
  pad_option.value = pad_option_value;
  return input_row + pad_option.br.num_rowd + pad_option.br.num_rowu;
}

static inline int matrix_pad_output_cols(int input_col, unsigned int pad_option_value)
{
  ervp_mpad_option_t pad_option;
  pad_option.value = pad_option_value;
  return input_col + pad_option.br.num_cold + pad_option.br.num_colu;
}

ErvpMatrixInfo *matrix_pad_alloc_output(const ErvpMatrixInfo *input_info, unsigned int pad_option_value);
int matrix_pad_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *output_info, unsigned int pad_option_value);

// ervp_mop_mapping_t
ervp_mop_mapping_t *matrix_op_mapping_alloc();
static inline void matrix_op_mapping_free(ervp_mop_mapping_t *mapping)
{
  free(mapping);
}

int matrix_downsample_check_size(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *output_info, unsigned int downsample_option_value);

static inline int _matrix_downsample_cal_start_row_index_of_input_matrix(int output_row_index, ervp_mdownsample_option_t downsample_option)
{
  int input_index;
  input_index = output_row_index * (downsample_option.br.stride_m1 + 1);
  if (downsample_option.br.pad_has_rowd)
    input_index -= downsample_option.br.pad_amount;
  return input_index;
}

static inline int _matrix_downsample_cal_start_col_index_of_input_matrix(int output_col_index, ervp_mdownsample_option_t downsample_option)
{
  int input_index;
  input_index = output_col_index * (downsample_option.br.stride_m1 + 1);
  if (downsample_option.br.pad_has_cold)
    input_index -= downsample_option.br.pad_amount;
  return input_index;
}

#endif
