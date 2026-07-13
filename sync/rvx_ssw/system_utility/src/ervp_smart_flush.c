#include "ervp_smart_flush.h"

#include "ervp_misc_util.h"
#include "ervp_printf.h"
#include "ervp_caching.h"
#include "ervp_memory_allocator.h"
#include "ervp_variable_allocation.h"
#include <stdarg.h>

char trackedvar_track_enable[NUM_CORE] DATA_BSS;

#ifdef USE_SMART_FLUSH

static utset trackedvar_track[NUM_CORE] DATA_BSS;
static memory_allocator_t smart_allocator[NUM_CORE] DATA_BSS;

static unsigned short free_list_max_size[NUM_CORE] DATA_BSS;
static unsigned short free_list_current_size[NUM_CORE] DATA_BSS;
static memory_block_info_t **delayed_free_list[NUM_CORE] DATA_BSS;

static inline void increase_delayed_free_list()
{
  if (free_list_max_size[EXCLUSIVE_ID] == 0)
  {
    free_list_max_size[EXCLUSIVE_ID] = 16;
    delayed_free_list[EXCLUSIVE_ID] = malloc(free_list_max_size[EXCLUSIVE_ID] * sizeof(memory_block_info_t *));
  }
  else
  {
    void *old_ptr = delayed_free_list[EXCLUSIVE_ID];
    int old_size = free_list_max_size[EXCLUSIVE_ID] * sizeof(memory_block_info_t *);
    free_list_max_size[EXCLUSIVE_ID] = free_list_max_size[EXCLUSIVE_ID] << 1;
    delayed_free_list[EXCLUSIVE_ID] = malloc(free_list_max_size[EXCLUSIVE_ID] * sizeof(memory_block_info_t *));
    memcpy_rvx(delayed_free_list[EXCLUSIVE_ID], old_ptr, old_size);
    free_rvx(old_ptr);
  }
}

static inline void add_block_to_delayed_free_list(memory_block_info_t *block)
{
  if (free_list_current_size[EXCLUSIVE_ID] == free_list_max_size[EXCLUSIVE_ID])
    increase_delayed_free_list();
  delayed_free_list[EXCLUSIVE_ID][free_list_current_size[EXCLUSIVE_ID]++] = block;
}

static inline void release_delayed_free_list()
{
  for (int i = 0; i < free_list_current_size[EXCLUSIVE_ID]; i++)
    memory_allocator_push(&(smart_allocator[EXCLUSIVE_ID]), delayed_free_list[EXCLUSIVE_ID][i]);
  free_list_current_size[EXCLUSIVE_ID] = 0;
}

int trackedvar_add(void *ptr, int dirty)
{
  int inserted = 0;

  // needs region check
  if (trackedvar_track_enable[EXCLUSIVE_ID])
  {
    if (is_cacheable_region(ptr))
      inserted = utset_add(&(trackedvar_track[EXCLUSIVE_ID]), ptr);
  }
  return inserted;
}

// Without checking trackedvar_track_enable[EXCLUSIVE_ID]
static inline int _trackedvar_exist(void *ptr)
{
  return utset_exist(&(trackedvar_track[EXCLUSIVE_ID]), ptr);
}

int trackedvar_exist(void *ptr)
{
  int exist = 1;
  if (trackedvar_track_enable[EXCLUSIVE_ID])
    exist = _trackedvar_exist(ptr);
  return exist;
}

static inline void trackedvar_flush_cache()
{
  if (!is_sim())
    printf_function();
  flush_cache();
  utset_clear(&(trackedvar_track[EXCLUSIVE_ID]));
}

void trackedvar_track_start()
{
  flush_cache();
  utset_clear(&(trackedvar_track[EXCLUSIVE_ID]));
  if (free_list_current_size[EXCLUSIVE_ID] > 0)
    release_delayed_free_list();
  trackedvar_track_enable[EXCLUSIVE_ID] = 1;
}

int trackedvar_smart_flush(int region, ...)
{
  int is_cached = 1;
  if (trackedvar_track_enable[EXCLUSIVE_ID])
  {
    va_list args;
    va_start(args, region);

    for (int i = 0; i < region; i++)
    {
      void *addr = va_arg(args, void *);
      assert(addr);
      is_cached = _trackedvar_exist(addr);
      // printf("\n0x%0x: %d", addr, is_cached);
      if (is_cached)
        break;
    }
    va_end(args);
  }
  if (is_cached)
    trackedvar_flush_cache();
  return is_cached;
}

void trackedvar_print()
{
  utset_print(&(trackedvar_track[EXCLUSIVE_ID]));
}

void *trackedvar_malloc(size_t size)
{
  assert(size > 0);
  size_t aligned_size = ALIGN_UP_POW2(size, CACHE_LINE_SIZE);
  size_t extended_size = aligned_size + MEMORY_BLOCK_INFO_SIZE;
  memory_block_info_t *block = NULL;

#ifdef USE_REUSE_MEMORY_ALLOCATOR
  block = memory_allocator_pop(&(smart_allocator[EXCLUSIVE_ID]), extended_size);
#endif
  if (block == NULL)
  {
    _acquire_lock_for_malloc();
    heap_dram_addr += MEMORY_BLOCK_INFO_SIZE;
    heap_dram_addr = ALIGN_UP_POW2(heap_dram_addr, CACHE_LINE_SIZE);
    assert(heap_dram_size >= aligned_size);
    block = heap_dram_addr - MEMORY_BLOCK_INFO_SIZE;
    heap_dram_addr += aligned_size;
    _release_lock_for_malloc();

    block->size = extended_size;
  }

  assert(block);
  uintptr_t ptr = (uintptr_t)block + MEMORY_BLOCK_INFO_SIZE;
  assert(is_aligned_to_cacheline((unsigned int)ptr));
  if (trackedvar_track_enable[EXCLUSIVE_ID])
    assert(!_trackedvar_exist(ptr));
  return (void *)ptr;
}

void trackedvar_free(void *ptr)
{
  assert(ptr);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
  assert(is_aligned_to_cacheline(ptr));
  memory_block_info_t *block;
  block = (void *)((uintptr_t)ptr - MEMORY_BLOCK_INFO_SIZE);
  if (trackedvar_track_enable[EXCLUSIVE_ID] && trackedvar_exist(ptr))
    add_block_to_delayed_free_list(block);
  else
    memory_allocator_push(&(smart_allocator[EXCLUSIVE_ID]), block);
#endif
}

#endif