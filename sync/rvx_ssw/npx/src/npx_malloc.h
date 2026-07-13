#ifndef __NPX_MALLOC_H__
#define __NPX_MALLOC_H__

#include <stddef.h>

#include "ervp_assert.h"
#include "ervp_smart_flush.h"
#include "npx_buffer_allocator.h"

#define npx_malloc trackedvar_malloc

void *npx_malloc_buffer(size_t size);
void npx_free(void *ptr);

#endif
