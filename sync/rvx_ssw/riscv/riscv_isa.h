#ifndef __RISCV_ISA_H__
#define __RISCV_ISA_H__

#include <stdint.h>

//#define read_csr(csr_reg, result) ({asm volatile ("csrr %0, %1" : "=r"(result) : "i"(csr_reg));})
//#define write_csr(csr_reg, data) ({asm volatile ("csrw %0, %1" : : "i"(csr_reg), "r"(data));})

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
    asm volatile ("csrw " #reg ", %0" :: "i"(val)); \
  else \
    asm volatile ("csrw " #reg ", %0" :: "r"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
  if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
    asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "i"(val)); \
  else \
    asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
  if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
    asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
  else \
    asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
    asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
  else \
    asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })

#define get_internal_core_id() read_csr(mhartid)

#define IDLE asm volatile ("")

static inline uintptr_t get_stack_pointer()
{
	uintptr_t reg_sp;
	asm volatile("mv %0, sp" : "=r"(reg_sp));
	return reg_sp;
};

static inline uintptr_t  get_return_address()
{
	uintptr_t reg_ra;
	asm volatile("mv %0, ra" : "=r"(reg_ra));
	return reg_ra;
};

#endif

