#include "npx_malloc.h"

void *npx_malloc_buffer(size_t size)
{
  void *result;
  result = npx_buffer_allocator_pop(size);
  if (!result)
    result = npx_malloc(size);
  assert(result);
  return result;
}

void npx_free(void *ptr)
{
  if (!npx_buffer_allocator_push(ptr))
    trackedvar_free(ptr);
}