#include <stdarg.h>

#include "platform_info.h"
#include "ervp_platform_api.h"

#include "ervp_mmio_util.h"
#include "ervp_external_peri_group_memorymap.h"
#include "ervp_platform_controller_memorymap.h"
#include "ervp_auto_id.h"
#include "ervp_multicore_synch.h"
#include "ervp_platform_controller_api.h"
#include "ervp_core_id.h"
#include "ervp_printf.h"
#include "ervp_uart.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_interrupt.h"
#include "ervp_auto_id.h"
#include "ervp_profiling.h"
#include "ervp_tick.h"
#include "ervp_delay.h"
#include "ervp_variable_allocation.h"
#include "ervp_mmiox1.h"
#include "ervp_thread.h"

#ifdef INCLUDE_FUSE_BOX
#include "ervp_fuse_box.h"
#endif
#ifdef INCLUDE_TCACHING
#include "ervp_tcaching.h"
#endif
#ifdef INCLUDE_EDGE_VIDEO_SYSTEM
#include "edge_video_system.h"
#endif
#ifdef INCLUDE_EXT_MRAM
#include "ip_instance_info.h"
#endif

extern linker_var_t _bss_start_;
extern linker_var_t _bss_end_;
extern linker_var_t _backup_start_;
extern linker_var_t _cacheable_start_;
extern linker_var_t _cacheable_end_;

#ifdef USE_SMALL_RAM
extern linker_var_t _sram_start_;
extern linker_var_t _sram_end_;
extern linker_var_t _heap_sram_baseaddr_;
#endif
#ifdef USE_LARGE_RAM
extern linker_var_t _dram_start_;
extern linker_var_t _dram_end_;
extern linker_var_t _heap_dram_baseaddr_;
#endif

extern volatile unsigned int heap_sram_addr;
extern volatile unsigned int heap_sram_lastaddr;
extern volatile unsigned int heap_dram_addr;
extern volatile unsigned int heap_dram_lastaddr;

int num_restart NON_VOLATILE = 0;

void print_linker_var()
{
#ifdef USE_SMALL_RAM
	debug_printx(GET_LINKER_VAR(_sram_start_));
	debug_printx(GET_LINKER_VAR(_sram_end_));
#endif
#ifdef USE_LARGE_RAM
	debug_printx(GET_LINKER_VAR(_dram_start_));
	debug_printx(GET_LINKER_VAR(_dram_end_));
#endif
	debug_printx(GET_LINKER_VAR(_cacheable_start_));
	debug_printx(GET_LINKER_VAR(_cacheable_end_));
	debug_printx(GET_LINKER_VAR(_bss_start_));
	debug_printx(GET_LINKER_VAR(_bss_end_));
	debug_printx(GET_LINKER_VAR(_backup_start_));
}

void print_platform_info()
{
	const int name_string_size = 16;
	char name_string[name_string_size];
	const unsigned int name_addr = MMAP_DESIGN_INFO_PLATFORM_NAME00;

	const int date_string_size = 24;
	char date_string[date_string_size];
	const unsigned int date_addr = MMAP_DESIGN_INFO_DATE00;

	mmio_read_string(name_addr, name_string, name_string_size);
	mmio_read_string(date_addr, date_string, date_string_size);

	printf("\n[INFO] platform: %s @ %s", name_string, date_string);
}

void print_rvx_info()
{
	const int temp_string_size = 16;
	char temp_string[temp_string_size];
	const unsigned int user_name_addr = MMAP_DESIGN_INFO_USERNAME00;
	const unsigned int home_git_name_addr = MMAP_DESIGN_INFO_HOME_GIT_NAME00;
	const unsigned int home_git_version_addr = MMAP_DESIGN_INFO_HOME_GIT_VERSION00;
	const unsigned int devkit_git_version_addr = MMAP_DESIGN_INFO_DEVKIT_GIT_VERSION00;
	mmio_read_string(user_name_addr, temp_string, temp_string_size);
	printf("\n[INFO] user name: %s", temp_string);
	mmio_read_string(home_git_name_addr, temp_string, temp_string_size);
	printf("\n[INFO] rvx name: %s", temp_string);
	mmio_read_string(home_git_version_addr, temp_string, temp_string_size);
	printf("\n[INFO] home git: %s", temp_string);
	mmio_read_string(devkit_git_version_addr, temp_string, temp_string_size);
	printf("\n[INFO] devkit git: %s", temp_string);
}

