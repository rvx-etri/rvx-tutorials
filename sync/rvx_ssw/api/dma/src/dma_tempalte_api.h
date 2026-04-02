#ifndef __DMA_TEMPLATE_API_H__
#define __DMA_TEMPLATE_API_H__

#include "ervp_dma_memorymap_offset.h"
#include "ervp_mmio_util.h"
#include "ervp_dma_info.h"
#include "ervp_matrix.h"

static inline void __dma_start(unsigned int baseaddr)
{
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_START, 0);
}

static inline int __dma_get_status(unsigned int baseaddr)
{
  return mmio_read_data(baseaddr+MMAP_OFFSET_DMA_STATUS);
}

static inline void __dma_wait_until_finished(unsigned int baseaddr)
{
  while(__dma_get_status(baseaddr)!=DMA_STATUS_IDLE);
}

static inline void __dma_set_periodic(unsigned int baseaddr, int enable, int period_us)
{
  unsigned int data;
  data = period_us<<1;
  data |= (enable&1);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_PERIOD_INFO, data);
}

void __dma_insert_info(unsigned int baseaddr, const ErvpDmaInfo* src, const ErvpDmaInfo* dst);
void __dma_insert_matrix_info(unsigned int baseaddr, const ErvpMatrixInfo* src, const ErvpMatrixInfo* dst);

#endif
