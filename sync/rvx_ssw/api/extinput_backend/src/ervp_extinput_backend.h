#ifndef __ERVP_EXTINPUT_BACKEND_H__
#define __ERVP_EXTINPUT_BACKEND_H__

#include "ervp_mmiox1.h"

typedef ervp_mmiox1_hwinfo_t ervp_extinput_hwinfo_t;

typedef struct
{
  unsigned int num_interval_m1;
  unsigned int dma_write_addr;
  unsigned int num_input_m1 : 8;
  unsigned int enable_by_external_valid : 1;
  unsigned int enable_by_tick1us_interval : 1;
  unsigned int enable_by_clock_interval : 1;
  unsigned int dma_enable : 1;
  unsigned int infinite_repeat : 1;
  // unsigned int dma_axi_size : 3;
} extinput_inst_t;

static inline void extinput_inst_init(const extinput_inst_t *inst)
{
  int *p = (int *)inst;
  int size = ((sizeof(extinput_inst_t) + 1) / sizeof(int)) - 1;
  for (int i = 0; i < size; i++)
    p[i] = 0;
}

static inline void extinput_inst_push(const ervp_mmiox1_hwinfo_t *const hwinfo, const extinput_inst_t *inst, int enable_itr)
{
  mmiox1_inst_push(hwinfo, inst, 1, enable_itr);
}

#define extinput_config_write mmiox1_config_write
#define extinput_status_read mmiox1_core_status_read
#define extinput_inst_num_fifo mmiox1_inst_num_fifo
#define extinput_is_busy mmiox1_inst_is_busy
#define extinput_wait_busy mmiox1_inst_wait_busy
#define extinput_clear_request mmiox1_core_clear_request
#define extinput_is_clear_finished mmiox1_core_is_clear_finished
#define extinput_clear mmiox1_core_clear
#define extinput_has_log mmiox1_core_has_log
#define extinput_data_clear mmiox1_output_clear
#define extinput_data_pop mmiox1_output_pop

#endif