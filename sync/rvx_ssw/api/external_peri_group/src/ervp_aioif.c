#include "ervp_external_peri_group_api.h"

#include "ervp_aioif.h"
#include "ervp_mmio_util.h"

void aio_set_type(unsigned int index, unsigned int type)
{
  uint32_t value;
  value = 1 << (index+16);
  value |= type;
  mmio_write_data(MMAP_SPIO_AIOIF_CONFIG, value);
}