static const int GPREG_INDEX_FOR_BOOT_INIT = 0;
static const int INITIALIZED = 1;

int wait_or_initialize()
{
	int wait;
	wait = (EXCLUSIVE_ID != 0);
	if (wait)
	{
		while (get_gpreg(GPREG_INDEX_FOR_BOOT_INIT) != INITIALIZED)
			;
	}
	return wait;
}

void _set_initialized()
{
	set_gpreg(GPREG_INDEX_FOR_BOOT_INIT, INITIALIZED);
}

void _clear_bss(int bss_start, int bss_end)
{
	int i;
	int imp_type;
	imp_type = get_imp_type();
	switch (imp_type)
	{
	case IMP_TYPE_VP:
	case IMP_TYPE_FPGA:
	case IMP_TYPE_CHIP:
		for (i = bss_start; i < bss_end; i += 4)
			REG32(i) = 0;
		break;
	case IMP_TYPE_RTL:
		break;
	}
}

// after _clear_bss
void _init_platform()
{
#ifdef BARRIER_INDEX_FOR_INIT
	const int barrier_index = BARRIER_INDEX_FOR_INIT;
#else
	const int barrier_index = 0;
#endif
	const int core_id = EXCLUSIVE_ID;
	int imp_type;
	if (core_id == 0)
	{
		uart_init();
		ready_to_print = 1;
		memory_dump_init();
		imp_type = get_imp_type();

		switch (imp_type)
		{
		case IMP_TYPE_VP:
			printf_must("\n\n[RVX/START] %s@VP", (is_sim() ? "SIM" : "EMU"));
			break;
		case IMP_TYPE_RTL:
			printf_must("\n\n[RVX/START] %s@RTL", (is_sim() ? "SIM" : "EMU"));
			break;
		case IMP_TYPE_FPGA:
			printf_must("\n\n[RVX/START] %s@FPGA", (is_sim() ? "SIM" : "EMU"));
			break;
		case IMP_TYPE_CHIP:
			printf_must("\n\n[RVX/START] %s@CHIP", (is_sim() ? "SIM" : "EMU"));
			break;
		}

		num_restart++;
		if (num_restart > 1)
			printf_must("\n[RVX/RESTART] %d", num_restart);
#ifdef INCLUDE_EXT_MRAM
		volatile int ext_mram_config;
		if (EXT_MRAM_CYCLE > 0)
		{
			ext_mram_config = 1 << EXT_MRAM_CYCLE;
			mmiox1_config_write(i_system_ext_mram_control_info, &ext_mram_config);
		}
		if (!is_sim())
		{
			mmiox1_config_read(i_system_ext_mram_control_info, &ext_mram_config);
			printf_must("\n[RVX/EXT_MRAM/CONFIG] %d", ext_mram_config);
		}
#endif
		printf_must("\n");

		init_system_tick();
		init_core_tick();
		init_switch();
#ifdef INCLUDE_EDGE_VIDEO_SYSTEM
		edge_video_system_init();
#endif
#ifdef PACT_INCLUDE_CORE
		pact_core0_init();
#endif
		if (!is_sim())
			delay_ms(100);
	}
	reach_and_pass_barrier(barrier_index, NUM_CORE_PHYSICAL);
	if (core_id >= NUM_CORE_USER)
		worker_core_entry();
	else if (core_id == 0)
	{
		if (!is_sim())
		{
			profiling_init();
			profiling_start("main");
		}
	}
}

void worker_core_entry()
{
	unsigned int auto_id = get_proc_auto_id();
	assert(auto_id != ((2 * NUM_CORE_PHYSICAL) - 1));
	_worker_thread_wait_loop();
}

static inline size_t _get_backup_sram_size()
{
	size_t backup_size = 0;
#ifdef USE_SMALL_RAM
	backup_size += (GET_LINKER_VAR(_sram_end_) - GET_LINKER_VAR(_sram_start_));
#endif
	return backup_size;
}

