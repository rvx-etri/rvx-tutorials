#ifndef __ERVP_MATRIX_OP_SW_H__
#define __ERVP_MATRIX_OP_SW_H__

#include "ervp_matrix.h"
#include "ervp_matrix_op.h"
#include "ervp_assert.h"

// cal by sw
static inline void _matrix_check_copy_part(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  assert(num_row <= a->num_row);
  assert(num_row <= c->num_row);
  assert(num_col <= a->num_col);
  assert(num_col <= c->num_col);
}

static inline void _matrix_check_transpose_part(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value)
{
  assert(num_row <= a->num_row);
  assert(num_row <= c->num_col);
  assert(num_col <= a->num_col);
  assert(num_col <= c->num_row);
}

ervp_hwtask_busy_fx_t _matrix_copy_part_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_transpose_part_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_reshape_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_add_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_add_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_add_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_sub_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_sub_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_sub_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_ewmult_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_ewmult_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_ewmult_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_mult_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_mult_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t _matrix_mult_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_conv_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t _matrix_conv_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t _matrix_conv_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);

ervp_hwtask_busy_fx_t _matrix_shift_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, int shamount, ErvpMatrixInfo *c, unsigned int option_value);

ervp_hwtask_busy_fx_t _matrix_downsample_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value);
ervp_hwtask_busy_fx_t _matrix_downsample_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value);
ervp_hwtask_busy_fx_t _matrix_downsample_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value);

ervp_hwtask_busy_fx_t _matrix_max_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_max_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_max_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);

ervp_hwtask_busy_fx_t _matrix_min_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_min_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_min_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);

ervp_hwtask_busy_fx_t _matrix_asl_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_asl_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);

ervp_hwtask_busy_fx_t _matrix_asr_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);
ervp_hwtask_busy_fx_t _matrix_asr_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c);

ervp_hwtask_busy_fx_t _matrix_compare_float_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode);
ervp_hwtask_busy_fx_t _matrix_compare_fixed_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode);
ervp_hwtask_busy_fx_t _matrix_compare_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode);

// redefine

static inline void matrix_copy_part_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value) { _matrix_copy_part_sw(NULL, a, c, num_row, num_col, option_value); }
static inline void matrix_transpose_part_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, int num_row, int num_col, unsigned int option_value) { _matrix_transpose_part_sw(NULL, a, c, num_row, num_col, option_value); }
static inline void matrix_reshape_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_reshape_sw(NULL, a, c, option_value); }

static inline void matrix_add_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_add_float_sw(NULL, a, b, c, option_value); }
static inline void matrix_add_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_add_fixed_sw(NULL, a, b, c, option_value); }
static inline void matrix_add_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_add_sw(NULL, a, b, c, option_value); }

static inline void matrix_sub_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_sub_float_sw(NULL, a, b, c, option_value); }
static inline void matrix_sub_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_sub_fixed_sw(NULL, a, b, c, option_value); }
static inline void matrix_sub_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_sub_sw(NULL, a, b, c, option_value); }

static inline void matrix_ewmult_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_ewmult_float_sw(NULL, a, b, c, option_value); }
static inline void matrix_ewmult_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_ewmult_fixed_sw(NULL, a, b, c, option_value); }
static inline void matrix_ewmult_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_ewmult_sw(NULL, a, b, c, option_value); }

static inline void matrix_mult_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_mult_float_sw(NULL, a, b, c, option_value); }
static inline void matrix_mult_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_mult_fixed_sw(NULL, a, b, c, option_value); }
static inline void matrix_mult_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_mult_sw(NULL, a, b, c, option_value); }

static inline void matrix_conv_float_sw(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value) { _matrix_conv_float_sw(NULL, input_info, kernel_info, output_info, conv_option_value); }
static inline void matrix_conv_fixed_sw(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value) { _matrix_conv_fixed_sw(NULL, input_info, kernel_info, output_info, conv_option_value); }
static inline void matrix_conv_sw(const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value) { _matrix_conv_sw(NULL, input_info, kernel_info, output_info, conv_option_value); }

static inline void matrix_shift_fixed_sw(const ErvpMatrixInfo *a, int shamount, ErvpMatrixInfo *c, unsigned int option_value) { _matrix_shift_fixed_sw(NULL, a, shamount, c, option_value); }

static inline void matrix_downsample_float_sw(const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value) { _matrix_downsample_float_sw(NULL, input_info, output_info, downsample_option_value); }
static inline void matrix_downsample_fixed_sw(const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value) { _matrix_downsample_fixed_sw(NULL, input_info, output_info, downsample_option_value); }
static inline void matrix_downsample_sw(const ErvpMatrixInfo *input_info, ErvpMatrixInfo *output_info, unsigned int downsample_option_value) { _matrix_downsample_sw(NULL, input_info, output_info, downsample_option_value); }

static inline void matrix_max_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_max_float_sw(NULL, a, b, c); }
static inline void matrix_max_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_max_fixed_sw(NULL, a, b, c); }
static inline void matrix_max_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_max_sw(NULL, a, b, c); }

static inline void matrix_min_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_min_float_sw(NULL, a, b, c); }
static inline void matrix_min_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_min_fixed_sw(NULL, a, b, c); }
static inline void matrix_min_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_min_sw(NULL, a, b, c); }

static inline void matrix_asl_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_asl_fixed_sw(NULL, a, b, c); }
static inline void matrix_asl_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_asl_sw(NULL, a, b, c); }

static inline void matrix_asr_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_asr_fixed_sw(NULL, a, b, c); }
static inline void matrix_asr_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c) { _matrix_asr_sw(NULL, a, b, c); }

static inline void matrix_compare_float_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode) { _matrix_compare_float_sw(NULL, a, b, c, compare_mode); }
static inline void matrix_compare_fixed_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode) { _matrix_compare_fixed_sw(NULL, a, b, c, compare_mode); }
static inline void matrix_compare_sw(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode) { _matrix_compare_sw(NULL, a, b, c, compare_mode); }

static inline ervp_hwtask_busy_fx_t _matrix_copy_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(matrix_is_same_size(a, c));
  ervp_hwtask_busy_fx_t hwtask_busy_fx = _matrix_copy_part_sw(NULL, a, c, a->num_row, a->num_col, option_value);
  if (!c->is_sub)
    c->is_binary = a->is_binary;
  return hwtask_busy_fx;
}

static inline void matrix_copy_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value)
{
  _matrix_copy_sw(NULL, a, c, option_value);
}

static inline ervp_hwtask_busy_fx_t _matrix_transpose_sw(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value)
{
  assert(a->num_row == c->num_col);
  assert(a->num_col == c->num_row);
  ervp_hwtask_busy_fx_t hwtask_busy_fx = _matrix_transpose_part_sw(NULL, a, c, a->num_row, a->num_col, option_value);
  if (!c->is_sub)
    c->is_binary = a->is_binary;
  return hwtask_busy_fx;
}

static inline void matrix_transpose_sw(const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value)
{
  _matrix_transpose_sw(NULL, a, c, option_value);
}

#endif
