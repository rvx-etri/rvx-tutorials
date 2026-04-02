#ifndef __ERVP_MMIOX1_H__
#define __ERVP_MMIOX1_H__

#include "ervp_hwtask.h"
#include "ervp_mmiox1_raw.h"

typedef struct
{
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
} ervp_mmiox1_hwpara_t;

typedef struct
{
  mmio_addr_t baseaddr;
  ervp_hwtask_busy_fx_t busy_fx;
  unsigned int size_of_config : 16;
  unsigned int size_of_inst : 16;
  unsigned int size_of_log : 16;
  unsigned int size_of_status : 16;
  unsigned int size_of_input : 16;
  unsigned int size_of_output : 16;
} ervp_mmiox1_hwinfo_t;

void mmiox1_hwinfo_elaborate(ervp_mmiox1_hwpara_t *hwpara, ervp_mmiox1_hwinfo_t *hwinfo);

void mmiox1_print_info(const ervp_mmiox1_hwinfo_t *ip_info);

static inline void mmiox1_config_write(const ervp_mmiox1_hwinfo_t *const hwinfo, const mmio_struct_t *config)
{
  mmiox1_raw_config_write(hwinfo->baseaddr, config, hwinfo->size_of_config);
}

static inline void mmiox1_config_read(const ervp_mmiox1_hwinfo_t *const hwinfo, mmio_struct_t *config)
{
  mmiox1_raw_config_read(hwinfo->baseaddr, config, hwinfo->size_of_config);
}

static inline void mmiox1_inst_push(const ervp_mmiox1_hwinfo_t *const hwinfo, const mmio_struct_t *inst, int num_inst, int enable_itr)
{
  mmiox1_raw_inst_push(hwinfo->baseaddr, inst, num_inst, hwinfo->size_of_inst, enable_itr);
}

static inline mmio_data_t mmiox1_itr_get_status(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_itr_get_status(hwinfo->baseaddr);
}

static inline void mmiox1_itr_clear(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  mmiox1_raw_itr_clear(hwinfo->baseaddr);
}

static inline int mmiox1_inst_num_vacant(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_inst_num_vacant(hwinfo->baseaddr);
}

static inline void mmiox1_inst_wait_vacanct(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  while (mmiox1_inst_num_vacant(hwinfo) == 0)
    ;
}

static inline int mmiox1_inst_num_busy(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_inst_num_busy(hwinfo->baseaddr);
}

static inline int mmiox1_inst_is_busy(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_inst_is_busy(hwinfo->baseaddr);
}

static inline int mmiox1_core_has_log(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_core_has_log(hwinfo->baseaddr);
}

static inline void mmiox1_core_log_read(const ervp_mmiox1_hwinfo_t *const hwinfo, mmio_struct_t *log)
{
  mmiox1_raw_core_log_read(hwinfo->baseaddr, log, hwinfo->size_of_log);
}

static inline void mmiox1_core_status_read(const ervp_mmiox1_hwinfo_t *const hwinfo, mmio_struct_t *status)
{
  mmiox1_raw_core_status_read(hwinfo->baseaddr, status, hwinfo->size_of_status);
}

static inline void mmiox1_core_clear_request(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  mmiox1_raw_core_clear_request(hwinfo->baseaddr);
}

static inline int mmiox1_core_is_clear_finished(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_core_is_clear_finished(hwinfo->baseaddr);
}

static inline void mmiox1_core_clear(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  mmiox1_raw_core_clear(hwinfo->baseaddr);
}

static inline void mmiox1_inst_wait_busy(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  mmiox1_raw_inst_wait_busy(hwinfo->baseaddr);
}

static inline int mmiox1_input_num_fifo(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_input_num_fifo(hwinfo->baseaddr);
}

static inline int mmiox1_output_num_fifo(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  return mmiox1_raw_output_num_fifo(hwinfo->baseaddr);
}

static inline void mmiox1_input_push(const ervp_mmiox1_hwinfo_t *const hwinfo, const mmio_struct_t *data, int num_data)
{
  mmiox1_raw_input_push(hwinfo->baseaddr, data, num_data, hwinfo->size_of_input);
}

static inline void mmiox1_output_pop(const ervp_mmiox1_hwinfo_t *const hwinfo, mmio_struct_t *data, int num_data)
{
  mmiox1_raw_output_pop(hwinfo->baseaddr, data, num_data, hwinfo->size_of_output);
}

static inline void mmiox1_output_clear(const ervp_mmiox1_hwinfo_t *const hwinfo)
{
  mmiox1_raw_output_clear(hwinfo->baseaddr, hwinfo->size_of_output);
}

#endif