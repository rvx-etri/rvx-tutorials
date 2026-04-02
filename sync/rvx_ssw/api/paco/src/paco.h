#ifndef __PACO_H__
#define __PACO_H__

#include "pact.h"
#include "csr_encoding.h"
#include "ervp_cache_memorymap_offset.h"
#include "riscv_memorymap_offset.h"

extern void paco_init(void);
extern void paco_stop(void);

static inline void paco_lsu_wait(void)
{
  int busy;
  while(1)
  {
    asm volatile ("csrr %0, %1":"=r"(busy):"i"(PACO_CSR_ADDR_LSU_BUSY));
    if(busy==0)
      break;
  }
}

static inline void paco_corecache_invalidate(void)
{
  asm volatile ("csrwi %0, %1"::"i"(PACO_CSR_ADDR_CORECACHE_COMMAND), "i"(CACHE_CONTROL_CMD_INVALIDATE));
}

static inline void paco_corecache_flush(void)
{
  asm volatile ("csrwi %0, %1"::"i"(PACO_CSR_ADDR_CORECACHE_COMMAND), "i"(CACHE_CONTROL_CMD_FLUSH));
}

static inline void paco_corecache_clean(void)
{
  asm volatile ("csrwi %0, %1"::"i"(PACO_CSR_ADDR_CORECACHE_COMMAND), "i"(CACHE_CONTROL_CMD_CLEAN));
}

static inline void paco_corecache_wait(void)
{
  int busy;
  while(1)
  {
    asm volatile ("csrr %0, %1":"=r"(busy):"i"(PACO_CSR_ADDR_CORECACHE_BUSY));
    if(busy==0)
      break;
  }
}

static inline void paco_memory_set_start_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_MEMORY_START), "r"(addr));
}

static inline void paco_memory_set_last_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_MEMORY_LAST), "r"(addr));
}

static inline void paco_dcache_set_start_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_DCACHE_START), "r"(addr));
}

static inline void paco_dcache_set_last_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_DCACHE_LAST), "r"(addr));
}

static inline void paco_lsu0cache_set_start_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_LSU0_START), "r"(addr));
}

static inline void paco_lsu0cache_set_last_addr(unsigned int addr)
{
  paco_lsu_wait();
  asm volatile ("csrw %0, %1"::"i"(PACO_CSR_ADDR_LSU0_LAST), "r"(addr));
}

#endif

