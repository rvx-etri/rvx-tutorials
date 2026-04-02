#include "ervp_wifi.h"

void wifi_set_interrupt_pending(int pending)
{
	REG32(MMAP_SPIO_WIFI_ITR_PENDING) = pending;
}

int wifi_is_interrupt_pending()
{
	return REG32(MMAP_SPIO_WIFI_ITR_PENDING);
}
