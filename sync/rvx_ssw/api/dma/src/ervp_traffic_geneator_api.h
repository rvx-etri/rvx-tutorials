#ifndef __ERVP_TRAFFIC_GENERATOR_API_H__
#define __ERVP_TRAFFIC_GENERATOR_API_H__

#include "ervp_dma_api.h"

void traffic_generate(int index, int period_us, const ErvpDmaInfo* src, const ErvpDmaInfo* dst);
void traffic_stop(int index);

#endif