static inline size_t _get_backup_dram_size()
{
	size_t backup_size = 0;
#ifdef USE_LARGE_RAM
	backup_size += (GET_LINKER_VAR(_dram_end_) - GET_LINKER_VAR(_backup_start_));
#endif
	return backup_size;
}

static inline size_t _get_backup_total_size()
{
	return _get_backup_sram_size() + _get_backup_dram_size();
}

void _reboot_nvm()
{
	uint8_t *src, *dst;
	if (!is_sim())
	{
#ifdef USE_SMALL_RAM
		if (num_restart == 0)
		{
			src = GET_LINKER_VAR(_sram_start_);
			dst = GET_LINKER_VAR(_heap_sram_baseaddr_);
		}
		else
		{
			src = GET_LINKER_VAR(_heap_sram_baseaddr_);
			dst = GET_LINKER_VAR(_sram_start_);
		}
		for (int i = 0; i < _get_backup_sram_size(); i++)
			dst[i] = src[i];
#endif
#ifdef USE_LARGE_RAM
		if (num_restart == 0)
		{
			src = GET_LINKER_VAR(_backup_start_);
			dst = GET_LINKER_VAR(_heap_dram_baseaddr_) + _get_backup_sram_size();
		}
		else
		{
			src = GET_LINKER_VAR(_heap_dram_baseaddr_) + _get_backup_sram_size();
			dst = GET_LINKER_VAR(_backup_start_);
		}
		for (int i = 0; i < _get_backup_dram_size(); i++)
			dst[i] = src[i];
#endif
		flush_cache();
	}
}

void _init_heap()
{
	if (EXCLUSIVE_ID == 0)
	{
#ifdef USE_SMALL_RAM
		heap_sram_addr = GET_LINKER_VAR(_heap_sram_baseaddr_);
		heap_sram_lastaddr = ((unsigned int)SMALL_RAM_LASTADDR);
#endif
#ifdef USE_LARGE_RAM
		heap_dram_addr = GET_LINKER_VAR(_heap_dram_baseaddr_);
#ifdef INCLUDE_EXT_MRAM
		heap_dram_addr += round_up_int(_get_backup_total_size(), DATA_ALIGN_SIZE);
#endif
		heap_dram_lastaddr = ((unsigned int)LARGE_RAM_LASTADDR);
#endif
	}
}

void _init_each_core()
{
	init_core();
#ifdef INCLUDE_TCACHING
	tcaching_init();
#endif
	unsigned int cacheable_start = GET_LINKER_VAR(_cacheable_start_);
	unsigned int cacheable_last = GET_LINKER_VAR(_cacheable_end_) - 1;
#ifdef USE_TCACHING
#ifdef USE_LARGE_RAM
	assert((cacheable_start - 1) == TEMPORARY_CACHING_HEAP_LAST); // NOT continuous cacheable region!
	cacheable_start = TEMPORARY_CACHING_HEAP_START;
#endif
#endif
	init_cache(cacheable_start, cacheable_last);
	init_handle_trap();
}

void _stop_cpu()
{
	mmio_read_data(MMAP_SUBMODULE_POWER);
}

static inline void terminate_simulation()
{
	if (is_sim())
		printf_must("%c", 4);
}

void exit_platform()
{
	unsigned int auto_id = get_proc_auto_id();
	if (auto_id == ((2 * NUM_CORE_PHYSICAL) - 1))
	{
		if (!is_sim())
		{
			profiling_end("main");
			profiling_print();
			printf_must("\n\n[RVX/END]\n");
		}
#ifdef USE_FAKEFILE
		fakefile_prepare_dump();
#endif
		terminate_simulation();
		flush_cache();
	}
#ifdef INCLUDE_FUSE_BOX
	disable_core();
#endif
	_stop_cpu();
}

void exit_rvx(unsigned int status)
{
	if (status != 0)
		printf_must("\nError code: %8x\n", status);
#ifdef USE_FAKEFILE
	fakefile_prepare_dump();
#endif
	terminate_simulation();
	while (1)
		;
}

__attribute__((weak)) void _init()
{
}

__attribute__((weak)) void _fini()
{
}
