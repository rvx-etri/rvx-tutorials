#include "ervp_printf.h"
#include "ervp_assert.h"
#include "ervp_malloc.h"

#include "ervp_tensor.h"

ErvpTensorInfo *tensor_make(int num_dim)//, int datatype)
{
  ErvpTensorInfo *a = (ErvpTensorInfo *)malloc(sizeof(ErvpTensorInfo));
  assert(a);

  a->addr = NULL;
  a->size = (int *)calloc(num_dim, sizeof(int));
  a->stride = (int *)calloc(num_dim, sizeof(int));
  a->num_dim = num_dim;
  //a->datatype = datatype;

  return a;
}

void tensor_alloc(ErvpTensorInfo *a)
{
  assert(a->stride[0]>0); // stride[0] is data size of value.

  for(int i=1; i<a->num_dim; i++)
  {
    if(a->stride[i]==0)
    {
        a->stride[i] = a->stride[i-1] * a->size[i-1];
    }
  }

  int alloc_size = a->size[a->num_dim - 1] * a->stride[a->num_dim - 1];
  //printf("\nalloc_size: %d", alloc_size);
  a->addr = (void *)malloc(alloc_size);
  assert(a->addr);
}

int tensor_num_value(const ErvpTensorInfo *a)
{
  int i;
  int num_value = 1;

  if(a->num_dim < 1)
    num_value = 0;
  else
  {
    num_value = 1;
    for(i = 0; i < a->num_dim; i++)
    {
      num_value *= a->size[i];
    }
  }
  return num_value;
}

static int get_offset(const ErvpTensorInfo *a, int index)
{
  int i;
  int offset = 0;
  int coord_of_current_axis;
  
  /* x dim more than 0-dim */
  for(i = 0; i < a->num_dim; i++)
  {
    coord_of_current_axis = index % a->size[i];
    offset += coord_of_current_axis * a->stride[i];
    index /= a->size[i];
  }

  return offset;
}

static int get_offset_from_coordinate(const ErvpTensorInfo *a, int *coordinate)
{
  int offset = 0;
  int inverval;

  for(int i = 0; i < a->num_dim; i++)
  {
    offset += coordinate[i] * a->stride[i];
  }

  return offset;
}

static inline void countup_coordinate(const ErvpTensorInfo *a, int *coordinate, int dim_index)
{
  if( (dim_index == a->num_dim-1) && (coordinate[dim_index] == a->size[dim_index]-1) )
  { 
    printf("Coordinates are out of range: dim_index %d\n", dim_index);
    assert(0);
  }
  else
    coordinate[dim_index] += 1;

  if(coordinate[dim_index] >= a->size[dim_index])
  {
    countup_coordinate(a, coordinate, dim_index+1);
    coordinate[dim_index] = 0;
  }
}

#if 1
void tensor_reshape(const ErvpTensorInfo *a, ErvpTensorInfo *b)
{
  int i;
  int a_offset;
  int b_offset;
  int a_num_value = tensor_num_value(a);
  int b_num_value = tensor_num_value(b);
  size_t value_size = a->stride[0];
 
  //printf("src_num_value: %d\n", src_num_value);
  if(a_num_value != b_num_value)
  {
    printf("b tensor size is not equal to a tensor size!\n");
    assert(0);
  }
  
  for(i = 0; i < a_num_value; i++)
  {
    a_offset = get_offset(a, i);
    b_offset = get_offset(b, i);
    //printf("0x%08x\n0x%08x\n", a_offset, b_offset);
    //printf("%d %d\n", a_offset, b_offset);
    //memcpy(&b->addr[b_offset], &a->addr[a_offset], value_size);
    memcpy(b->addr+b_offset, a->addr+a_offset, value_size);
  }
}
#else
void tensor_reshape(const ErvpTensorInfo *a, ErvpTensorInfo *b)
{
  int i;
  int a_offset;
  int b_offset;
  int a_num_value = tensor_num_value(a);
  int b_num_value = tensor_num_value(b);
  size_t value_size = a->stride[0];
 
  //printf("src_num_value: %d\n", src_num_value);
  if(a_num_value != b_num_value)
  {
    printf("b tensor size is not equal to a tensor size!\n");
    assert(0);
  }

  int *a_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  int *b_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  
  for(i = 0; i < a_num_value; i++)
  {
    a_offset = get_offset_from_coordinate(a, a_coordinate);
    b_offset = get_offset_from_coordinate(b, b_coordinate);
    //printf("0x%08x\n0x%08x\n", a_offset, b_offset);
    //printf("%d %d\n", a_offset, b_offset);
    //memcpy(&b->addr[b_offset], &a->addr[a_offset], value_size);
    memcpy(b->addr+b_offset, a->addr+a_offset, value_size);
    if(i < (a_num_value-1))
    {
      countup_coordinate(a, a_coordinate, 0);
      countup_coordinate(b, b_coordinate, 0);
    }
  }
}
#endif

ErvpTensorInfo *tensor_permute(const ErvpTensorInfo *a, ErvpTensorInfo *b, int *dims)
{
  ErvpTensorInfo *result;
  int a_offset;
  int b_offset;
  
  size_t value_size = a->stride[0];

  int *a_coordinate = (int *)calloc(a->num_dim, sizeof(int));
  int *b_coordinate = (int *)calloc(a->num_dim, sizeof(int));

  if(b!=NULL)
  {
    result = b;
    for(int dim_index = 0; dim_index < a->num_dim; dim_index++)
      result->size[dim_index] = a->size[dims[dim_index]];
  }
  else
  {
    result = tensor_make(a->num_dim);
    for(int dim_index = 0; dim_index < a->num_dim; dim_index++)
      result->size[dim_index] = a->size[dims[dim_index]];
    result->stride[0] = value_size;
    tensor_alloc(result);
  }
  
  int num_value = tensor_num_value(a);
  
  for(int i = 0; i < num_value; i++)
  {
    for(int dim_index = 0; dim_index < a->num_dim; dim_index++)
      b_coordinate[dim_index] = a_coordinate[dims[dim_index]];

    a_offset = get_offset_from_coordinate(a, a_coordinate);
    b_offset = get_offset_from_coordinate(result, b_coordinate);
    //printf("%d %d\n", a_offset, b_offset);

    //memcpy(&result->addr[b_offset], &a->addr[a_offset], value_size);
    memcpy(result->addr+b_offset, a->addr+a_offset, value_size);

    if(i < (num_value-1))
      countup_coordinate(a, a_coordinate, 0);
  }

  return result;
}
