#ifndef __ERVP_WIFI_H__
#define __ERVP_WIFI_H__

#include "ervp_external_peri_group_memorymap.h"
#include "ervp_reg_util.h"

void wifi_set_interrupt_pending(int pending);
int wifi_is_interrupt_pending();

static inline wifi_clear_interrupt()
{
	REG32(MMAP_SPIO_WIFI_ITR_CLEAR) = 1;
}

#endif
