#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_memory_util.h"
#include "ervp_smart_flush.h"

#include "npx_malloc.h"
#include "npx_tensor.h"

NpxTensorInfo *npx_tensor_alloc_wo_data(int num_dim)
{
  assert(num_dim > 0);
  assert(num_dim <= NPX_TENSOR_MAX_DIM);
  NpxTensorInfo *a = (NpxTensorInfo *)malloc(sizeof(NpxTensorInfo));
  assert(a);

  a->addr = NULL;
  for (int i = 0; i < NPX_TENSOR_MAX_DIM; i++)
    a->_size_array[i] = 0;
  for (int i = 0; i < NPX_TENSOR_MAX_DIM - 1; i++)
    a->_stride_array[i] = 0;
  a->datatype = 0;
  a->num_dim = num_dim;
  a->is_binary = 0;
  a->is_array_allocated = 0;
  a->is_sub = 0;
  a->refcount = NULL;

  return a;
}

static inline int npx_tensor_row_size(const NpxTensorInfo *a)
{
  int row_size;
  if (a->num_dim < 1)
    row_size = 0;
  else
  {
    int row_bitsize = matrix_datatype_get_num_bits(a->datatype) * npx_tensor_get_size(a, 0);
    row_size = rshift_ru(row_bitsize, 3);
  }
  return row_size;
}

void npx_tensor_set_contiguous_layout(NpxTensorInfo *a)
{
  assert(a);
  assert(a->num_dim > 1);

  npx_tensor_set_stride(a, 1, npx_tensor_row_size(a));
  for (int i = 2; i < a->num_dim; i++)
  {
    assert(npx_tensor_get_stride(a, i) == 0);
    npx_tensor_set_stride(a, i, npx_tensor_get_stride(a, i - 1) * npx_tensor_get_size(a, i - 1));
  }
}

void npx_tensor_alloc_data(NpxTensorInfo *a)
{
  assert(a);
  npx_tensor_set_contiguous_layout(a);
  int alloc_size = npx_tensor_sizes(a);
  a->addr = npx_malloc(alloc_size);
  assert(a->addr);
  a->is_array_allocated = 1;
}

void npx_tensor_set_size_array(NpxTensorInfo *a, npx_tensor_dim_size_t *size_array)
{
  assert(a);
  assert(npx_tensor_get_size_array(a));
  assert(a->num_dim > 0);
  for (int i = 0; i < a->num_dim; i++)
    npx_tensor_set_size(a, i, size_array[i]);
}

NpxTensorInfo *npx_tensor_alloc(ervp_matrix_datatype_t datatype, int num_dim, npx_tensor_dim_size_t *size_array)
{
  NpxTensorInfo *result = npx_tensor_alloc_wo_data(num_dim);
  npx_tensor_set_size_array(result, size_array);
  npx_tensor_set_datatype(result, datatype);
  npx_tensor_alloc_data(result);
  return result;
}

static void __npx_tensor_free_except_refcount(NpxTensorInfo *a)
{
  assert(a);
  if (a->is_array_allocated)
    npx_free(a->addr);
  free(a);
}

void npx_tensor_free(NpxTensorInfo *a)
{
  assert(a);
  if (a->refcount)
    sharedpointer_free(a, a->refcount);
  else
    __npx_tensor_free_except_refcount(a);
}

NpxTensorInfo *npx_tensor_generate_subtensor_info(NpxTensorInfo *a)
{
  assert(a);
  NpxTensorInfo *result = npx_tensor_alloc_wo_data(a->num_dim);
  npx_tensor_set_size_array(result, npx_tensor_get_size_array(a));
  npx_tensor_set_datatype(result, a->datatype);
  for (int i = 1; i < a->num_dim; i++)
    npx_tensor_set_stride(result, i, 0); // MUST be set separately
  result->addr = a->addr;
  result->is_binary = a->is_binary;
  result->is_array_allocated = 0;
  result->is_sub = 1;

  if (a->refcount)
    a->refcount->count++;
  else
  {
    refcount_t *refcount = refcount_alloc(a, __npx_tensor_free_except_refcount);
    refcount->count = 2;
    a->refcount = refcount;
  }
  result->refcount = a->refcount;
  return result;
}

int npx_tensor_elements(const NpxTensorInfo *a)
{
  int num_value;
  if (a->num_dim < 1)
    num_value = 0;
  else
  {
    num_value = 1;
    for (int i = 0; i < a->num_dim; i++)
      num_value *= npx_tensor_get_size(a, i);
  }
  return num_value;
}

