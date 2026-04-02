#include "ervp_traffic_geneator_api.h"
#include "platform_info.h"

#ifdef INCLUDE_TRAFFIC_GENERATOR

void traffic_stop(int index)
{
  if(index < NUM_TRAFFIC_GENERATOR)
  {
    unsigned int baseaddr = I_TRAFFIC_GENERATOR00_CONTROL_BASEADDR;
    baseaddr += (I_TRAFFIC_GENERATOR00_CONTROL_SIZE*index);
    __dma_set_periodic(baseaddr, 0,0);
    __dma_wait_until_finished(baseaddr);
  }
}

void traffic_generate(int index, int period_us, const ErvpDmaInfo* src, const ErvpDmaInfo* dst)
{
  if(index < NUM_TRAFFIC_GENERATOR)
  {
    unsigned int baseaddr = I_TRAFFIC_GENERATOR00_CONTROL_BASEADDR;
    baseaddr += (I_TRAFFIC_GENERATOR00_CONTROL_SIZE*index);
    traffic_stop(index);
    __dma_set_periodic(baseaddr, 1,period_us);
    __dma_insert_info(baseaddr, src, dst);
    __dma_start(baseaddr);
  }
}

#else

void traffic_stop(int index){}
void traffic_generate(int index, int period_us, const ErvpDmaInfo* src, const ErvpDmaInfo* dst){}

#endif
