#ifndef __DCA_NEUGEMM_H__
#define __DCA_NEUGEMM_H__

#include "ervp_smart_flush.h"
#include "ervp_matrix_op.h"
#include "ervp_mmiox1.h"
#include "dca_module_memorymap_offset.h"

static const int _DCA_NEUGEMM_ALL_FROM_MEMORY = DCA_NEUGEMM_OPCODE_LSU0_REQ | DCA_NEUGEMM_OPCODE_LSU1_REQ | DCA_NEUGEMM_OPCODE_LSU2_REQ;
static const int DCA_NEUGEMM_ADD = DCA_NEUGEMM_OPCODE_ADDSUB | _DCA_NEUGEMM_ALL_FROM_MEMORY;
static const int DCA_NEUGEMM_SUB = DCA_NEUGEMM_OPCODE_ADDSUB | DCA_NEUGEMM_OPCODE_RSRC_INV | _DCA_NEUGEMM_ALL_FROM_MEMORY;
static const int DCA_NEUGEMM_EWMULT = DCA_NEUGEMM_OPCODE_EWMULT | _DCA_NEUGEMM_ALL_FROM_MEMORY;

static const int DCA_NEUGEMM_MULT_COND = DCA_NEUGEMM_OPCODE_MULT_COND | _DCA_NEUGEMM_ALL_FROM_MEMORY;
static const int DCA_NEUGEMM_MULT = DCA_NEUGEMM_MULT_COND | DCA_NEUGEMM_OPCODE_INIT_ACC;
// static const int DCA_MULT_ACC = DCA_NEUGEMM_MULT_COND | DCA_NEUGEMM_OPCODE_LOAD_ACC;

static const int DCA_NEUGEMM_CONV_COND = DCA_NEUGEMM_OPCODE_CONV_COND | DCA_NEUGEMM_OPCODE_RSRC_CONSTANT | _DCA_NEUGEMM_ALL_FROM_MEMORY;
static const int DCA_NEUGEMM_CONV = DCA_NEUGEMM_CONV_COND | DCA_NEUGEMM_OPCODE_INIT_ACC;
// static const int DCA_CONV_ACC = DCA_NEUGEMM_CONV_COND | DCA_NEUGEMM_OPCODE_LOAD_ACC;

typedef struct
{
  unsigned int bw_addr;
  unsigned int ma_bw_data;
  unsigned int ma_has_burden;
  unsigned int ma_bw_burden;
  unsigned int mb_bw_data;
  unsigned int mb_has_burden;
  unsigned int mb_bw_burden;
  unsigned int mc_bw_data;
  unsigned int mc_has_burden;
  unsigned int mc_bw_burden;
  unsigned int matrix_size_para;
  unsigned int tensor_para;
  unsigned int bw_config;
  unsigned int bw_status;
  unsigned int bw_log;
  unsigned int bw_inst;
  unsigned int bw_input;
  unsigned int bw_output;
  unsigned int config_default_value; // invalid due to the limit of data type, but not used
  unsigned int log_fifo_depth;
  unsigned int inst_fifo_depth;
  unsigned int input_fifo_depth;
  unsigned int output_fifo_depth;
  unsigned int lsu_para;
} dca_neugemm_hwpara_t;

typedef struct
{
  const ervp_mmiox1_hwinfo_t *mmiox_info;
  uint32_t num_row : 16;
  uint32_t num_col : 16;
  uint32_t id;
} dca_neugemm_hwinfo_t;

void dca_neugemm_hwinfo_elaborate(dca_neugemm_hwpara_t *hwpara, dca_neugemm_hwinfo_t *hwinfo);

static inline ervp_hwtask_busy_fx_t dca_neugemm_busy_fx(const dca_neugemm_hwinfo_t *const hwinfo)
{
  return hwinfo->mmiox_info->busy_fx;
}

static inline void dca_neugemm_wait(const dca_neugemm_hwinfo_t *const hwinfo)
{
  hwtask_wait_complete(dca_neugemm_busy_fx(hwinfo));
}

ervp_hwtask_busy_fx_t dca_neugemm_start(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, unsigned int opcode, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value);

static inline ervp_hwtask_busy_fx_t dca_neugemm_start_with_flush(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, unsigned int opcode, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  if (mb_info->is_scalar)
    trackedvar_smart_flush(2, ma_info->addr, mc_info->addr);
  else
    trackedvar_smart_flush(3, ma_info->addr, mb_info->addr, mc_info->addr);
  return dca_neugemm_start(mop_mapping, hwinfo, opcode, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_neugemm_add(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  assert(ma_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mb_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mc_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(ma_info, mb_info));
  assert(matrix_is_same_size(ma_info, mc_info));
  return dca_neugemm_start_with_flush(mop_mapping, hwinfo, DCA_NEUGEMM_ADD, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_neugemm_sub(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  assert(ma_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mb_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mc_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(ma_info, mb_info));
  assert(matrix_is_same_size(ma_info, mc_info));
  return dca_neugemm_start_with_flush(mop_mapping, hwinfo, DCA_NEUGEMM_SUB, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_neugemm_ewmult(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  assert(ma_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mb_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mc_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(matrix_is_same_size(ma_info, mb_info));
  assert(matrix_is_same_size(ma_info, mc_info));
  mc_info->is_binary = ma_info->is_binary & mb_info->is_binary;
  return dca_neugemm_start_with_flush(mop_mapping, hwinfo, DCA_NEUGEMM_EWMULT, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_neugemm_mult(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  assert(ma_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mb_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(mc_info->datatype != MATRIX_DATATYPE_UINT32);
  assert(ma_info->num_col == mb_info->num_row);
  assert(ma_info->num_row == mc_info->num_row);
  assert(mb_info->num_col == mc_info->num_col);
  return dca_neugemm_start_with_flush(mop_mapping, hwinfo, DCA_NEUGEMM_MULT, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t dca_neugemm_conv_oneblock(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t dca_neugemm_conv_oneblock_sharedinput(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, int num_output, const ErvpMatrixInfo *input_info, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo **output_info_list, unsigned int conv_option_value);
ervp_hwtask_busy_fx_t dca_neugemm_conv_oneblock_sharedoutput(ervp_mop_mapping_t *mop_mapping, const dca_neugemm_hwinfo_t *const hwinfo, int num_input, const ErvpMatrixInfo **input_info_list, const ErvpMatrixInfo **kernel_info_list, ErvpMatrixInfo *output_info, unsigned int conv_option_value, int init_ouptut);

#endif