int npx_tensor_sizes(const NpxTensorInfo *a)
{
  int size;
  if (a->num_dim < 1)
    size = 0;
  else
  {
    size = npx_tensor_row_size(a);
    for (int i = 1; i < a->num_dim; i++)
      size *= npx_tensor_get_size(a, i);
  }
  return size;
}

/*
static int get_offset(const NpxTensorInfo *a, int index)
{
  int i;
  int offset = 0;
  int coord_of_current_axis;

  for (i = 0; i < a->num_dim; i++)
  {
    coord_of_current_axis = index % a->size[i];
    offset += coord_of_current_axis * a->stride[i];
    index /= a->size[i];
  }

  return offset;
}
*/

static int get_offset_from_coordinate(const NpxTensorInfo *a, int *coordinate)
{
  int offset = 0;
  int inverval;

  for (int i = 0; i < a->num_dim; i++)
  {
    offset += coordinate[i] * npx_tensor_get_stride(a, i);
  }

  return offset;
}

static inline void countup_coordinate(const NpxTensorInfo *a, int *coordinate, int dim_index)
{
  if ((dim_index == a->num_dim - 1) && (coordinate[dim_index] == npx_tensor_get_size(a, dim_index) - 1))
  {
    printf("Coordinates are out of range: dim_index %d\n", dim_index);
    assert(0);
  }
  else
    coordinate[dim_index] += 1;

  if (coordinate[dim_index] >= npx_tensor_get_size(a, dim_index))
  {
    countup_coordinate(a, coordinate, dim_index + 1);
    coordinate[dim_index] = 0;
  }
}

void npx_tensor_reshape(ervp_mop_mapping_t *mop_mapping, NpxTensorInfo *src, NpxTensorInfo *dst)
{
  int num_value_src = npx_tensor_elements(src);
  int num_value_dst = npx_tensor_elements(dst);
  assert(num_value_src == num_value_dst);

  if (npx_tensor_has_contiguous_layout(src) && npx_tensor_has_contiguous_layout(dst))
  {
    ErvpMatrixInfo contiguous_src;
    ErvpMatrixInfo contiguous_dst;
    matrix_generate_info(src->datatype, 1, num_value_src, src->addr, &contiguous_src);
    matrix_generate_info(dst->datatype, 1, num_value_dst, dst->addr, &contiguous_dst);
    ervp_hwtask_busy_fx_t hwtask_busy_fx;
    hwtask_busy_fx = mop_mapping->matrix_copy(mop_mapping, &contiguous_src, &contiguous_dst, 0);
    hwtask_wait_complete(hwtask_busy_fx);
  }
  else
    assert_not_implemented();
}

#if 0
void npx_tensor_reshape(const NpxTensorInfo *a, NpxTensorInfo *b)
{
  int i;
  int a_offset;
  int b_offset;
  int a_num_value = npx_tensor_elements(a);
  int b_num_value = npx_tensor_elements(b);
  size_t value_size = npx_tensor_get_datasize(a);

  // printf("src_num_value: %d\n", src_num_value);
  if (a_num_value != b_num_value)
  {
    printf("b tensor size is not equal to a tensor size!\n");
    assert(0);
  }

  for (i = 0; i < a_num_value; i++)
  {
    a_offset = get_offset(a, i);
    b_offset = get_offset(b, i);
    // printf("0x%08x\n0x%08x\n", a_offset, b_offset);
    // printf("%d %d\n", a_offset, b_offset);
    // memcpy(&b->addr[b_offset], &a->addr[a_offset], value_size);
    assert(a_offset==b_offset);
    memcpy(b->addr + b_offset, a->addr + a_offset, value_size);
  }
}
#endif
#if 0
void npx_tensor_reshape(const NpxTensorInfo *a, NpxTensorInfo *b)
{
  int i;
  int a_offset;
  int b_offset;
  int a_num_value = npx_tensor_elements(a);
  int b_num_value = npx_tensor_elements(b);
  size_t value_size = npx_tensor_get_datasize(a);

  // printf("src_num_value: %d\n", src_num_value);
  if (a_num_value != b_num_value)
  {
    printf("b tensor size is not equal to a tensor size!\n");
    assert(0);
  }

  int *a_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  int *b_coordinate = (int *)calloc(a->num_dim, sizeof(int));

  for (i = 0; i < a_num_value; i++)
  {
    a_offset = get_offset_from_coordinate(a, a_coordinate);
    b_offset = get_offset_from_coordinate(b, b_coordinate);
    // printf("0x%08x\n0x%08x\n", a_offset, b_offset);
    // printf("%d %d\n", a_offset, b_offset);
    // memcpy(&b->addr[b_offset], &a->addr[a_offset], value_size);
    memcpy(b->addr + b_offset, a->addr + a_offset, value_size);
    if (i < (a_num_value - 1))
    {
      countup_coordinate(a, a_coordinate, 0);
      countup_coordinate(b, b_coordinate, 0);
    }
  }
}
#endif

