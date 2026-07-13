#ifndef __UTSET_H__
#define __UTSET_H__

#include "uthash.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_printf.h"

typedef uintptr_t word_t;

typedef struct utset_node
{
  word_t value;
  UT_hash_handle hh;
} utset_node_t;

typedef utset_node_t *utset;

static inline int utset_add(utset *set, word_t value)
{
  utset_node_t *n;
  int inserted = 0;
  HASH_FIND_INT(*set, &value, n);
  if (n == NULL)
  {
    n = (utset_node_t *)malloc(sizeof *n);
    assert(n != NULL);
    n->value = value;
    HASH_ADD_INT(*set, value, n);
    inserted = 1;
  }
  return inserted;
}

static inline int utset_exist(utset *set, word_t value)
{
  utset_node_t *n;
  HASH_FIND_INT(*set, &value, n);
  return n != NULL;
}

static inline int utset_delete(utset *set, word_t value)
{
  utset_node_t *n;
  HASH_FIND_INT(*set, &value, n);
  if (n == NULL)
    return 0;
  HASH_DEL(*set, n);
  free(n);
  return 1;
}

static inline unsigned int utset_count(utset *set)
{
  return (unsigned int)HASH_COUNT(*set);
}

static inline void utset_clear(utset *set)
{
  utset_node_t *n, *tmp;
  HASH_ITER(hh, *set, n, tmp)
  {
    HASH_DEL(*set, n);
    free(n);
  }
}

void utset_print(utset *set);

#endif
