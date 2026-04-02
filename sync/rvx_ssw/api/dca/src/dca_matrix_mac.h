#ifndef __DCA_MATRIX_MAC_H__
#define __DCA_MATRIX_MAC_H__

#include "ervp_matrix_op.h"
#include "ervp_mmiox1.h"
#include "dca_module_memorymap_offset.h"

static const int _DCA_MAC_ALL_FROM_MEMORY = DCA_MATRIX_MAC_OPCODE_LSU0_REQ | DCA_MATRIX_MAC_OPCODE_LSU1_REQ | DCA_MATRIX_MAC_OPCODE_LSU2_REQ;
static const int DCA_MAC_ADD = DCA_MATRIX_MAC_OPCODE_ADDSUB | _DCA_MAC_ALL_FROM_MEMORY;
static const int DCA_MAC_SUB = DCA_MATRIX_MAC_OPCODE_ADDSUB | DCA_MATRIX_MAC_OPCODE_RSRC_INV | _DCA_MAC_ALL_FROM_MEMORY;
static const int DCA_MAC_EWMULT = DCA_MATRIX_MAC_OPCODE_EWMULT | _DCA_MAC_ALL_FROM_MEMORY;
static const int DCA_MAC_SCALAR_MULT = DCA_MATRIX_MAC_OPCODE_EWMULT | DCA_MATRIX_MAC_OPCODE_RSRC_CONSTANT | _DCA_MAC_ALL_FROM_MEMORY;

static const int DCA_MAC_MULT_COND = DCA_MATRIX_MAC_OPCODE_MULT_COND | _DCA_MAC_ALL_FROM_MEMORY;
static const int DCA_MAC_MULT = DCA_MAC_MULT_COND | DCA_MATRIX_MAC_OPCODE_INIT_ACC;
//static const int DCA_MULT_ACC = DCA_MAC_MULT_COND | DCA_MATRIX_MAC_OPCODE_LOAD_ACC;

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
} dca_matrix_mac_hwpara_t;

typedef struct
{
  const ervp_mmiox1_hwinfo_t *mmiox_info;
  unsigned int num_row : 16;
  unsigned int num_col : 16;
  uint32_t id;
} dca_matrix_mac_hwinfo_t;

void dca_matrix_mac_hwinfo_elaborate(dca_matrix_mac_hwpara_t *hwpara, dca_matrix_mac_hwinfo_t *hwinfo);

static inline ervp_hwtask_busy_fx_t dca_matrix_mac_busy_fx(const dca_matrix_mac_hwinfo_t *const hwinfo)
{
  return hwinfo->mmiox_info->busy_fx;
}

static inline void dca_matrix_mac_wait(const dca_matrix_mac_hwinfo_t *const hwinfo)
{
  hwtask_wait_complete(dca_matrix_mac_busy_fx(hwinfo));
}

ervp_hwtask_busy_fx_t dca_matrix_mac_start(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, int opcode, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value);

static inline ervp_hwtask_busy_fx_t dca_matrix_add(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  return dca_matrix_mac_start(mop_mapping, hwinfo, DCA_MAC_ADD, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_matrix_sub(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  return dca_matrix_mac_start(mop_mapping, hwinfo, DCA_MAC_SUB, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_matrix_ewmult(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  return dca_matrix_mac_start(mop_mapping, hwinfo, DCA_MAC_EWMULT, ma_info, mb_info, mc_info, option_value);
}

static inline ervp_hwtask_busy_fx_t dca_matrix_mult(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, const ErvpMatrixInfo *mb_info, ErvpMatrixInfo *mc_info, unsigned int option_value)
{
  return dca_matrix_mac_start(mop_mapping, hwinfo, DCA_MAC_MULT, ma_info, mb_info, mc_info, option_value);
}

ervp_hwtask_busy_fx_t dca_matrix_scalar_mult_fixed(ervp_mop_mapping_t *mop_mapping, const dca_matrix_mac_hwinfo_t *const hwinfo, const ErvpMatrixInfo *ma_info, int scalar_value, ErvpMatrixInfo *mc_info, unsigned int option_value);

#endif