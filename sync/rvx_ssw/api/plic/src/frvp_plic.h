// See LICENSE file for licence details

#ifndef __FRVP_PLIC_H__
#define __FRVP_PLIC_H__

#include <stdint.h>

typedef unsigned int            uintptr_t;

typedef struct __plic_instance_t
{
  uintptr_t base_addr;

  uint32_t num_sources;
  uint32_t num_priorities;
  
} plic_instance_t;

typedef uint32_t plic_source;
typedef uint32_t plic_priority;
typedef uint32_t plic_threshold;

void PLIC_init (
                plic_instance_t * this_plic,
                uintptr_t         base_addr,
                uint32_t num_sources,
                uint32_t num_priorities
                );

void PLIC_set_threshold (plic_instance_t * this_plic,
			 plic_threshold threshold);
  
void PLIC_enable_interrupt (plic_instance_t * this_plic,
			    plic_source source);

void PLIC_disable_interrupt (plic_instance_t * this_plic,
			     plic_source source);
  
void PLIC_set_priority (plic_instance_t * this_plic,
			plic_source source,
			plic_priority priority);

plic_source PLIC_claim_interrupt(plic_instance_t * this_plic);

void PLIC_complete_interrupt(plic_instance_t * this_plic,
			     plic_source source);

#define PLIC_NUM_INTERRUPTS 32
#define PLIC_NUM_PRIORITIES 7

#endif

