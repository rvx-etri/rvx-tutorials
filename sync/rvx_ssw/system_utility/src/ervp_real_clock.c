#include <stdint.h>
#include "platform_info.h"

#include "ervp_misc_util.h"
#include "ervp_mmio_util.h"
#include "ervp_common_peri_group_memorymap.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"

static const int min_period_us = 10;

static inline uint64_t __get_real_clock_tick()
{
	volatile uint64_t* addr = (uint64_t*)(MMAP_SUBMODULE_REAL_CLOCK);
	uint64_t value;
#ifdef LOCK_INDEX_FOR_SYSTEM_HW
	const int lock_index = LOCK_INDEX_FOR_SYSTEM_HW;
#else
	const int lock_index = -1;
#endif
	acquire_lock(lock_index);
	value = *addr;
	release_lock(lock_index);
	return value;
}

uint64_t get_real_clock_tick()
{
	uint64_t value = __get_real_clock_tick();
	return value;
}

void print_real_clock_tick()
{
	printf("\n%llu", get_real_clock_tick());
}

static inline void __delay_until(uint64_t start_clock_us, uint64_t target_clock_us)
{
	unsigned int upper;
	uint64_t current_tick;
	if(start_clock_us >= target_clock_us)
		while(1)
		{
			upper = (__get_real_clock_tick() >> 32);
			if(upper==0)
				break;
		}
	while((current_tick=__get_real_clock_tick())<target_clock_us);
}

void delay_us_by_real_clock(unsigned int time_in_us)
{
	uint64_t start_clock_us, target_clock_us;
	start_clock_us = __get_real_clock_tick();
	while(time_in_us < min_period_us);
	target_clock_us = start_clock_us + time_in_us;
	__delay_until(start_clock_us, target_clock_us);
}

static inline void __delay_unit(uint64_t time_in_unit, const unsigned int unit_scale)
{
	uint64_t start_clock_us, target_clock_us;
	start_clock_us = __get_real_clock_tick();
	target_clock_us = time_in_unit;
	target_clock_us = target_clock_us*unit_scale;
	target_clock_us += start_clock_us;
	__delay_until(start_clock_us, target_clock_us);
}

void delay_ms_by_real_clock(unsigned int time_in_ms)
{
	__delay_unit(time_in_ms, 1000);
}

void delay_sec_by_real_clock(int time_in_sec)
{
	__delay_unit(time_in_sec, 1000000);
}

void print_real_clock_dhms()
{
  uint64_t tick = __get_real_clock_tick();
  uint64_t ms, sec, min, hour, day;
  int ms_trunc, sec_trunc, min_trunc, hour_trunc, day_trunc;

  ms = tick / 1000;
  sec = ms / 1000;
  min = sec / 60;
  hour = min / 60;
  day = hour / 24;

  ms_trunc = ms % 1000;
  sec_trunc = sec % 60;
  min_trunc = min % 60;
  hour_trunc = hour % 24;
  day_trunc = day;
  printf("\nd%03d:h%02d:m%02d:s%02d:ms%03d", day_trunc, hour_trunc, min_trunc, sec_trunc, ms_trunc);
}
