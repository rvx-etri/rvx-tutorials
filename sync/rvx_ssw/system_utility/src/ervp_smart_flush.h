#ifndef __ERVP_SMART_FLUSH_H__
#define __ERVP_SMART_FLUSH_H__

#include "platform_info.h"
#include "utset.h"
#include "core_dependent.h"
#include "ervp_malloc.h"
#include "ervp_core_id.h"
#include "ervp_platform_controller_api.h"

extern char trackedvar_track_enable[NUM_CORE];

#ifdef USE_SMART_FLUSH

void trackedvar_track_start();

static inline void trackedvar_track_end()
{
  _acquire_lock_for_malloc();
  trackedvar_track_enable[EXCLUSIVE_ID] = 0;
  _release_lock_for_malloc();
}

int trackedvar_add(void *ptr, int dirty);
int trackedvar_exist(void *ptr);
int trackedvar_smart_flush(int region, ...);
void trackedvar_print();
void *trackedvar_malloc(size_t size);
void trackedvar_free(void *ptr);

#define cache_flush_smart trackedvar_smart_flush

#else

static inline void trackedvar_track_start() {}
static inline void trackedvar_track_end() {}

static inline int trackedvar_add(void *ptr, int dirty) { return 0; }
static inline int trackedvar_exist(void *ptr) { return 0; }
static inline int trackedvar_smart_flush(int region, ...)
{
  if (!is_sim())
    printf_function();
  flush_cache();
  return 1;
}

static inline void trackedvar_print() {}
#define trackedvar_malloc malloc_rvx
#define trackedvar_free free_rvx

#endif

#define cache_flush_smart trackedvar_smart_flush

#endif
