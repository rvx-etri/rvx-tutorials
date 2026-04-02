#ifndef __ERVP_VDMA_H__
#define __ERVP_VDMA_H__

void configure_vdma(unsigned int src_addr, unsigned int dst_addr, unsigned int size,
		unsigned int count, unsigned int src_stride, unsigned int dst_stride,
		unsigned int period_us);
unsigned int wait_end_of_work();
void stop_vdma();
void start_once_vdma();
void start_periodic_vdma();

#endif