NpxTensorInfo *npx_tensor_permute(const NpxTensorInfo *a, NpxTensorInfo *b, int *dims)
{
  assert(!matrix_datatype_is_subbyte(a->datatype));
  NpxTensorInfo *result;
  int a_offset;
  int b_offset;

  int *a_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  int *b_coordinate = (int *)calloc(a->num_dim, sizeof(int));

  if (b != NULL)
  {
    result = b;
    for (int dim_index = 0; dim_index < a->num_dim; dim_index++)
      npx_tensor_set_size(result, dim_index, npx_tensor_get_size(a, dims[dim_index]));
  }
  else
  {
    result = npx_tensor_alloc_wo_data(a->num_dim);
    for (int dim_index = 0; dim_index < a->num_dim; dim_index++)
      npx_tensor_set_size(result, dim_index, npx_tensor_get_size(a, dims[dim_index]));
    npx_tensor_set_datatype(result, a->datatype);
    npx_tensor_alloc_data(result);
  }

  int num_value = npx_tensor_elements(a);

  for (int i = 0; i < num_value; i++)
  {
    for (int dim_index = 0; dim_index < a->num_dim; dim_index++)
      b_coordinate[dim_index] = a_coordinate[dims[dim_index]];

    a_offset = get_offset_from_coordinate(a, a_coordinate);
    b_offset = get_offset_from_coordinate(result, b_coordinate);
    // printf("%d %d\n", a_offset, b_offset);

    // memcpy(&result->addr[b_offset], &a->addr[a_offset], value_size);
    memcpy(result->addr + b_offset, a->addr + a_offset, npx_tensor_get_datasize(a));

    if (i < (num_value - 1))
      countup_coordinate(a, a_coordinate, 0);
  }

  return result;
}

static void resize_nearest_int8(char *a, int a_h, int a_w, char *b, int b_h, int b_w)
{
  char (*a_2d)[a_w] = a;
  char (*b_2d)[b_w] = b;

  float w_ratio = (float)a_w / b_w;
  float h_ratio = (float)a_h / b_h;

  int a_hidx;
  int a_widx;
  int h, w;
  for (h = 0; h < b_h; h++)
  {
    a_hidx = (int)((float)h * h_ratio);
    for (w = 0; w < b_w; w++)
    {
      a_widx = (int)((float)w * w_ratio);
      b_2d[h][w] = a_2d[a_hidx][a_widx];
    }
  }
}

static void npx_resize_recursive(NpxTensorInfo *a, NpxTensorInfo *b, int cur_dim)
{
  if (cur_dim == 2)
  {
    resize_nearest_int8(a->addr, npx_tensor_get_size(a, 1), npx_tensor_get_size(a, 0), b->addr, npx_tensor_get_size(b, 1), npx_tensor_get_size(b, 0));
    // a->addr += a->stride[2];
    // b->addr += b->stride[2];
    a->addr += npx_tensor_get_size(a, 1) * npx_tensor_get_stride(a, 1);
    b->addr += npx_tensor_get_size(b, 1) * npx_tensor_get_stride(b, 1);
  }
  else
  {
    for (int i = 0; i < npx_tensor_get_size(a, cur_dim - 1); i++)
    {
      npx_resize_recursive(a, b, cur_dim - 1);
    }
  }
}

NpxTensorInfo *npx_tensor_resize(const NpxTensorInfo *a, NpxTensorInfo *b, int h, int w)
{
  assert(!matrix_datatype_is_subbyte(a->datatype));
  NpxTensorInfo *result;
  assert(a->num_dim >= 3);
  if ((npx_tensor_get_size(a, 1) == h) && (npx_tensor_get_size(a, 0) == w))
  {
    result = a;
    return result;
  }
  if (b == NULL)
  {
    result = npx_tensor_alloc_wo_data(a->num_dim);
    npx_tensor_set_size_array(result, npx_tensor_get_size_array(a));
    npx_tensor_set_size(result, 0, w);
    npx_tensor_set_size(result, 1, h);
    // printf("flatten ndata %d\n", result->size[1]);
    npx_tensor_set_datatype(result, a->datatype);
    npx_tensor_alloc_data(result);
  }
  else
  {
    result = b;
  }
  NpxTensorInfo tmp_a = *a;
  NpxTensorInfo tmp_result = *result;

  npx_resize_recursive(&tmp_a, &tmp_result, a->num_dim);

  trackedvar_add(a->addr, 0);
  trackedvar_add(b->addr, 1);

  return result;
}

