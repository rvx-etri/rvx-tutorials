#include "platform_info.h"
#include "ervp_printf.h"
#include "ervp_reg_util.h"

#include "ervp_vdma_memorymap_offset.h"
#include "ervp_vdma.h"

#define VDMA_CTRL_BASEADDR      (0x98000000)

static inline int get_vdma_status()
{
	return REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_STATUS);
}

void configure_vdma(unsigned int src_addr, unsigned int dst_addr, unsigned int size,
		unsigned int count, unsigned int src_stride, unsigned int dst_stride,
		unsigned int period_us)
{
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_SRC_ADDR) = src_addr;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_DST_ADDR) = dst_addr;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_SIZE) = size;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_COUNT) = count;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_SRC_STRIDE) = src_stride;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_DST_STRIDE) = dst_stride;
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_PERIOD) = period_us;
}

unsigned int wait_end_of_work()
{
	unsigned int status;
	do {
		status = get_vdma_status();
	} while((status != VDMA_STATUS_IDLE) && (status != VDMA_STATUS_WAIT_PERIODIC));
	return status;
}

void stop_vdma()
{
	unsigned int status;
	status = wait_end_of_work();
	if(status == VDMA_STATUS_WAIT_PERIODIC)
		REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_CMD) = VDMA_CMD_STOP;
}

void start_once_vdma()
{
	stop_vdma();
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_CMD) = VDMA_CMD_ONCE;
}

void start_periodic_vdma()
{
	stop_vdma();
	REG32(VDMA_CTRL_BASEADDR + MMAP_SUBOFFSET_VDMA_CMD) = VDMA_CMD_PERIODIC;
}
