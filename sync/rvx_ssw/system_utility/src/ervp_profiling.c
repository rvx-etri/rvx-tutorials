#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "ervp_assert.h"
#include "ervp_misc_util.h"
#include "ervp_printf.h"
#include "ervp_real_clock.h"
#include "ervp_profiling.h"

#ifdef USE_PROFILING

static const int RECORDING_IDLE = 0;
static const int RECORDING_BUSY = 1;

profile_t *_section_table = NULL;
static int profile_id = 0;

static float min_duration_ms = 0;

static void profiling_warning()
{
#ifndef SILENT_FOR_PROFILING
	printf_must("\n[RVX/PROFILING/WARNING] includes printf time\n");
#endif
}

void profiling_init()
{
	for (int i = 0; i < 3; i++)
	{
		profiling_start("__profiling_delay");
		profiling_end("__profiling_delay");
	}
	profile_t *section = _section_table;
	float average_ms, total_ms;
	total_ms = ((float)section->num_tick_rshifted) / ((TICK_HZ / 1000) >> section->shift_amount);
	average_ms = total_ms / section->num_called;
	min_duration_ms = average_ms * 5;
	// HASH_DEL(_section_table, section);
}

static void _profiling_section_print(const profile_t* section)
{
	float average_ms, total_ms;
	printf_must("\n\n[section] %s", section->name);
	printf_must("\ntotal tick %llu<<%u", section->num_tick_rshifted, section->shift_amount);
	printf_must("\nnum_called %u", section->num_called);
	if (section->num_called == 0)
	{
		printf_must("\ntotal time (ms) N/A");
		printf_must("\naverage time (ms) N/A");
	}
	else
	{
		total_ms = ((float)section->num_tick_rshifted) / ((TICK_HZ / 1000) >> section->shift_amount);
		average_ms = total_ms / section->num_called;
		printf_must("\ntotal time (ms) %.2f", total_ms);
		printf_must("\naverage time (ms) %.4f", average_ms);
	}
	if (average_ms <= min_duration_ms)
	{
		printf_must("\n[RVX/PROFILING/WARNING] NOT accurate due to short running time");
	}
}

static inline void __section_init(profile_t *section)
{
	section->id = profile_id++;
	section->num_called = 0;
	section->state = RECORDING_IDLE;
	section->num_tick_rshifted = 0;
	section->shift_amount = 0;
}

static profile_t *__profiling_register(const char *name)
{
	profile_t *section;
	section = (profile_t *)malloc(sizeof(profile_t));
	assert(section);
	__section_init(section);
	section->name = malloc(sizeof(char)*(strlen(name)+1));
	strcpy(section->name, name);
	HASH_ADD_STR(_section_table, name, section);
	return section;
}

profile_t *profiling_register(const char *name)
{
	profile_t *section;
	HASH_FIND_STR(_section_table, name, section);
	assert(section == NULL);
	return __profiling_register(name);
}

void profiling_start_by_name(const char *name)
{
	profile_t *section;
	HASH_FIND_STR(_section_table, name, section);
	if (section == NULL)
		section = __profiling_register(name);
	profiling_start_by_section(section);
}

void profiling_end_by_name(const char *name)
{
	uint64_t current_tick;
	uint64_t diff;
	profile_t *section;
	HASH_FIND_STR(_section_table, name, section);
	profiling_end_by_section(section);
}

void profiling_start_by_section(profile_t *section)
{
	assert(section);
	assert_msg(section->state != RECORDING_BUSY, "profiling pair is NOT matched %s", section->name);
	section->num_called++;
	section->state = RECORDING_BUSY;
	section->previous_tick = get_real_clock_tick();
}

void profiling_end_by_section(profile_t *section)
{
	uint64_t current_tick;
	uint64_t diff;
	current_tick = get_real_clock_tick();
	assert(section);
	assert_msg(section->state == RECORDING_BUSY, "profiling pair is NOT matched %s", section->name);
	section->state = RECORDING_IDLE;
	diff = (current_tick - section->previous_tick) >> section->shift_amount;
	section->num_tick_rshifted += diff;
	if ((section->num_tick_rshifted >> 62) > 0)
	{
		section->num_tick_rshifted >>= 1;
		section->shift_amount++;
	}
}

void profiling_print()
{
	printf_must("\n");
	profiling_warning();
	for (profile_t *section = _section_table; section != NULL; section = (profile_t *)(section->hh.next))
	{
		char section_name_prefix[3];
		section_name_prefix[0] = section->name[0];
		section_name_prefix[1] = section->name[1];
		section_name_prefix[2] = 0;
		if (strcmp(section_name_prefix, "__") != 0)
			_profiling_section_print(section);
	}
}

#endif