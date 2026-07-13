#ifndef __NPX_TENSOR_H__
#define __NPX_TENSOR_H__

#include <stdint.h>
#include "ervp_matrix_op.h"
#include "ervp_special_matrix_op.h"
#include "ervp_sharedpointer.h"

typedef uint16_t npx_tensor_dim_size_t;

#ifndef NPX_TENSOR_MAX_DIM
#define NPX_TENSOR_MAX_DIM 4
#endif

typedef struct
{
	void *addr;
	npx_tensor_dim_size_t _size_array[NPX_TENSOR_MAX_DIM]; // 0:w, 1:h, 2:c, 3: timestep
	intptr_t _stride_array[NPX_TENSOR_MAX_DIM - 1];
	ervp_matrix_datatype_t datatype;
	uint8_t num_dim;
	uint8_t is_binary;
	uint8_t is_array_allocated;
	uint8_t is_sub;
	refcount_t *refcount;
} NpxTensorInfo;

NpxTensorInfo *npx_tensor_alloc_wo_data(int num_dim);
void npx_tensor_set_size_array(NpxTensorInfo *a, npx_tensor_dim_size_t *size_array);
void npx_tensor_alloc_data(NpxTensorInfo *a);
void npx_tensor_free(NpxTensorInfo *a);
NpxTensorInfo *npx_tensor_alloc(ervp_matrix_datatype_t datatype, int num_dim, npx_tensor_dim_size_t *size_array);
NpxTensorInfo *npx_tensor_generate_subtensor_info(NpxTensorInfo *a);
static inline NpxTensorInfo *npx_tensor_get_original_tensor(const NpxTensorInfo *a)
{
	assert(a);
	assert(a->refcount != NULL);
	assert(a->refcount->data != NULL);
	assert(((NpxTensorInfo *)a->refcount->data)->is_sub == 0);
	return a->refcount->data;
}

static inline void npx_tensor_set_stride(NpxTensorInfo *a, int dim_index, intptr_t value)
{
	assert(dim_index > 0);
	a->_stride_array[dim_index - 1] = value;
}

static inline intptr_t npx_tensor_get_stride(const NpxTensorInfo *a, int dim_index)
{
	intptr_t result;
	if (dim_index == 0)
	{
		assert(!matrix_datatype_is_subbyte(a->datatype));
		result = matrix_datatype_size(a->datatype);
	}
	else
		result = a->_stride_array[dim_index - 1];
	return result;
}

static inline intptr_t *npx_tensor_get_stride_array(NpxTensorInfo *a)
{
	return a->_stride_array;
}

static inline void npx_tensor_set_size(NpxTensorInfo *a, int dim_index, unsigned int value)
{
	a->_size_array[dim_index] = value;
}

static inline int npx_tensor_get_size(const NpxTensorInfo *a, int dim_index)
{
	return a->_size_array[dim_index];
}

static inline npx_tensor_dim_size_t *npx_tensor_get_size_array(NpxTensorInfo *a)
{
	return a->_size_array;
}

int npx_tensor_elements(const NpxTensorInfo *a);
static inline void npx_tensor_set_datatype(NpxTensorInfo *a, ervp_matrix_datatype_t datatype)
{
#ifndef USE_SUBBYTE_NPX
	assert(!matrix_datatype_is_subbyte(datatype));
#endif
	a->datatype = datatype;
}
static inline size_t npx_tensor_get_datasize(const NpxTensorInfo *a)
{
	return matrix_datatype_size(a->datatype);
}
void npx_tensor_set_contiguous_layout(NpxTensorInfo *a);
static inline int npx_tensor_has_contiguous_layout(const NpxTensorInfo *a)
{
	return ((matrix_datatype_get_num_bits(a->datatype) * npx_tensor_get_size(a, 0)) == (npx_tensor_get_stride(a, 1) << 3));
}

int npx_tensor_sizes(const NpxTensorInfo *a);

ErvpMatrixInfo *npx_tensor_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo *npx_tensor_to_flattened_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo *npx_tensor_to_iterative_matrix_info(const NpxTensorInfo *tensor, int num_channel, ErvpMatrixInfo *preallocated);
ErvpMatrixInfo **npx_tensor_to_matrix_info_list(const NpxTensorInfo *tensor, int num_channel, int num_info);
void npx_tensor_print(const NpxTensorInfo *tensor, int num_elements);

void npx_tensor_reshape(ervp_mop_mapping_t *mop_mapping, NpxTensorInfo *src, NpxTensorInfo *dst);
NpxTensorInfo *npx_tensor_permute(const NpxTensorInfo *a, NpxTensorInfo *b, int *dims);
NpxTensorInfo *npx_tensor_resize(const NpxTensorInfo *a, NpxTensorInfo *b, int h, int w);

NpxTensorInfo *npx_tensor_cast_sint8_to_float(const NpxTensorInfo *tensor);

static inline ervp_hwtask_busy_fx_t npx_tensor_zero(ervp_mop_mapping_t *mop_mapping, NpxTensorInfo *tensor)
{
	ervp_hwtask_busy_fx_t (*p_matrix_zero)(ervp_mop_mapping_t *mop_mapping, ErvpMatrixInfo *result);
	if (mop_mapping == NULL)
		p_matrix_zero = _matrix_zero_sw;
	else
		p_matrix_zero = mop_mapping->matrix_zero;
	assert(p_matrix_zero != NULL);

	ErvpMatrixInfo contiguous_info;
	npx_tensor_to_flattened_matrix_info(tensor, &contiguous_info);
	return p_matrix_zero(mop_mapping, &contiguous_info);
}

#endif