// See LICENSE for license details.

// See LICENSE for license details.

#include "platform_info.h" // should be the first

#include "ervp_platform_controller_memorymap.h"
#include "ervp_common_peri_group_memorymap.h"
#include "ervp_external_peri_group_memorymap.h"
#include "ervp_asm_util.h"

.section .init
.global _start
.type _start,@function

_start:
.option push
.option norelax
	la gp, _gp
.option pop

	GET_AUTO_ID
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
1:
	j 1b
