#ifndef __ERVP_DMA_API_H__
#define __ERVP_DMA_API_H__

#include "platform_info.h"
#include "ervp_dma_info.h"
#include "dma_tempalte_api.h"

#ifdef INCLUDE_DMA

static inline void dma_start()
{
  __dma_start(I_DMA_CONTROL_BASEADDR);
}

static inline int dma_get_status()
{
  return __dma_get_status(I_DMA_CONTROL_BASEADDR);
}

static inline void dma_wait_until_finished()
{
  __dma_wait_until_finished(I_DMA_CONTROL_BASEADDR);
}

static inline void dma_set_periodic(int enable, int period_us)
{
  __dma_set_periodic(I_DMA_CONTROL_BASEADDR, enable, period_us);
}

static inline void dma_insert_info(const ErvpDmaInfo* src, const ErvpDmaInfo* dst)
{
  __dma_insert_info(I_DMA_CONTROL_BASEADDR, src, dst);
}

static inline void dma_insert_matrix_info(const ErvpMatrixInfo* src, const ErvpMatrixInfo* dst)
{
  __dma_insert_matrix_info(I_DMA_CONTROL_BASEADDR, src, dst);
}

#else

static inline void dma_start(){}
static inline int dma_get_status(){ return 0; }
static inline void dma_wait_until_finished(){}
static inline void dma_set_periodic(int enable, int period_us){}
static inline void dma_insert_info(const ErvpDmaInfo* src, const ErvpDmaInfo* dst){}
static inline void dma_insert_matrix_info(const ErvpMatrixInfo* src, const ErvpMatrixInfo* dst){}

#endif

#endif
