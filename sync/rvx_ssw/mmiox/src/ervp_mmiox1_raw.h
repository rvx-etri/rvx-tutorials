#ifndef __ERVP_MMIOX1_RAW_H__
#define __ERVP_MMIOX1_RAW_H__

#include "ervp_mmio_util.h"
#include "ervp_round_int.h"
#include "ervp_core_id.h"
#include "ervp_mmiox1_memorymap_offset.h"

static inline void mmiox1_raw_config_write(mmio_addr_t mmiox1_maddr, const mmio_struct_t *config, int size_of_config)
{
  mmio_write_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_CORE_CONFIG_SAWD, size_of_config, config);
}

static inline void mmiox1_raw_config_read(mmio_addr_t mmiox1_maddr, mmio_struct_t *config, int size_of_config)
{
  mmio_read_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_CORE_CONFIG_SAWD, size_of_config, config);
}

static inline void mmiox1_raw_inst_push(mmio_addr_t mmiox1_maddr, const mmio_struct_t *inst, int num_inst, int size_of_inst, int enable_itr)
{
  int size_in_4byte = rshift_ru(size_of_inst, 2) * num_inst;
  if (enable_itr)
    mmio_write_data(mmiox1_maddr + MMAP_OFFSET_MMIO_ITR_REQUEST, (1 << EXCLUSIVE_ID));
  mmio_write_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_INST_FIFO_SAWD, size_in_4byte << 2, inst);
}

static inline mmio_data_t mmiox1_raw_itr_get_status(mmio_addr_t mmiox1_maddr)
{
  return mmio_read_data(mmiox1_maddr + MMAP_OFFSET_MMIO_ITR_STATUS);
}

static inline void mmiox1_raw_itr_clear(mmio_addr_t mmiox1_maddr)
{
  mmio_write_data(mmiox1_maddr + MMAP_OFFSET_MMIO_ITR_STATUS, (1 << EXCLUSIVE_ID));
}

typedef union
{
  mmio_data_t value;
  struct
  {
    unsigned int num_vacant : 8;
    unsigned int num_busy : 8;
    unsigned int has_log : 1;
  } br;
} mmiox1_inst_status_t;

static inline mmio_data_t __mmiox1_raw_get_inst_status(mmio_addr_t mmiox1_maddr)
{
  return mmio_read_data(mmiox1_maddr + MMAP_OFFSET_MMIO_INST_STATUS);
}

static inline int mmiox1_raw_inst_num_vacant(mmio_addr_t mmiox1_maddr)
{
  mmiox1_inst_status_t status;
  status.value = __mmiox1_raw_get_inst_status(mmiox1_maddr);
  return status.br.num_vacant;
}

static inline int mmiox1_raw_inst_num_busy(mmio_addr_t mmiox1_maddr)
{
  mmiox1_inst_status_t status;
  status.value = __mmiox1_raw_get_inst_status(mmiox1_maddr);
  return status.br.num_busy;
}

static inline int mmiox1_raw_inst_is_busy(mmio_addr_t mmiox1_maddr)
{
  return (mmiox1_raw_inst_num_busy(mmiox1_maddr) > 0);
}

static inline int mmiox1_raw_core_has_log(mmio_addr_t mmiox1_maddr)
{
  mmiox1_inst_status_t status;
  status.value = __mmiox1_raw_get_inst_status(mmiox1_maddr);
  return status.br.has_log;
}

static inline void mmiox1_raw_core_log_read(mmio_addr_t mmiox1_maddr, mmio_struct_t *log, int size_of_log)
{
  mmio_read_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_LOG_FIFO_SAWD, size_of_log, log);
}

static inline void mmiox1_raw_core_status_read(mmio_addr_t mmiox1_maddr, mmio_struct_t *status, int size_of_status)
{
  mmio_read_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_CORE_STATUS_SAWD, size_of_status, status);
}

static inline void mmiox1_raw_core_clear_request(mmio_addr_t mmiox1_maddr)
{
  mmio_write_data(mmiox1_maddr + MMAP_OFFSET_MMIO_CORE_CLEAR, 0);
}

static inline int mmiox1_raw_core_is_clear_finished(mmio_addr_t mmiox1_maddr)
{
  return (mmio_read_data(mmiox1_maddr + MMAP_OFFSET_MMIO_CORE_CLEAR) == 0);
}

static inline void mmiox1_raw_core_clear(mmio_addr_t mmiox1_maddr)
{
  mmiox1_raw_core_clear_request(mmiox1_maddr);
  while (mmiox1_raw_core_is_clear_finished(mmiox1_maddr) == 0)
    ;
}

static inline void mmiox1_raw_inst_wait_busy(mmio_addr_t mmiox1_maddr)
{
  while (mmiox1_raw_inst_is_busy(mmiox1_maddr))
    ;
}

typedef union
{
  mmio_data_t value;
  struct
  {
    unsigned int num_valid_output : 16;
    unsigned int num_vacant_input : 16;
  } br;
} mmiox1_fifo_status_t;

static inline mmio_data_t __mmiox1_raw_get_fifo_status(mmio_addr_t mmiox1_maddr)
{
  return mmio_read_data(mmiox1_maddr + MMAP_OFFSET_MMIO_FIFO_STATUS);
}

static inline int mmiox1_raw_input_num_fifo(mmio_addr_t mmiox1_maddr)
{
  mmiox1_fifo_status_t status;
  status.value = __mmiox1_raw_get_fifo_status(mmiox1_maddr);
  return status.br.num_vacant_input;
}

static inline int mmiox1_raw_output_num_fifo(mmio_addr_t mmiox1_maddr)
{
  mmiox1_fifo_status_t status;
  status.value = __mmiox1_raw_get_fifo_status(mmiox1_maddr);
  return status.br.num_valid_output;
}

static inline void mmiox1_raw_input_push(mmio_addr_t mmiox1_maddr, const mmio_struct_t *data, int num_data, int size_of_data)
{
  int size_in_4byte = rshift_ru(size_of_data, 2) * num_data;
  mmio_write_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_INPUT_FIFO_SAWD, size_in_4byte << 2, data);
}

static inline void mmiox1_raw_output_pop(mmio_addr_t mmiox1_maddr, mmio_struct_t *data, int num_data, int size_of_data)
{
  int size_in_4byte = rshift_ru(size_of_data, 2) * num_data;
  mmio_read_struct(mmiox1_maddr + MMAP_OFFSET_MMIO_OUTPUT_FIFO_SAWD, size_in_4byte << 2, data);
}

static inline void mmiox1_raw_output_clear(mmio_addr_t mmiox1_maddr, int size_of_data)
{
  int num_data = mmiox1_raw_output_num_fifo(mmiox1_maddr);
  int size_in_4byte = rshift_ru(size_of_data, 2) * num_data;
  for (int i = 0; i < size_in_4byte; i++)
    mmio_read_data(mmiox1_maddr + MMAP_OFFSET_MMIO_OUTPUT_FIFO_SAWD);
}

#endif
