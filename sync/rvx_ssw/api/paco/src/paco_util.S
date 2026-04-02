#include "platform_info.h" // should be the first
#include "riscv_memorymap_offset.h"
#include "ervp_cache_memorymap_offset.h"

.section      .text.entry
.align 4

.global paco_stop
.type paco_stop,@function

paco_stop:
  csrwi PACO_CSR_ADDR_STOP_REQUEST,1
1:
	j 1b

.global paco_init
.type paco_init,@function

paco_init:
	li x0, 0
  li x1, 1
  li x2, 2
  li x3, 3
  li x4, 4
  li x5, 5
  li x6, 6
  li x7, 7
  li x8, 8
  li x9, 9

  li x10, 10
  li x11, 11
  li x12, 12
  li x13, 13
  li x14, 14
  li x15, 15
  li x16, 16
  li x17, 17
  li x18, 18
  li x19, 19

  li x20, 20
  li x21, 21
  li x22, 22
  li x23, 23
  li x24, 24
  li x25, 25
  li x26, 26
  li x27, 27
  li x28, 28
  li x29, 29

  li x30, 30
  li x31, 31

  li t0, STACK_SIZE*NUM_CORE_USER
	la sp, _sp
	sub sp, sp, t0

#ifdef USE_LARGE_RAM
  li t0, LARGE_RAM_BASEADDR
#else
  li t0, SMALL_RAM_BASEADDR
#endif
  csrw PACO_CSR_ADDR_MEMORY_START, t0
#ifdef USE_LARGE_RAM
  li t0, LARGE_RAM_LASTADDR
#else
  li t1, SMALL_RAM_LASTADDR
#endif
  csrw PACO_CSR_ADDR_MEMORY_LAST, t1

  la ra, paco_stop
  ret
