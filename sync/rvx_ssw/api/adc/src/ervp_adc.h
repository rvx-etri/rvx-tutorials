#ifndef __ERVP_ADC_H__
#define __ERVP_ADC_H__

#include <stdint.h>
#include "ervp_adc_memorymap_offset.h"
#include "ervp_mmio_util.h"

static inline void adc_set_config(uint32_t baseaddr, uint32_t config)
{
  mmio_write_data(baseaddr+MMAP_SUBOFFSET_ADC_ANALOG_CONFIG, config);
}

static inline uint32_t adc_get_config(uint32_t baseaddr)
{
  return mmio_read_data(baseaddr+MMAP_SUBOFFSET_ADC_ANALOG_CONFIG);
}

#endif
