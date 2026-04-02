#include "ervp_dma_api.h"

void __dma_insert_info(unsigned int baseaddr, const ErvpDmaInfo* src, const ErvpDmaInfo* dst)
{
  mmfifo_wait_unitl_ready(baseaddr+MMAP_OFFSET_DMA_SRC_ADDR, 1);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_SRC_ADDR, (unsigned int)src->addr);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_SRC_STRIDE, src->stride);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_LINE_INFO, ((src->line_size<<16)|src->num_lines));
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_DST_ADDR, (unsigned int)dst->addr);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_DST_STRIDE, dst->stride);  
}

void __dma_insert_matrix_info(unsigned int baseaddr, const ErvpMatrixInfo* src, const ErvpMatrixInfo* dst)
{
  int line_size;
  line_size = (src->num_row) << matrix_datatype_get_addr_lsa(src->datatype);
  mmfifo_wait_unitl_ready(baseaddr+MMAP_OFFSET_DMA_SRC_ADDR, 1);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_SRC_ADDR, (unsigned int)src->addr);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_SRC_STRIDE, matrix_get_stride(src));
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_LINE_INFO, ((line_size<<16)|src->num_col));
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_DST_ADDR, (unsigned int)dst->addr);
  mmio_write_data(baseaddr+MMAP_OFFSET_DMA_DST_STRIDE, matrix_get_stride(dst));
}
