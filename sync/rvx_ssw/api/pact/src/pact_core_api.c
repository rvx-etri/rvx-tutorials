#include "ervp_malloc.h"
#include "core_dependent.h"

#include "pact.h"
#include "paco.h"

void pact_core0_init()
{
  pact_core0_start(paco_init);
  pact_core0_wait();
}

void pact_core0_start(void* start_addr)
{
  pact_insert_inst(pact_inst_core0_startaddr((unsigned int)start_addr),0);
  pact_insert_inst(pact_inst_core0_active(),0);
  pact_insert_inst(pact_inst_finish(),0);
  pact_start();
}

void pact_core0_wait()
{
  pact_insert_inst(pact_inst_core0_wait(),0);
  pact_insert_inst(pact_inst_finish(),0);
  pact_start();
  pact_wait_until_finished();
}
