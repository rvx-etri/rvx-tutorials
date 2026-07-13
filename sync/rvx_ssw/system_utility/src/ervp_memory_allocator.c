#include "platform_info.h"

#include "ervp_printf.h"
#include "ervp_memory_allocator.h"
#include "ervp_multicore_synch.h"
#include "ervp_assert.h"
#include "ervp_variable_allocation.h"

memory_block_info_t *memory_allocator_pop(memory_allocator_t *allocator, size_t size)
{
  memory_block_head_info_t *head;
  memory_block_info_t *block;
  assert(allocator != NULL);
  HASH_FIND(hh, *allocator, &size, sizeof(size_t), head);
  block = NULL;
  if (head != NULL)
  {
    block = head->next;
    if (block != NULL) // if not first dummy element
      LL_DELETE(head, block);
  }
  return block;
}

static ervp_private_cacheline_t private_cacheline[NUM_CORE] DATA_BSS;

#if defined(CACHE_LINE_SIZE)
static const int ALLOC_SIZE = ALIGN_UP_POW2(sizeof(memory_block_head_info_t), CACHE_LINE_SIZE);
#else
static const int ALLOC_SIZE = sizeof(memory_block_head_info_t);
#endif

static inline void *palloc_head_info()
{
  assert(sizeof(memory_block_head_info_t) <= ALLOC_SIZE);

  ervp_private_cacheline_t *cacheline = &(private_cacheline[EXCLUSIVE_ID]);
  if (cacheline->current_size < ALLOC_SIZE)
  {
    cacheline->ptr = palloc_cacheline(ALLOC_SIZE);
    cacheline->current_size = ALLOC_SIZE;
  }
  uintptr_t ptr;
  ptr = cacheline->ptr;
  cacheline->ptr += ALLOC_SIZE;
  cacheline->current_size -= ALLOC_SIZE;
  return (void *)ptr;
}

void memory_allocator_push(memory_allocator_t *allocator, memory_block_info_t *block)
{
  memory_block_head_info_t *head;
  assert(allocator != NULL);
  assert(block);
  HASH_FIND(hh, *allocator, &(block->size), sizeof(size_t), head);
  if (head == NULL)
  {
    // make first element dummy
    // head = _alloc_new_memory_space(sizeof(memory_block_head_info_t));
    head = palloc_head_info();
    head->size = block->size;
    head->next = NULL;
    HASH_ADD(hh, *allocator, size, sizeof(size_t), head);
  }
  LL_APPEND_ELEM(head, head, block);
}
