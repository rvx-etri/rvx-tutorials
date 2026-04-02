#ifndef __ERVP_PLATFORM_API_H__
#define __ERVP_PLATFORM_API_H__

#include "ervp_error_code.h"
#include "ervp_printf.h"

typedef unsigned int* linker_var_t;
#define GET_LINKER_VAR(x) ((unsigned int)(&x))

extern int num_restart;

void print_platform_info();
void print_rvx_info();
int wait_or_initialize();
void _set_initialized();
void _clear_bss(int bss_start, int bss_end);
void _init_platform();
void _init_heap();
void _init_each_core();
void _reboot_nvm();
void _stop_cpu();

void worker_core_entry();
void exit_platform();

#define exit exit_rvx
void exit_rvx(unsigned int status);

void print_linker_var();

#endif
