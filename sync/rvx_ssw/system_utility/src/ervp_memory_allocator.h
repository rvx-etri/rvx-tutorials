#ifndef __ERVP_MEMORY_ALLOCATOR_H__
#define __ERVP_MEMORY_ALLOCATOR_H__

#include "utlist.h"
#include "uthash.h"

#include "platform_info.h"
#include "ervp_memory_util.h"

typedef struct memory_block_info
{
  size_t size;
  struct memory_block_info *next;
} memory_block_info_t;

typedef struct memory_block_list_info
{
  size_t size;
  struct memory_block_info *next;
  UT_hash_handle hh;
} memory_block_head_info_t;

typedef memory_block_head_info_t *memory_allocator_t;

#define MEMORY_BLOCK_INFO_SIZE sizeof(size_t)

// needs a lock for allocator
memory_block_info_t *memory_allocator_pop(memory_allocator_t *allocator, size_t size);
void memory_allocator_push(memory_allocator_t *allocator, memory_block_info_t *block);

#endif
