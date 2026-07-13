#include "ervp_malloc_system.h"
#include "ervp_variable_allocation.h"

volatile uintptr_t heap_sram_addr NOTCACHED_DATA = 0;
volatile uintptr_t heap_sram_size NOTCACHED_DATA = 0;
volatile uintptr_t heap_dram_addr NOTCACHED_DATA = 0;
volatile uintptr_t heap_dram_size NOTCACHED_DATA = 0;
volatile uintptr_t heap_sram_last_id NOTCACHED_DATA = -1;
volatile uintptr_t heap_dram_last_id NOTCACHED_DATA = -1;

void *palloc_largeram_backpart(size_t size, size_t align_size)
{
#ifndef USE_LARGE_RAM
  assert(0);
#endif
  assert(size > 0);
  uintptr_t ptr;

  _acquire_lock_for_malloc();
  ptr = heap_dram_addr - size + heap_dram_size;
  ptr = ALIGN_DOWN_POW2(ptr, align_size);
#if defined(CACHE_LINE_SIZE)
  ptr = ALIGN_DOWN_POW2(ptr, CACHE_LINE_SIZE);
#endif
  heap_dram_size = ptr - heap_dram_addr;
  assert(heap_dram_size >= 0);
  _release_lock_for_malloc();

  return (void *)ptr;
}

void *palloc_cacheline(size_t size)
{
  assert(is_aligned_to_cacheline((unsigned int)size));
  assert(size > 0);
  uintptr_t ptr;

  _acquire_lock_for_malloc();
#if defined(USE_LARGE_RAM)
#if defined(CACHE_LINE_SIZE)
  heap_dram_addr = ALIGN_UP_POW2(heap_dram_addr, CACHE_LINE_SIZE);
#endif
  ptr = heap_dram_addr;
  heap_dram_addr += size;
#elif defined(USE_SMALL_RAM)
#if defined(CACHE_LINE_SIZE)
  heap_sram_addr = ALIGN_UP_POW2(heap_sram_addr, CACHE_LINE_SIZE);
#endif
  ptr = heap_sram_addr;
  heap_sram_addr += size;
#endif
  _release_lock_for_malloc();

  return (void *)ptr;
}

void *_alloc_new_memory_space(size_t size)
{
  void *ptr = NULL;
  _acquire_lock_for_malloc();
#if defined(USE_LARGE_RAM)
  ptr = _alloc_largeram(size);
#elif defined(USE_SMALL_RAM)
  ptr = _alloc_smallram(size);
#endif
  _release_lock_for_malloc();
  return ptr;
}