#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_sharedpointer.h"

refcount_t *refcount_alloc(void *data, void (*deleter)(void *))
{
  assert(data);
  assert(deleter);
  refcount_t *result = malloc(sizeof(refcount_t));
  result->data = data;
  result->count = 0;
  result->deleter = deleter;
  return result;
}

void sharedpointer_free(void *a, refcount_t *refcount)
{
  assert(a);
  assert(refcount);
  assert(refcount->count > 0);

  refcount->count--;

  if (refcount->count == 0)
  {
    if (a != refcount->data)
      refcount->deleter(a);
    refcount->deleter(refcount->data);
    free(refcount);
  }
  else if (a == refcount->data)
    ;
  else
    refcount->deleter(a);
}