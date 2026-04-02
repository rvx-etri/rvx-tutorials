#ifndef __ERVP_DMA_INFO_H__
#define __ERVP_DMA_INFO_H__

typedef struct {
	void *addr;
	unsigned int stride;
	unsigned int line_size;
	unsigned int num_lines;
} ErvpDmaInfo;

#endif
