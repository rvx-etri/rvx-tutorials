#ifndef __PACT_CACHED_FUNCTION_H__
#define __PACT_CACHED_FUNCTION_H__

#include "pact_function.h"

struct _PactCachedFunctionInfo
{
	const PactFx* fx;
  int start_index;
  int size;
};

typedef struct _PactCachedFunctionInfo PactCfx;

PactCfx* pact_fx_allocate(const PactFx* fx);
void pact_cfx_execute(const PactCfx* cfx);
void pact_cfx_config_memory_access(const PactCfx* cfx, int access_index, const ErvpMatrixInfo* info);

#endif
