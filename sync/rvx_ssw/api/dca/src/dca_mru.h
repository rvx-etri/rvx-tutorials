#ifndef __DCA_MRU_H__
#define __DCA_MRU_H__

#include "ervp_matrix_op.h"
#include "ervp_matrix_op_sw.h"
#include "ervp_mmiox1.h"
#include "dca_module_memorymap_offset.h"

static const int DCA_MRU_COPY = DCA_MRU_OPCODE_COPY | DCA_MRU_OPCODE_LSU0_REQ;
static const int DCA_MRU_TRANSPOSE = DCA_MRU_OPCODE_TRANSPOSE | DCA_MRU_OPCODE_LSU0_REQ;
static const int DCA_MRU_FILL = DCA_MRU_OPCODE_FILL | DCA_MRU_OPCODE_LSU0_REQ;

typedef struct
{
  unsigned int bw_addr;
  unsigned int mi_bw_data;
  unsigned int mi_has_burden;
  unsigned int mi_bw_burden;
  unsigned int mo_bw_data;
  unsigned int mo_has_burden;
  unsigned int mo_bw_burden;
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
} dca_mru_hwpara_t;

typedef struct
{
  const ervp_mmiox1_hwinfo_t *mmiox_info;
  int id;
} dca_mru_hwinfo_t;

void dca_mru_hwinfo_elaborate(dca_mru_hwpara_t *hwpara, dca_mru_hwinfo_t *hwinfo);

static inline ervp_hwtask_busy_fx_t dca_mru_busy_fx(const dca_mru_hwinfo_t *const hwinfo)
{
  return hwinfo->mmiox_info->busy_fx;
}

static inline void dca_mru_wait(const dca_mru_hwinfo_t *const hwinfo)
{
  hwtask_wait_complete(dca_mru_busy_fx(hwinfo));
}

ervp_hwtask_busy_fx_t dca_mru_copy_part(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, const ErvpMatrixInfo *mi_info, ErvpMatrixInfo *mo_info, int num_row, int num_col, unsigned int option_value);
ervp_hwtask_busy_fx_t dca_mru_transpose_part(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, const ErvpMatrixInfo *mi_info, ErvpMatrixInfo *mo_info, int num_row, int num_col, unsigned int option_value);
ervp_hwtask_busy_fx_t dca_mru_fill_fixed(ervp_mop_mapping_t *mop_mapping, const dca_mru_hwinfo_t *const hwinfo, ErvpMatrixInfo *result, int32_t value);

#endif