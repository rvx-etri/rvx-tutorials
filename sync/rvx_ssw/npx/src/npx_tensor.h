#ifndef __NPX_TENSOR_H__
#define __NPX_TENSOR_H__

#include <stdint.h>
#include "platform_info.h"
#include "ervp_matrix.h"
#include "ervp_sharedpointer.h"

typedef struct
{
	void *addr;
	int *size;						// 0:w, 1:h, 2:c
	unsigned int *stride; // stride[0] is data size of value.
	int datatype;
	uint8_t num_dim;
	uint8_t is_binary;
	uint8_t is_array_allocated;
	refcount_t *refcount;
} NpxTensorInfo;

NpxTensorInfo *npx_tensor_alloc_wo_data(int num_dim);
void npx_tensor_set_sizes(NpxTensorInfo *a, int *size);
void npx_tensor_alloc_data(NpxTensorInfo *a);
void npx_tensor_free(NpxTensorInfo *a);
NpxTensorInfo *npx_tensor_alloc(int datatype, int num_dim, int *size);
NpxTensorInfo *npx_tensor_generate_subtensor_info(NpxTensorInfo *a);

int npx_tensor_elements(const NpxTensorInfo *a);
int npx_tensor_set_datatype(NpxTensorInfo *a, int datatype);
void npx_tensor_set_contiguous_layout(NpxTensorInfo *a);
static inline size_t npx_tensor_get_datasize(const NpxTensorInfo *a)
{
	return a->stride[0];
}

static inline int npx_tensor_sizes(const NpxTensorInfo *a)
{
	return npx_tensor_get_datasize(a) * npx_tensor_elements(a);
}

ErvpMatrixInfo *npx_tensor_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo *npx_tensor_to_flattened_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo *npx_tensor_to_iterative_matrix_info(const NpxTensorInfo *tensor, int num_channel, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo **npx_tensor_to_matrix_info_list(const NpxTensorInfo *tensor, int num_channel, int num_info);
void npx_tensor_print(const NpxTensorInfo *tensor, int num_elements);

void npx_tensor_reshape(const NpxTensorInfo *a, NpxTensorInfo *b);
NpxTensorInfo *npx_tensor_permute(const NpxTensorInfo *a, NpxTensorInfo *b, int *dims);
NpxTensorInfo *npx_tensor_resize(const NpxTensorInfo *a, NpxTensorInfo *b, int h, int w);

NpxTensorInfo *npx_tensor_cast_sint8_to_float(const NpxTensorInfo *tensor);

#endif