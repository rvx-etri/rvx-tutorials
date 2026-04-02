#ifndef __PACT_FUNCTION_H__
#define __PACT_FUNCTION_H__

#include "pact_inst.h"
#include "ervp_matrix.h"

#define NUM_INST_PER_PACT_CHUNK 8
#define NUM_ACCESS_PER_PACT_CHUCK (((NUM_INST_PER_PACT_CHUNK-1)>>2)+1)

struct _PactChunkContainer
{
	int num_inst;
	int num_access;
	int num_reconfig;
	char access2inst[NUM_ACCESS_PER_PACT_CHUCK];
	unsigned int is_reconfigure_list;
	PactInstBinary inst[NUM_INST_PER_PACT_CHUNK];
	struct _PactChunkContainer* next;
};

typedef struct _PactChunkContainer PactFx;

void pact_fx_print(const PactFx* fx);

PactFx* pact_fx_alloc();
void pact_fx_insert_inst(PactFx* fx, const PactInstBinary inst);
void pact_fx_insert_lsu0_load(PactFx* fx, const ErvpMatrixInfo* info);
void pact_fx_insert_lsu0_store(PactFx* fx, const ErvpMatrixInfo* info);
void pact_fx_insert_lsu1_load(PactFx* fx, const ErvpMatrixInfo* info);
void pact_fx_insert_lsu1_store(PactFx* fx, const ErvpMatrixInfo* info);

void pact_fx_insert_reconfigure_inst(PactFx* fx);
void pact_fx_set_reconfigure_inst(PactFx* fx, int reconfig_index, const PactInstBinary inst);

void pact_fx_set_inst(PactFx* fx, int inst_index, const PactInstBinary inst);
PactInstBinary pact_fx_get_inst(const PactFx* fx, int inst_index);
void pact_fx_config_memory_access(PactFx* fx, int access_index, const ErvpMatrixInfo* info);

void _pact_fx_insert(const PactFx* fx, int force);
void pact_fx_execute(const PactFx* fx);


////////////////////////////////////////////////////////////////////////

struct _PactFunctionContainer
{
	const PactFx* fx;
	struct _PactFunctionContainer* next;
};

typedef struct _PactFunctionContainer PactLfx;

PactLfx* pact_lfx_alloc();
void pact_lfx_insert_fx(PactLfx* lfx, const PactFx* fx);
void pact_lfx_execute(const PactLfx* lfx);
void pact_lfx_print(const PactLfx* lfx);

#endif
