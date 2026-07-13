#include "utset.h"

void utset_print(utset *set)
{
  utset_node_t *n, *tmp;
  printf("\n[utset] %u", utset_count(set));
  HASH_ITER(hh, *set, n, tmp)
  {
    printf("\n%08x", n->value);
  }
}