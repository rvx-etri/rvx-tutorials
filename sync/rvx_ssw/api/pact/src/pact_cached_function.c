#include "ervp_variable_allocation.h"
#include "ervp_mmio_util.h"
#include "ervp_assert.h"

#include "pact.h"

#define NUM_CFX 10
static PactCfx preallocated_cfx_list[NUM_CFX] CACHED_DATA;

volatile int cfx_index_to_alloc = 0;

PactCfx* pact_fx_allocate(const PactFx* fx)
{
  PactCfx* result;
	if(cfx_index_to_alloc>=NUM_CFX)
		assert(0); // All cfxs are occupied
	result = &(preallocated_cfx_list[cfx_index_to_alloc++]);
  result->fx = fx;
  result->start_index = mmio_read_data(MMAP_PACT_INST_MEM_WINDEX);
  _pact_fx_insert(fx, 1);
  pact_insert_inst(pact_inst_finish(),1);
  result->size = mmio_read_data(MMAP_PACT_INST_MEM_WINDEX) - result->start_index;
  return result;
}

void pact_cfx_execute(const PactCfx* cfx)
{
  pact_start_mem(cfx->start_index, cfx->size);
}

void pact_cfx_config_memory_access(const PactCfx* cfx, int access_index, const ErvpMatrixInfo* info)
{
  const PactFx* fx = cfx->fx;
  int remaining_access_index = access_index;
  int inst_index = 0;
  int inst_index_offset;
  while(1)
  {
    if(remaining_access_index>=fx->num_access)
    {
      inst_index += fx->num_inst;
      remaining_access_index -= fx->num_access;
      fx = fx->next;
    }
    else
    {
      inst_index_offset = fx->access2inst[remaining_access_index];
      inst_index += inst_index_offset;
      break;
    }
  }
  int windex = cfx->start_index + inst_index;
  mmio_write_data(MMAP_PACT_INST_MEM_WINDEX, windex);
  //
  PactAccessConfig config;
  unsigned int* p = (unsigned int*)&config;
  generate_pact_access_config(info, &config);
  for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
  {
    PactImmeInst inst;
    inst.binary = pact_fx_get_inst(fx, inst_index_offset+i);
    inst.isa.imme.int_value = p[i];
    pact_insert_inst(inst.binary, 1);
  }
}
