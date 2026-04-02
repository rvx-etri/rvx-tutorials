#include <stdint.h>
#include "platform_info.h"
#include "ervp_memory_dump.h"
#include "ervp_external_peri_group_api.h"
#include "ervp_platform_controller_api.h"
#include "ervp_assert.h"
#include "core_dependent.h"

static int prepared_dump_type = MEMORY_DUMP_NONE;

void memory_dump_init()
{
  set_gpreg(0, MEMORY_DUMP_NONE);
  set_gpreg(1, 0);
  set_gpreg(2, 0);
}

void memory_dump_prepare(int dump_type, void *addr, int size)
{
  const char *dump_type_string;
  assert(prepared_dump_type == MEMORY_DUMP_NONE);
  assert((size & 3) == 0); // size error due to OCD requirement
  prepared_dump_type = dump_type;
  set_gpreg(0, (unsigned int)dump_type);
  set_gpreg(1, (unsigned int)addr);
  set_gpreg(2, (unsigned int)size);
  switch (dump_type)
  {
  case MEMORY_DUMP_FAKEFILE:
    dump_type_string = "fakefile";
    break;
  case MEMORY_DUMP_IMAGE:
    dump_type_string = "image";
    break;
  default:
    assert(0);
  }
  flush_cache();
  if (!is_sim())
  {
    printf_must("\nMemory dump is prepared (%s)", dump_type_string);
    printf_must("\ntry \'make dump\'");
  }
}
