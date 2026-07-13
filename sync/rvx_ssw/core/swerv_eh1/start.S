// See LICENSE for license details.

// See LICENSE for license details.

#include "platform_info.h" // should be the first

#include "ervp_platform_controller_memorymap.h"
#include "ervp_common_peri_group_memorymap.h"
#include "ervp_external_peri_group_memorymap.h"
#include "ervp_asm_util.h"
#include "csr_encoding.h"

.section .init
.global _start
.type _start,@function



_start:
        #clear minstret
        csrw minstret, zero
        csrw minstreth, zero

        #clear registers
        li  x1, 0
        li  x2, 0
        li  x3, 0
        li  x4, 0
        li  x5, 0
        li  x6, 0
        li  x7, 0
        li  x8, 0
        li  x9, 0
        li  x10,0
        li  x11,0
        li  x12,0
        li  x13,0
        li  x14,0
        li  x15,0
        li  x16,0
        li  x17,0
        li  x18,0
        li  x19,0
        li  x20,0
        li  x21,0
        li  x22,0
        li  x23,0
        li  x24,0
        li  x25,0
        li  x26,0
        li  x27,0
        li  x28,0
        li  x29,0
        li  x30,0
        li  x31,0


        #cache configuration
        li t1, 0x55555555
        csrw 0x7c0, t1

        #setup MEIP and MTIP
        #li t0, (1<<7 | 1<<11)
        #csrw mie, t0
        #li t0, (1<<3)
        #csrw mstatus, t0

        # initialize global pointer
        .option push
        .option norelax
        la gp, _gp
        .option pop
        #la sp, _sp


#       #hart id
        csrr a0, mhartid

	#GET_AUTO_ID
	SET_STACK_POINTER
	
	jal wait_or_initialize
	bnez a0, _init
	INIT_ONCE

_init:
	INIT_EACH_CORE
	INIT_PLATFORM

	li a0, 0 # argc = 0
	li a1, 0 # argv = 0
	jal main
	jal exit_platform

1:  j 1b

