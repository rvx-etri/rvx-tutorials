#ifndef __CORE_DEPENDENT_H__
#define __CORE_DEPENDENT_H__

#include "platform_info.h"

#include "riscv_isa.h"
#include "csr_encoding.h"
#include "ervp_reg_util.h"
#include "ervp_interrupt.h"

#include "orca_cache.h"

#define BIG_ENDIAN_ACCESS 0

extern volatile int cache_temp;

static inline int get_local_id()
{
	int result;
	if(NUM_CORE_PER_CORECLUSTER==1)
		result = 0;
	else
		while(1);
	return result;
}

static inline unsigned int get_interrupt(){
	return ((read_csr(mstatus) & MSTATUS_MIE)>0);
}

static inline void enable_interrupt(){
	// Enable interrupts in general.
	set_csr(mstatus, MSTATUS_MIE);
}

static inline void disable_interrupt(){
	// Enable interrupts in general.
	clear_csr(mstatus, MSTATUS_MIE);
}

static inline void allow_interrupt_timer(){
	// Enable the Machine-Timer bit in MIE
	set_csr(mie, MIP_MTIP);
}

static inline void disallow_interrupt_timer(){
	// Disable the Machine-Timer bit in MIE
	clear_csr(mie, MIP_MTIP);
}

static inline void allow_interrupt_plic(){
	check_plic_grant();
	// Enable the Machine-External bit in MIE
	set_csr(mie, MIP_MEIP);
}

static inline void disallow_interrupt_plic(){
	// Disable the Machine-External bit in MIE
	clear_csr(mie, MIP_MEIP);
}

static inline void wait_flush(){
  cache_temp = 0;
}

static inline void flush_cache()
{
	orca_flush_dcache_range((void *)0x0, (void *)0xFFFFFFFF);
  wait_flush();
}

/*
static inline void disable_interrupt(){
	int value;
	value = 0;
	write_csr(MEIMASK, value);
	value = 0;
	write_csr(MSTATUS, value);
}
static inline void enable_interrupt(){
	int value;
	value = 0xFFFFFFFF;
	write_csr(MEIMASK, value);
	value = 8;
	write_csr(MSTATUS, value);
}
*/

//#define CSR_MUMR0_BASE 0xBE0
//#define CSR_MUMR1_BASE 0xBE1
//#define CSR_MUMR2_BASE 0xBE2
//#define CSR_MUMR3_BASE 0xBE3
//#define CSR_MUMR0_LAST 0xBE8
//#define CSR_MUMR1_LAST 0xBE9
//#define CSR_MUMR2_LAST 0xBEA
//#define CSR_MUMR3_LAST 0xBEB

static inline void init_core()
{
	write_csr(mie, 0);
};

void init_cache(unsigned int cacheable_start, unsigned int cacheable_last);
void set_cacheable_region(int index, unsigned int cacheable_start, unsigned int cacheable_last);

#endif