static inline ErvpMatrixInfo *_convert_to_matrix_info(const NpxTensorInfo *tensor, int size1, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result = matrix_generate_info(tensor->datatype, size1, npx_tensor_get_size(tensor, 0), tensor->addr, preallocated);
  matrix_set_stride(result, npx_tensor_get_stride(tensor, 1));
  result->is_binary = tensor->is_binary;
  result->is_sub = tensor->is_sub;
  return result;
}

static inline ErvpMatrixInfo *_convert_2dim_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  return _convert_to_matrix_info(tensor, npx_tensor_get_size(tensor, 1), preallocated);
}

ErvpMatrixInfo *npx_tensor_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  assert(tensor->num_dim == 2);
  return _convert_2dim_to_matrix_info(tensor, preallocated);
}

ErvpMatrixInfo *npx_tensor_to_flattened_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  assert(npx_tensor_has_contiguous_layout(tensor));
  assert(tensor->num_dim >= 2);
  int size1 = 1;
  for (int i = 1; i < tensor->num_dim; i++)
    size1 *= npx_tensor_get_size(tensor, i);
  return _convert_to_matrix_info(tensor, size1, preallocated);
}

ErvpMatrixInfo *npx_tensor_to_iterative_matrix_info(const NpxTensorInfo *tensor, int num_channel, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result;
  assert(tensor);
  if (preallocated == NULL)
  {
    // assert(tensor->num_dim >= 3);
    result = _convert_2dim_to_matrix_info(tensor, NULL);
    result->is_sub = 1;
  }
  else
  {
    assert(preallocated->datatype == tensor->datatype);
    assert(preallocated->num_col == npx_tensor_get_size(tensor, 0));
    assert(preallocated->num_row == npx_tensor_get_size(tensor, 1));
    result = preallocated;
    assert(tensor->num_dim >= 3); // DO NOT move to upward
    result->addr += num_channel * npx_tensor_get_stride(tensor, 2);
  }
  return result;
}

ErvpMatrixInfo **npx_tensor_to_matrix_info_list(const NpxTensorInfo *tensor, int num_channel, int num_info)
{
  assert(tensor);
  assert(num_info | num_channel);
  assert(num_info * num_channel * npx_tensor_get_size(tensor, 1) * npx_tensor_get_size(tensor, 0) <= npx_tensor_elements(tensor));

  ErvpMatrixInfo **result;
  result = calloc(sizeof(ErvpMatrixInfo *), num_info);
  for (int i = 0; i < num_info; i++)
  {
    result[i] = _convert_2dim_to_matrix_info(tensor, result[i]);
    result[i]->addr += (i * num_channel * npx_tensor_get_stride(tensor, 2));
    result[i]->is_sub = 1;
  }
  return result;
}

void npx_tensor_print(const NpxTensorInfo *tensor, int num_elements)
{
  assert(tensor);
  assert(tensor->num_dim > 0);
  printf("\nDim: %d = %d", tensor->num_dim, npx_tensor_get_size(tensor, tensor->num_dim - 1));
  for (int i = 1; i < tensor->num_dim; i++)
    printf(" x %d", npx_tensor_get_size(tensor, tensor->num_dim - 1 - i));
  printf("\nStride: %d", npx_tensor_get_stride(tensor, tensor->num_dim - 1));
  for (int i = 1; i < tensor->num_dim; i++)
    printf(" / %d", npx_tensor_get_stride(tensor, tensor->num_dim - 1 - i));

  int num_print = npx_tensor_elements(tensor);
  if ((num_elements >= 0) && (num_elements < num_print))
    num_print = num_elements;

  ErvpMatrixInfo *minfo = NULL;
  while (num_print > 0)
  {
    minfo = npx_tensor_to_iterative_matrix_info(tensor, 1, minfo);
    matrix_print(minfo);
    num_print -= matrix_num_elements(minfo);
  }
}

NpxTensorInfo *npx_tensor_cast_sint8_to_float(const NpxTensorInfo *tensor)
{
  // printf("\n%s\n", __func__);
  assert(tensor->datatype == MATRIX_DATATYPE_SINT08);
  int i;
  NpxTensorInfo *result;
  result = npx_tensor_alloc_wo_data(tensor->num_dim);
  npx_tensor_set_size_array(result, npx_tensor_get_size_array(tensor));
  npx_tensor_set_datatype(result, MATRIX_DATATYPE_FLOAT32);
  npx_tensor_alloc_data(result);

  int8_t *input = tensor->addr;
  float *output = result->addr;
  int num_elements = npx_tensor_elements(tensor);
  for (i = 0; i < num_elements; i++)
    output[i] = (float)input[i];

  trackedvar_add(tensor->addr, 0);
  trackedvar_add(result->addr, 1);

  return result;
}