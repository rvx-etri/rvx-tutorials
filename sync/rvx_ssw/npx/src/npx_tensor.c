#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"

#include "npx_tensor.h"

NpxTensorInfo *npx_tensor_alloc_wo_data(int num_dim)
{
  NpxTensorInfo *a = (NpxTensorInfo *)malloc(sizeof(NpxTensorInfo));
  assert(a);

  a->addr = NULL;
  a->size = (int *)calloc(num_dim, sizeof(int));
  a->stride = (unsigned int *)calloc(num_dim, sizeof(int));
  a->datatype = 0;
  a->num_dim = num_dim;
  a->is_binary = 0;
  a->is_array_allocated = 0;
  a->refcount = NULL;

  return a;
}

void npx_tensor_set_contiguous_layout(NpxTensorInfo *a)
{
  assert(npx_tensor_get_datasize(a) > 0); // stride[0] is data size of value.

  for (int i = 1; i < a->num_dim; i++)
  {
    assert(a->stride[i] == 0);
    a->stride[i] = a->stride[i - 1] * a->size[i - 1];
  }
}

void npx_tensor_alloc_data(NpxTensorInfo *a)
{
  npx_tensor_set_contiguous_layout(a);
  int alloc_size = npx_tensor_sizes(a);
  a->addr = (void *)malloc(alloc_size);
  assert(a->addr);
  a->is_array_allocated = 1;
}

void npx_tensor_set_sizes(NpxTensorInfo *a, int *size)
{
  assert(a);
  assert(a->size);
  assert(a->num_dim > 0);
  for (int i = 0; i < a->num_dim; i++)
    a->size[i] = size[i];
}

NpxTensorInfo *npx_tensor_alloc(int datatype, int num_dim, int *size)
{
  NpxTensorInfo *result = npx_tensor_alloc_wo_data(num_dim);
  npx_tensor_set_sizes(result, size);
  npx_tensor_set_datatype(result, datatype);
  npx_tensor_alloc_data(result);
  return result;
}

static void __npx_tensor_free_except_refcount(NpxTensorInfo *a)
{
  assert(a);
  free(a->size);
  free(a->stride);
  if (a->is_array_allocated)
    free(a->addr);
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
  NpxTensorInfo *result = npx_tensor_alloc_wo_data(a->num_dim);
  npx_tensor_set_sizes(result, a->size);
  npx_tensor_set_datatype(result, a->datatype);
  for (int i = 0; i < a->num_dim; i++)
    result->stride[i] = a->stride[i];
  result->is_binary = a->is_binary;
  result->addr = a->addr;
  // result->is_array_allocated = 0;

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
  int i;
  int num_value = 1;

  if (a->num_dim < 1)
    num_value = 0;
  else
  {
    num_value = 1;
    for (i = 0; i < a->num_dim; i++)
    {
      num_value *= a->size[i];
    }
  }
  return num_value;
}

int npx_tensor_set_datatype(NpxTensorInfo *a, int datatype)
{
  assert(!matrix_datatype_is_subbyte(datatype));
  a->datatype = datatype;
  a->stride[0] = matrix_datatype_size(datatype);
}

static int get_offset(const NpxTensorInfo *a, int index)
{
  int i;
  int offset = 0;
  int coord_of_current_axis;

  /* x dim more than 0-dim */
  for (i = 0; i < a->num_dim; i++)
  {
    coord_of_current_axis = index % a->size[i];
    offset += coord_of_current_axis * a->stride[i];
    index /= a->size[i];
  }

  return offset;
}

static int get_offset_from_coordinate(const NpxTensorInfo *a, int *coordinate)
{
  int offset = 0;
  int inverval;

  for (int i = 0; i < a->num_dim; i++)
  {
    offset += coordinate[i] * a->stride[i];
  }

  return offset;
}

static inline void countup_coordinate(const NpxTensorInfo *a, int *coordinate, int dim_index)
{
  if ((dim_index == a->num_dim - 1) && (coordinate[dim_index] == a->size[dim_index] - 1))
  {
    printf("Coordinates are out of range: dim_index %d\n", dim_index);
    assert(0);
  }
  else
    coordinate[dim_index] += 1;

  if (coordinate[dim_index] >= a->size[dim_index])
  {
    countup_coordinate(a, coordinate, dim_index + 1);
    coordinate[dim_index] = 0;
  }
}

void npx_tensor_reshape(const NpxTensorInfo *a, NpxTensorInfo *b)
{
  int num_value_a = npx_tensor_elements(a);
  int num_value_b = npx_tensor_elements(b);
  assert(num_value_a == num_value_b);
  size_t datasize_a = npx_tensor_get_datasize(a);
  size_t datasize_b = npx_tensor_get_datasize(b);
  if (datasize_a == datasize_b)
  {
    memcpy(b->addr, a->addr, num_value_a * datasize_a);
  }
  else
    assert(0);
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
  NpxTensorInfo *result;
  int a_offset;
  int b_offset;

  size_t value_size = npx_tensor_get_datasize(a);

  int *a_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  int *b_coordinate = (int *)calloc(a->num_dim, sizeof(int));

  if (b != NULL)
  {
    result = b;
    for (int dim_index = 0; dim_index < a->num_dim; dim_index++)
      result->size[dim_index] = a->size[dims[dim_index]];
  }
  else
  {
    result = npx_tensor_alloc_wo_data(a->num_dim);
    for (int dim_index = 0; dim_index < a->num_dim; dim_index++)
      result->size[dim_index] = a->size[dims[dim_index]];
    result->stride[0] = value_size;
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
    memcpy(result->addr + b_offset, a->addr + a_offset, value_size);

    if (i < (num_value - 1))
      countup_coordinate(a, a_coordinate, 0);
  }

  return result;
}

