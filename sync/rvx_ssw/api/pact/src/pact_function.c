#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_bit_util.h"
#include "ervp_variable_allocation.h"

#include "pact_function.h"
#include "pact_ctrl_api.h"

#define NUM_CHUCK 100
#define NUM_FX 100

static PactFx preallocated_chunk_list[NUM_CHUCK] CACHED_DATA;
static PactLfx preallocated_fx_list[NUM_FX] CACHED_DATA;

volatile int chuck_index_to_alloc = 0;
volatile int fx_index_to_alloc = 0;

void pact_fx_print(const PactFx* fx)
{
	const PactFx* current = fx;
	while(1)
	{
		if(current==0)
			break;
		printf("\nc%x %d %d:", current, current->num_inst, current->num_access);
		for(int i=0; i<current->num_access; i++)
			printf(" %d", current->access2inst[i]);
		current = current->next;
	}
}

static inline void pact_fx_init(PactFx* fx)
{
	fx->next = 0;
	fx->num_inst = 0;
	fx->num_access = 0;
	fx->num_reconfig = 0;
	fx->is_reconfigure_list = 0;
	for(int i=0; i<NUM_ACCESS_PER_PACT_CHUCK; i++)
		fx->access2inst[i] = -1;
}

PactFx* pact_fx_alloc()
{
	PactFx* result;
	if(chuck_index_to_alloc>=NUM_CHUCK)
		assert(0); // All chucks are occupied
	result = &(preallocated_chunk_list[chuck_index_to_alloc++]);
	pact_fx_init(result);
	return result;
}

static inline PactFx* get_chunk_to_insert_with_extension(PactFx* fx)
{
	PactFx* result;
	PactFx* last_chunk;
	last_chunk = fx;
	while(1)
	{
		if(last_chunk->next==0)
			break;
		last_chunk = last_chunk->next;
	}
	if(last_chunk->num_inst==NUM_INST_PER_PACT_CHUNK)
	{
		last_chunk->next = pact_fx_alloc();
		result = last_chunk->next;
	}
	else
		result = last_chunk;
	return result;
}

void pact_fx_insert_inst(PactFx* fx, const PactInstBinary inst)
{
	PactFx* fx_real = get_chunk_to_insert_with_extension(fx);
	fx_real->inst[fx_real->num_inst++] = inst;
}

static inline void __pact_insert_lsu0_config(PactFx* fx, const ErvpMatrixInfo* info)
{
	PactAccessConfig config;
	unsigned int* p = (unsigned int*)&config;

	generate_pact_access_config(info, &config);
	PactFx* fx_real = get_chunk_to_insert_with_extension(fx);
	assert(fx_real->num_access<=NUM_ACCESS_PER_PACT_CHUCK); // Too many access in one chunk
	fx_real->access2inst[fx_real->num_access++] = fx_real->num_inst;
	for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
		pact_fx_insert_inst(fx_real, pact_inst_lsu0_info(p[i]));
}

void pact_fx_insert_lsu0_load(PactFx* fx, const ErvpMatrixInfo* info)
{
	__pact_insert_lsu0_config(fx, info);
	pact_fx_insert_inst(fx, pact_inst_lsu0_load());
}

void pact_fx_insert_lsu0_store(PactFx* fx, const ErvpMatrixInfo* info)
{
	__pact_insert_lsu0_config(fx, info);
	pact_fx_insert_inst(fx, pact_inst_lsu0_store());
}

static inline void __pact_insert_lsu1_config(PactFx* fx, const ErvpMatrixInfo* info)
{
	PactAccessConfig config;
	unsigned int* p = (unsigned int*)&config;

	generate_pact_access_config(info, &config);
	PactFx* fx_real = get_chunk_to_insert_with_extension(fx);
	assert(fx_real->num_access<=NUM_ACCESS_PER_PACT_CHUCK); // Too many access in one chunk
	fx_real->access2inst[fx_real->num_access++] = fx_real->num_inst;
	for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
		pact_fx_insert_inst(fx_real, pact_inst_lsu1_info(p[i]));
}

void pact_fx_insert_lsu1_load(PactFx* fx, const ErvpMatrixInfo* info)
{
	__pact_insert_lsu1_config(fx, info);
	pact_fx_insert_inst(fx, pact_inst_lsu1_load());
}

void pact_fx_insert_lsu1_store(PactFx* fx, const ErvpMatrixInfo* info)
{
	__pact_insert_lsu1_config(fx, info);
	pact_fx_insert_inst(fx, pact_inst_lsu1_store());
}

void pact_fx_insert_reconfigure_inst(PactFx* fx)
{
	PactFx* fx_real = get_chunk_to_insert_with_extension(fx);
	fx_real->is_reconfigure_list = set_bits_by_index(fx_real->is_reconfigure_list, fx_real->num_inst, fx_real->num_inst);
	fx_real->num_inst++;
	fx_real->num_reconfig++;
}

