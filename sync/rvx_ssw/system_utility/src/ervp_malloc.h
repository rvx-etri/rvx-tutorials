#ifndef __ERVP_MALLOC_H__
#define __ERVP_MALLOC_H__

#include <stddef.h>
#include "ervp_malloc_system.h"

#ifdef ASSERT_WHEN_MALLOC_FAILS
#define malloc(x) malloc_rvx_with_assert(x, __FILE__, __LINE__, __func__)
#else
#define malloc malloc_rvx
#endif

void *malloc_rvx(size_t size);
void *malloc_rvx_with_assert(size_t size, const char *file, unsigned int line, const char *func);

#define calloc calloc_rvx
void *calloc_rvx(size_t elt_count, size_t elt_size);

#define free free_rvx
void free_rvx(void *ptr);

#define realloc realloc_rvx
void *realloc_rvx(void *ptr, size_t new_size);

void print_heap_status();
int test_memory_leak();

typedef union
{
  unsigned int value;
  struct
  {
    unsigned int is_permanent : 1;
    unsigned int no_access_from_cpu : 1;
  } br;
} ervp_malloc_option_t;

static inline void *malloc_ext(size_t size, size_t align_size, unsigned int option_value)
{
  ervp_malloc_option_t option;
  option.value = option_value;
  int go_to_backpart = 0;
#ifdef USE_LARGE_RAM
  go_to_backpart = option.br.is_permanent;
#endif
  void *ptr;
  if (go_to_backpart)
    ptr = palloc_largeram_backpart(size, align_size);
  else
    ptr = malloc_rvx(size);
  return ptr;
};

#endif
