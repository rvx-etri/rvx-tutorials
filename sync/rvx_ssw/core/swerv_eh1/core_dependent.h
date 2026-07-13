#ifndef __CORE_DEPENDENT_H__
#define __CORE_DEPENDENT_H__

#include "platform_info.h"

#include "riscv_isa.h"
#include "csr_encoding.h"
#include "ervp_reg_util.h"
#include "ervp_interrupt.h"

#define BIG_ENDIAN_ACCESS 0

static inline int get_local_id()
{
  int result;
  if (NUM_CORE_PER_CORECLUSTER == 1)
    result = 0;
  else
  {
    result = get_internal_core_id();
#ifdef VP_ENV
    switch (NUM_CORE_PER_CORECLUSTER)
    {
    case 2:
      result &= 0x1;
      break;
    case 4:
      result &= 0x3;
      break;
    default:
      while (1)
        ;
    }
#endif // VP_ENV
  }
  return result;
}

static inline void flush_cache()
{
  asm volatile("fence.i");
}

// static inline void flush_dcache(){
//	asm volatile ("fence");
// }

static inline unsigned int get_interrupt()
{
  return ((read_csr(mstatus) & MSTATUS_MIE) > 0);
}

static inline void enable_interrupt()
{
  // Enable interrupts in general.
  set_csr(mstatus, MSTATUS_MIE);
}

static inline void disable_interrupt()
{
  // Enable interrupts in general.
  clear_csr(mstatus, MSTATUS_MIE);
}

static inline void allow_interrupt_timer()
{
  // Enable the Machine-Timer bit in MIE
  set_csr(mie, MIP_MTIP);
}

static inline void disallow_interrupt_timer()
{
  // Disable the Machine-Timer bit in MIE
  clear_csr(mie, MIP_MTIP);
}

static inline void allow_interrupt_plic()
{
  check_plic_grant();
  // Enable the Machine-External bit in MIE
  set_csr(mie, MIP_MEIP);
}

static inline void disallow_interrupt_plic()
{
  // Disable the Machine-External bit in MIE
  clear_csr(mie, MIP_MEIP);
}

static inline void init_core()
{
  write_csr(mie, 0);
}
static inline void init_cache() {}

#endif
