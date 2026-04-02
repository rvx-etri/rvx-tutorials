.global GET_AUTO_ID
.global SET_STACK_POINTER

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
#ifdef INCLUDE_EXT_MRAM
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
