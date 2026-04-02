#ifndef __ERVP_SHAREDPOINTER_H__
#define __ERVP_SHAREDPOINTER_H__

typedef struct
{
    void *data;
    int count;
    void (*deleter)(void *); // WITHOUT refcount_t
} refcount_t;

refcount_t *refcount_alloc(void *data, void (*deleter)(void *));
void sharedpointer_free(void *a, refcount_t *refcount);

#endif