void resize_nearest_int8(char *a, int a_h, int a_w, char *b, int b_h, int b_w)
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

void npx_resize_recursive(NpxTensorInfo *a, NpxTensorInfo *b, int cur_dim)
{
  if (cur_dim == 2)
  {
    resize_nearest_int8(a->addr, a->size[1], a->size[0], b->addr, b->size[1], b->size[0]);
    // a->addr += a->stride[2];
    // b->addr += b->stride[2];
    a->addr += a->size[1] * a->stride[1];
    b->addr += b->size[1] * b->stride[1];
  }
  else
  {
    for (int i = 0; i < a->size[cur_dim - 1]; i++)
    {
      npx_resize_recursive(a, b, cur_dim - 1);
    }
  }
}

NpxTensorInfo *npx_tensor_resize(const NpxTensorInfo *a, NpxTensorInfo *b, int h, int w)
{
  NpxTensorInfo *result;
  assert(a->num_dim >= 3);
  if ((a->size[1] == h) && (a->size[0] == w))
  {
    result = a;
    return result;
  }
  if (b == NULL)
  {
    result = npx_tensor_alloc_wo_data(a->num_dim);
    result->size[0] = w;
    result->size[1] = h;
    for (int i = 2; i < a->num_dim; i++)
      result->size[i] = a->size[i];
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

  return result;
}

static inline ErvpMatrixInfo *convert_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result = matrix_generate_info(tensor->datatype, tensor->size[1], tensor->size[0], tensor->addr, preallocated);
  result->is_binary = tensor->is_binary;
  return result;
}

ErvpMatrixInfo *npx_tensor_to_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  assert(tensor->num_dim == 2);
  return matrix_generate_info(tensor->datatype, tensor->size[1], tensor->size[0], tensor->addr, preallocated);
}

ErvpMatrixInfo *npx_tensor_to_flattened_matrix_info(const NpxTensorInfo *tensor, ErvpMatrixInfo *preallocated)
{
  return matrix_generate_info(tensor->datatype, npx_tensor_elements(tensor) / tensor->size[0], tensor->size[0], tensor->addr, preallocated);
}

ErvpMatrixInfo *npx_tensor_to_iterative_matrix_info(const NpxTensorInfo *tensor, int num_channel, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result;
  assert(tensor);
  if (preallocated == NULL)
    result = convert_to_matrix_info(tensor, NULL);
  else
  {
    assert(preallocated->datatype == tensor->datatype);
    assert(preallocated->num_col == tensor->size[0]);
    assert(preallocated->num_row == tensor->size[1]);
    result = preallocated;
    assert(tensor->num_dim >= 3); // DO NOT move to upward
    result->addr += num_channel * tensor->stride[2];
  }
  return result;
}

ErvpMatrixInfo **npx_tensor_to_matrix_info_list(const NpxTensorInfo *tensor, int num_channel, int num_info)
{
  assert(tensor);
  assert(num_info | num_channel);
  assert(num_info * num_channel * tensor->size[1] * tensor->size[0] <= npx_tensor_elements(tensor));

  ErvpMatrixInfo **result;
  result = calloc(sizeof(ErvpMatrixInfo *), num_info);
  for (int i = 0; i < num_info; i++)
  {
    result[i] = convert_to_matrix_info(tensor, result[i]);
    result[i]->addr += (i * num_channel * tensor->stride[2]);
  }
  return result;
}

void npx_tensor_print(const NpxTensorInfo *tensor, int num_elements)
{
  assert(tensor);
  assert(tensor->num_dim > 0);
  printf("\nDim: %d = %d", tensor->num_dim, tensor->size[tensor->num_dim - 1]);
  for (int i = 1; i < tensor->num_dim; i++)
    printf(" x %d", tensor->size[tensor->num_dim - 1 - i]);
  printf("\nStride: %d", tensor->stride[tensor->num_dim - 1]);
  for (int i = 1; i < tensor->num_dim; i++)
    printf(" x %d", tensor->stride[tensor->num_dim - 1 - i]);

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
  for (i = 0; i < tensor->num_dim; i++)
    result->size[i] = tensor->size[i];
  npx_tensor_set_datatype(result, MATRIX_DATATYPE_FLOAT32);
  npx_tensor_alloc_data(result);

  int8_t *input = tensor->addr;
  float *output = result->addr;
  int num_elements = npx_tensor_elements(tensor);
  for (i = 0; i < num_elements; i++)
    output[i] = (float)input[i];

  return result;
}