void pact_fx_set_reconfigure_inst(PactFx* fx, int reconfig_index, const PactInstBinary inst)
{
	if(reconfig_index>=fx->num_reconfig)
		pact_fx_set_reconfigure_inst(fx->next, reconfig_index-fx->num_reconfig, inst);
	else
	{
		unsigned int shifted = fx->is_reconfigure_list;
		int current_index = 0;
		for(int i=0; i<32; i++)
		{
			unsigned int is_reconfigure = shifted&1;
			if(is_reconfigure)
			{
				if(reconfig_index==current_index)
				{
					pact_fx_set_inst(fx, i, inst);
					break;
				}
				else
					current_index++;
			}
			shifted>>=1;
		}
	}
}

//

void pact_fx_set_inst(PactFx* fx, int inst_index, const PactInstBinary inst)
{
	if(inst_index>=NUM_INST_PER_PACT_CHUNK)
	{
		assert(fx->next!=0); // Wrong next
		pact_fx_set_inst(fx->next, inst_index-NUM_INST_PER_PACT_CHUNK, inst);
	}
	else
	{
		assert(inst_index<fx->num_inst); // Wrong inst_index
		fx->inst[inst_index] = inst;
	}
}

PactInstBinary pact_fx_get_inst(const PactFx* fx, int inst_index)
{
	PactInstBinary result;
	if(inst_index>=NUM_INST_PER_PACT_CHUNK)
	{
		assert(fx->next!=0); // Wrong next
		result = pact_fx_get_inst(fx->next, inst_index-NUM_INST_PER_PACT_CHUNK);
	}
	else
	{
		assert(inst_index<fx->num_inst); // Wrong inst_index
		result = fx->inst[inst_index];
	}
	return result;
}

void pact_fx_config_memory_access(PactFx* fx, int access_index, const ErvpMatrixInfo* info)
{
	if(access_index>=fx->num_access)
		pact_fx_config_memory_access(fx->next, access_index-fx->num_access, info);
	else
	{
		int inst_index = fx->access2inst[access_index];
		PactAccessConfig config;
		unsigned int* p = (unsigned int*)&config;
		assert(inst_index>=0); // Wrong access_index
		//
		generate_pact_access_config(info, &config);
		for(int i=0; i<PACT_ACCESS_CONFIG_SIZE_4BYTE; i++)
		{
			PactImmeInst inst;
			inst.binary = pact_fx_get_inst(fx, inst_index+i);
			inst.isa.imme.int_value = p[i];
			pact_fx_set_inst(fx, inst_index+i, inst.binary);
		}
	}
}

//

void _pact_fx_insert(const PactFx* fx, int force)
{
	const PactFx* current_chunk = fx;
	while(1)
	{
		pact_insert_inst_list(current_chunk->inst, current_chunk->num_inst, force);
		current_chunk = current_chunk->next;
		if(current_chunk==0)
			break;
	}
}

void pact_fx_execute(const PactFx* fx)
{
	//printf("\n pf %04x:", fx);
	pact_start();
	_pact_fx_insert(fx,0);
	pact_insert_inst(pact_inst_finish(),0);
}

///////////////////////////////////////////////////////////////////////////////

static inline void pact_lfx_init(PactLfx* lfx)
{
	lfx->fx = 0;
	lfx->next = 0;
}

PactLfx* pact_lfx_alloc()
{
	PactLfx* result;
	if(fx_index_to_alloc>=NUM_FX)
		assert(0); // All fxs are occupied
	result = &(preallocated_fx_list[fx_index_to_alloc++]);
	pact_lfx_init(result);
	return result;
}

void pact_lfx_insert_fx(PactLfx* lfx, const PactFx* fx)
{
	PactLfx* current = lfx;
	while(1)
	{
		if(current->next==0)
		{
			if(current->fx==0)
			{
				current->fx = fx;
				break;
			}
			else
				current->next = pact_lfx_alloc();
		}
		current = current->next;
	}
}

void pact_lfx_execute(const PactLfx* lfx)
{
	const PactLfx* current;
	//printf("\nlfx %04x", lfx);
	pact_start();
	current = lfx;
	while(1)
	{
		if(current==0)
			break;
		//printf("\n fx %04x", current->fx);
		_pact_fx_insert(current->fx, 0);
		current = current->next;
	}
	pact_insert_inst(pact_inst_finish(),0);
}

void pact_lfx_print(const PactLfx* lfx)
{
	const PactLfx* current = lfx;
	while(1)
	{
		if(current==0)
			break;
		printf("\nf%04x", current->fx);
		current = current->next;
	}
}
