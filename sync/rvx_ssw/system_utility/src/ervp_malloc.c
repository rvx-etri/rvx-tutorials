#include "platform_info.h"

#include "utlist.h"
#include "uthash.h"

#include "ervp_printf.h"
#include "ervp_misc_util.h"
#include "ervp_malloc_system.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"
#include "ervp_memory_allocator.h"
#include "ervp_variable_allocation.h"
#include "ervp_core_id.h"

static memory_allocator_t default_allocator[NUM_CORE] DATA_BSS;

__attribute__((weak)) void *malloc_rvx(size_t size)
{
  void *ptr = NULL;
  if (size > 0)
  {
#ifdef USE_REUSE_MEMORY_ALLOCATOR
    memory_block_info_t *block;
    size_t extended_size = ALIGN_UP_POW2(size + MEMORY_BLOCK_INFO_SIZE, DATA_ALIGN_SIZE);
    block = memory_allocator_pop(&(default_allocator[EXCLUSIVE_ID]), extended_size);
    if (block == NULL)
    {
      block = _alloc_new_memory_space(extended_size);
      if (block == NULL)
        return NULL;
      block->size = extended_size;
    }
    ptr = (void *)(((unsigned int)block) + MEMORY_BLOCK_INFO_SIZE);
#else
    size_t extended_size = ALIGN_UP_POW2(size, DATA_ALIGN_SIZE);
    ptr = _alloc_new_memory_space(extended_size);
#endif
  }

#if 0
      printf("\n[malloc] %p %d", ptr, size);
#endif
  return ptr;
}

void *malloc_rvx_with_assert(size_t size, const char *file, unsigned int line, const char *func)
{
  void *ptr = malloc_rvx(size);
  if (!ptr)
  {
    printf_must("\n0x%08x", heap_sram_addr);
    printf_must("\n0x%08x", heap_sram_size);
    printf_must("\n0x%08x", heap_dram_addr);
    printf_must("\n0x%08x", heap_dram_size);
    assert_must_msg(0, "malloc fails");
  }
  return ptr;
}

void free_rvx(void *ptr)
{
  assert(ptr);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
  memory_block_info_t *block;
  block = (void *)(((unsigned int)ptr) - MEMORY_BLOCK_INFO_SIZE);
  memory_allocator_push(&(default_allocator[EXCLUSIVE_ID]), block);
#if 0
    printf("\n[free] %p %d", ptr, block->size);
#endif
#endif
}

void *realloc_rvx(void *ptr, size_t new_size)
{
  assert(default_allocator != NULL);
  memory_block_info_t *block;
  void *new_ptr;
  new_ptr = malloc_rvx(new_size);
  block = (void *)(((unsigned int)ptr) - MEMORY_BLOCK_INFO_SIZE);
  memcpy_rvx(new_ptr, ptr, block->size);
  free_rvx(ptr);
  return new_ptr;
}

void print_heap_status()
{
  printf("\nheap_sram_addr: 0x%x", heap_sram_addr);
  printf("\nheap_sram_size: 0x%x", heap_sram_size);
  printf("\nheap_dram_addr: 0x%x", heap_dram_addr);
  printf("\nheap_dram_size: 0x%x", heap_dram_size);
}

void *calloc_rvx(size_t elt_count, size_t elt_size)
{
  int num = elt_count * elt_size;
  void *result = malloc_rvx(num);
  result = memset_rvx(result, 0, num);
  return result;
}

int test_memory_leak()
{
  int diff = 0;
#ifdef USE_REUSE_MEMORY_ALLOCATOR
  static int init = 0;
  static unsigned int previous = 0;

  if (init < 2)
  {
    init++;
    previous = heap_dram_addr;
  }
  else
  {
    diff = heap_dram_addr - previous;
    if (diff == 0)
    {
      printf_must("\n[RVX/INFO\] No Memory Leak");
    }
    else
    {
      printf_must("\n[RVX/INFO\] Memory Leak");
      debug_printx(previous);
      debug_printx(heap_dram_addr);
      debug_printx(diff);
      previous = heap_dram_addr;
    }
  }
#else
  printf_must("\n\[RVX/WARNING\] Enables 'USE_REUSE_MEMORY_ALLOCATOR' for testing memory leak");
#endif
  return diff;
}