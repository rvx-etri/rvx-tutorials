#include "platform_info.h"

#include "ervp_printf.h"
#include "ervp_round_int.h"
#include "ervp_malloc.h"
#include "ervp_multicore_synch.h"
#include "ervp_assert.h"
#include "utlist.h"
#include "uthash.h"
#include "ervp_variable_allocation.h"

volatile unsigned int heap_sram_addr NOTCACHED_DATA = 0;
volatile unsigned int heap_sram_lastaddr NOTCACHED_DATA = 0;
volatile unsigned int heap_dram_addr NOTCACHED_DATA = 0;
volatile unsigned int heap_dram_lastaddr NOTCACHED_DATA = 0;

#ifdef LOCK_INDEX_FOR_SYSTEM_VARIABLE
static const int LOCK_INDEX = LOCK_INDEX_FOR_SYSTEM_VARIABLE;
#else
static const int LOCK_INDEX = -1;
#endif

static inline void *__alloc_to_sram(size_t size)
{
  unsigned int result = 0;
#ifdef USE_SMALL_RAM
  if ((heap_sram_lastaddr - heap_sram_addr) > size)
  {
    result = heap_sram_addr;
    heap_sram_addr += size;
  }
#endif
  return (void *)result;
}

static inline void *__alloc_to_dram(size_t size)
{
  unsigned int result = 0;
#ifdef USE_LARGE_RAM
  if ((heap_dram_lastaddr - heap_dram_addr) > size)
  {
    result = heap_dram_addr;
    heap_dram_addr += size;
  }
#endif
  return (void *)result;
}

// reallocator
typedef struct memory_block_info
{
  int size;
  struct memory_block_info *prev; /* needed for a doubly-linked list only */
  struct memory_block_info *next; /* needed for singly- or doubly-linked lists */
  UT_hash_handle hh;
} memory_block_info_t;

const int ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO = (((sizeof(memory_block_info_t) - 1) / 4) + 1) * 4;

static memory_block_info_t *reallocator = NULL;

static inline memory_block_info_t *__alloc_by_collector(size_t size)
{
  memory_block_info_t *head, *block;
  block = NULL;
#ifdef USE_REUSE_MEMORY_ALLOCATOR
  HASH_FIND_INT(reallocator, &size, head);
  if (head != NULL)
  {
    assert(head->prev != NULL);
    if (head != head->prev) // if not first dummy element
    {
      block = head->prev;
      CDL_DELETE(head, block);
    }
  }
#endif
  return block;
}

const int SMALL_DATA_SIZE = DATA_ALIGN_SIZE;

static inline void *_alloc_memory_addr_only(size_t size)
{
  void *ptr = NULL;
  if (size <= SMALL_DATA_SIZE)
  {
    ptr = __alloc_to_sram(size);
    if (ptr == NULL)
      ptr = __alloc_to_dram(size);
  }
  else
  {
    ptr = __alloc_to_dram(size);
    if (ptr == NULL)
      ptr = __alloc_to_sram(size);
  }
  return ptr;
}

__attribute__((weak)) void *malloc_rvx(size_t size)
{
  void *ptr = NULL; // direct or info
  if (size > 0)
  {
    size_t extended_size;
    acquire_lock(LOCK_INDEX);
    extended_size = round_up_int(size, DATA_ALIGN_SIZE);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
    extended_size = round_up_int(size + ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO, DATA_ALIGN_SIZE);
#endif
    ptr = __alloc_by_collector(extended_size);
    if (ptr == NULL)
    {
#if 0
      printf("\n[malloc new] %d", extended_size);
#endif
      ptr = _alloc_memory_addr_only(extended_size);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
      volatile memory_block_info_t *block = (memory_block_info_t *)ptr;
      block->size = extended_size;
      block->prev = NULL;
      block->next = NULL;
#endif
    }
    release_lock(LOCK_INDEX);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
    if (ptr != NULL)
      ptr = (void *)(((unsigned int)ptr) + ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO);
#endif
#if 0
      printf("\n[malloc] %p %d", ptr, size);
#endif
  }
  return ptr;
}

void *malloc_rvx_with_assert(size_t size, const char *file, unsigned int line, const char *func)
{
  void *ptr = malloc_rvx(size);
  if (!ptr)
  {
    printf_must("\n0x%08x", heap_sram_addr);
    printf_must("\n0x%08x", heap_sram_lastaddr);
    printf_must("\n0x%08x", heap_dram_addr);
    printf_must("\n0x%08x", heap_dram_lastaddr);
    assert_must_msg(0, "malloc fails");
  }
  return ptr;
}

void free_rvx(void *ptr)
{
  assert(ptr);
#ifdef USE_REUSE_MEMORY_ALLOCATOR
  memory_block_info_t *head, *block;
  block = (void *)(((unsigned int)ptr) - ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO);
  acquire_lock(LOCK_INDEX);
  HASH_FIND_INT(reallocator, &(block->size), head);
  if (head == NULL)
  {
    // make first element dummy
    head = _alloc_memory_addr_only(ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO);
    head->size = block->size;
    head->prev = head;
    head->next = head;
    HASH_ADD_INT(reallocator, size, head);
  }
  CDL_APPEND(head, block);
  release_lock(LOCK_INDEX);
#if 0
    printf("\n[free] %p %d", ptr, block->size);
#endif
#endif
}

void *realloc_rvx(void *ptr, size_t new_size)
{
  assert(reallocator != NULL);
  memory_block_info_t *block;
  void *new_ptr;
  new_ptr = malloc_rvx(new_size);
  block = (void *)(((unsigned int)ptr) - ALIGNED_SIZE_OF_MEMORY_BLOCK_INFO);
  memcpy_rvx(new_ptr, ptr, block->size);
  free_rvx(ptr);
  return new_ptr;
}

void print_heap_status()
{
  printf("\nheap_sram_addr: 0x%x", heap_sram_addr);
  printf("\nheap_sram_lastaddr: 0x%x", heap_sram_lastaddr);
  printf("\nheap_dram_addr: 0x%x", heap_dram_addr);
  printf("\nheap_dram_lastaddr: 0x%x", heap_dram_lastaddr);
}

void *malloc_permanent(size_t size, size_t align_size)
{
  const int LARGE_SIZE = 0x10000;
  void *ptr;
  int need_dram_backpart;
  if (size > 0)
  {
#ifdef USE_LARGE_RAM
    if (size >= LARGE_SIZE)
      need_dram_backpart = 1;
    else
      need_dram_backpart = 0;
#else
    need_dram_backpart = 0;
#endif
    if (need_dram_backpart)
    {
      ptr = heap_dram_lastaddr - (size - 1);
      ptr = round_down_int(ptr, align_size);
      heap_dram_lastaddr = ptr - 1;
    }
    else
    {
      if (align_size > DATA_ALIGN_SIZE)
        heap_sram_addr = round_up_int(heap_sram_addr, align_size);
      ptr = __alloc_to_sram(size);
      if (align_size < DATA_ALIGN_SIZE)
        heap_sram_addr = round_up_int(heap_sram_addr, DATA_ALIGN_SIZE);
    }
  }
  else
    ptr = 0;

  return ptr;
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