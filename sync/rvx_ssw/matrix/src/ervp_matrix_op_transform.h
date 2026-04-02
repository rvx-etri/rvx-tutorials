#ifndef __ERVP_MATRIX_OP_TRANSFORM_H__
#define __ERVP_MATRIX_OP_TRANSFORM_H__

#include "ervp_matrix.h"
#include "ervp_matrix_op.h"

ervp_hwtask_busy_fx_t matrix_copy2copy_part_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t matrix_transpose2transpose_part_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);
ervp_hwtask_busy_fx_t matrix_conv2mult_im2col(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t matrix_conv2mult_v1_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo *kernel_info, ErvpMatrixInfo *output_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t matrix_conv_sharedinput_tf(ervp_mop_mapping_t *mop_mapping, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t matrix_conv_sharedinput_im2col_tf(ervp_mop_mapping_t *mop_mapping, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t matrix_conv_sharedoutput_tf(ervp_mop_mapping_t *mop_mapping, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut);

ervp_hwtask_busy_fx_t matrix_zero2fill_tf(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);
ervp_hwtask_busy_fx_t matrix_one2fill_tf(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);

ervp_hwtask_busy_fx_t matrix_pad2copy_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int pad_option_value);
ervp_hwtask_busy_fx_t matrix_padfill2copy_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int pad_option_value);

ervp_hwtask_busy_fx_t matrix_perform_postprocess_tf(ervp_mop_mapping_t *mop_mapping, const ErvpMatrixInfo *a, ErvpMatrixInfo *c, unsigned int option_value);

#endif