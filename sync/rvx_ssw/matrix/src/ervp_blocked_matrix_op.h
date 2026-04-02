#ifndef __ERVP_BLOCKED_MATRIX_OP_H__
#define __ERVP_BLOCKED_MATRIX_OP_H__

#include "ervp_malloc.h"
#include "ervp_matrix_op.h"
#include "ervp_special_matrix_op.h"

typedef struct
{
  int block_size;
  ervp_mop_mapping_t *subop_mapping;
} ervp_blocked_matrix_info_t;

static inline ervp_blocked_matrix_info_t *blocked_matrix_info_alloc()
{
  ervp_blocked_matrix_info_t *info = malloc(sizeof(ervp_blocked_matrix_info_t));
  info->block_size = 0;
  info->subop_mapping = NULL;
  return info;
}

static inline void blocked_matrix_info_free(ervp_blocked_matrix_info_t *blocked_info)
{
  free(blocked_info);
}

ervp_hwtask_busy_fx_t blocked_matrix_add(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t blocked_matrix_sub(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t blocked_matrix_ewmult(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t blocked_matrix_mult(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t blocked_matrix_conv(ervp_blocked_matrix_info_t *blocked_info, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t blocked_matrix_conv_sharedinput(ervp_blocked_matrix_info_t *blocked_info, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t blocked_matrix_conv_sharedoutput(ervp_blocked_matrix_info_t *blocked_info, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut);

#endif
