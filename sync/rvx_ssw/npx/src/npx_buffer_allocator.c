#include "platform_info.h"
#include "ervp_malloc.h"
#include "ervp_variable_allocation.h"
#include "npx_buffer_allocator.h"

#include "utlist.h"

typedef struct buffer_block buffer_block_t;

struct buffer_block
{
	uintptr_t ptr;
	size_t size;
	buffer_block_t *prev;
	buffer_block_t *next;
	int available;
};

static uintptr_t BUFFER_START_ADDR = 0;
static size_t BUFFER_MAX_SIZE = 0;
static buffer_block_t *npx_buffer_allocator NOTCACHED_DATA = NULL;

void npx_buffer_create(uintptr_t baseaddr, size_t size)
{
	BUFFER_START_ADDR = baseaddr;
	BUFFER_MAX_SIZE = size;

	buffer_block_t *block = malloc(sizeof(buffer_block_t));
	block->ptr = baseaddr;
	block->size = size;
	block->available = 1;
	// DL_APPEND_ELEM(npx_buffer_allocator, npx_buffer_allocator, block);
	DL_APPEND(npx_buffer_allocator, block);
}

static void npx_buffer_print_block(buffer_block_t *temp, int index)
{
	printf("\n[%d] %08x, %8x, %1d", index, temp->ptr, temp->size, temp->available);
}

static void npx_buffer_print_all()
{
	buffer_block_t *temp;
	int i = 0;
	DL_FOREACH(npx_buffer_allocator, temp)
	{
		npx_buffer_print_block(temp, i++);
	}
}

void npx_buffer_destroy()
{
	assert(npx_buffer_allocator);
	buffer_block_t *temp;
	int count;
	DL_COUNT(npx_buffer_allocator, temp, count);
	if (count != 1)
	{
		npx_buffer_print_all();
		assert_msg(0, "some npx buffers are NOT freed");
	}
	DL_DELETE(npx_buffer_allocator, npx_buffer_allocator);
	assert(npx_buffer_allocator == 0);
}

static int lack_amount NOTCACHED_DATA = 0;

void npx_buffer_report()
{
#ifndef NDEBUG
	printf_function();
	printf("lack_amount: %d", lack_amount);
#endif
}

void *npx_buffer_allocator_pop(size_t size)
{
	assert(size);
	void *result;
	if (npx_buffer_allocator)
	{
		size = ALIGN_UP_POW2(size, CACHE_LINE_SIZE);
		buffer_block_t *best_block = NULL;
		buffer_block_t *temp;
		DL_FOREACH(npx_buffer_allocator, temp)
		{
			if (temp->available)
			{
				if (temp->size >= size)
				{
					if (best_block == NULL)
						best_block = temp;
					else if (best_block->size > temp->size)
						best_block = temp;
				}
			}
		}

		result = NULL;
		if (best_block)
		{
			buffer_block_t *remain;
			remain = malloc(sizeof(buffer_block_t));
			remain->ptr = best_block->ptr + size;
			remain->size = best_block->size - size;
			remain->available = 1;

			best_block->size = size;
			best_block->available = 0;

			DL_APPEND_ELEM(npx_buffer_allocator, best_block, remain);

			result = (void *)best_block->ptr;
		}
		else
		{
#ifndef NDEBUG
			printf("Buffer is not enough : %d", size);
			if (size > lack_amount)
				lack_amount = size;
#endif
		}
	}
	else
		result = NULL;

	if (result)
		printf_function();

	return result;
}

static inline int is_npx_buffer_addr(uintptr_t ptr)
{
	int valid = 0;
	if (npx_buffer_allocator)
	{
		if ((ptr >= BUFFER_START_ADDR) && ((ptr - BUFFER_MAX_SIZE) <= BUFFER_START_ADDR))
			valid = 1;
	}
	return valid;
}

int npx_buffer_allocator_push(void *ptr)
{
	int success = is_npx_buffer_addr((uintptr_t)ptr);
	if (success)
	{
		printf_function();
		buffer_block_t *temp;
		DL_FOREACH(npx_buffer_allocator, temp)
		{
			if (temp->ptr == ptr)
			{
				assert(temp->available == 0);
				int merge_to_next;
				int merge_to_prev;
				merge_to_next = temp->next && temp->next->available;
				merge_to_prev = (temp != npx_buffer_allocator) && temp->prev->available;

				if (merge_to_next && merge_to_prev)
				{
					buffer_block_t *update_block = temp->prev;
					buffer_block_t *delete1_block = temp->next;
					buffer_block_t *delete2_block = temp;
					update_block->size += temp->size;
					update_block->size += temp->next->size;

					DL_DELETE(npx_buffer_allocator, delete1_block);
					DL_DELETE(npx_buffer_allocator, delete2_block);
					free(delete1_block);
					free(delete2_block);
				}
				else if (merge_to_next)
				{
					buffer_block_t *update_block = temp->next;
					buffer_block_t *delete_block = temp;
					update_block->size += temp->size;
					update_block->ptr = temp->ptr;
					DL_DELETE(npx_buffer_allocator, delete_block);
					free(delete_block);
				}
				else if (merge_to_prev)
				{
					buffer_block_t *update_block = temp->prev;
					buffer_block_t *delete_block = temp;
					update_block->size += temp->size;
					DL_DELETE(npx_buffer_allocator, delete_block);
					free(delete_block);
				}
				else
				{
					temp->available = 1;
				}
				break;
			}
		}
	}
	return success;
}