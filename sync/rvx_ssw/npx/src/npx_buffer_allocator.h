#ifndef __NPX_BUFFER_ALLOCATOR_H__
#define __NPX_BUFFER_ALLOCATOR_H__

#include <stddef.h>
#include "ervp_matrix_op_sw.h"

void npx_buffer_create(uintptr_t baseaddr, size_t size);
void npx_buffer_destroy();
void *npx_buffer_allocator_pop(size_t size);
int npx_buffer_allocator_push(void* ptr);

#endif