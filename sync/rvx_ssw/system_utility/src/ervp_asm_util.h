.global INIT_REG
.global GET_AUTO_ID
.global SET_STACK_POINTER

.macro INIT_REG
	li x1, 0
	li x2, 0
	li x3, 0
	li x4, 0
	li x5, 0
	li x6, 0
	li x7, 0
	li x8, 0
	li x9, 0
	li x10, 0
	li x11, 0
	li x12, 0
	li x13, 0
	li x14, 0
	li x15, 0
	li x16, 0
	li x17, 0
	li x18, 0
	li x19, 0
	li x20, 0
	li x21, 0
	li x22, 0
	li x23, 0
	li x24, 0
	li x25, 0
	li x26, 0
	li x27, 0
	li x28, 0
	li x29, 0
	li x30, 0
	li x31, 0
.endm

.macro GET_AUTO_ID
	li t0, (MMAP_PLATFORM_REGISTER_PROC_AUTO_ID)
	lw a0, 0(t0)
.endm

.macro SET_STACK_POINTER
	li t0, STACK_SIZE
	mul t0, t0, a0
	la sp, _sp
	sub sp, sp, t0
.endm

.macro INIT_ONCE
#ifdef INCLUDE_OFFCHIP_MRAM
#ifndef USE_LARGE_RAM_MANUALLY
	jal _reboot_nvm;
#endif
#endif
	la a0, _bss_start_
	la a1, _bss_end_
	jal _clear_bss
	la a0, _bigdata_bss_start_
	la a1, _bigdata_bss_end_
	jal _clear_bss
  INIT_HEAP
	la a0, __libc_fini_array
	call atexit
	call __libc_init_array
	jal _set_initialized
.endm

.macro INIT_HEAP
	jal _init_heap
.endm

.macro INIT_EACH_CORE
	jal _init_each_core
.endm

.macro INIT_PLATFORM
	jal _init_platform
.endm

.macro STOP
stop:
	j stop
.endm
