#include "ervp_matrix.h"
#include "ervp_matrix_element.h"
#include "ervp_matrix_op.h"
#include "ervp_assert.h"
#include "ervp_printf.h"
#include "ervp_float.h"
#include "ervp_malloc.h"
#include "ervp_memory_util.h"
#include "ervp_smart_flush.h"

int matrix_num_bytes(const ErvpMatrixInfo *info)
{
  assert(matrix_get_num_bytes_per_row(info) == rshift_ru(info->stride_ls3, 3));
  int num_bytes = rshift_ru(info->stride_ls3 * info->num_row, 3);
  return num_bytes;
}

ErvpMatrixInfo *matrix_generate_info(ervp_matrix_datatype_t datatype, int num_row, int num_col, void *array_1d, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result;
  if (preallocated != NULL)
  {
    // DO NOT check the below
    /*
    assert(preallocated->datatype==datatype);
    assert(preallocated->num_row==num_row);
    assert(preallocated->num_col==num_col);
    if(preallocated->is_array_allocated)
      free(preallocated->addr);
    */
    result = preallocated;
  }
  else
  {
    result = malloc(sizeof(ErvpMatrixInfo));
    assert(result);
  }

  result->datatype = datatype;
  result->num_row = num_row;
  result->num_col = num_col;
  result->stride_ls3 = ALIGN_UP_POW2(matrix_get_num_bits_per_row(result), 8);
  result->addr = array_1d;

  result->is_array_allocated = 0;
  result->is_binary = (datatype == MATRIX_DATATYPE_UINT01);
  result->bit_offset = 0;
  result->is_sub = 0;
  result->is_scalar = 0;

  return result;
}

ErvpMatrixInfo *matrix_generate_submatrix_info(const ErvpMatrixInfo *original_matrix, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result;
  if (preallocated != NULL)
  {
    // DO NOT check the below
    /*
    assert(preallocated->datatype==datatype);
    assert(preallocated->num_row==num_row);
    assert(preallocated->num_col==num_col);
    if(preallocated->is_array_allocated)
      free(preallocated->addr);
    */
    result = preallocated;
  }
  else
  {
    result = malloc(sizeof(ErvpMatrixInfo));
    assert(result);
  }
  *result = *original_matrix;
  result->is_array_allocated = 0;
  result->is_sub = 1;
  return result;
}

ErvpMatrixInfo *matrix_alloc(ervp_matrix_datatype_t datatype, int num_row, int num_col, ErvpMatrixInfo *preallocated)
{
  ErvpMatrixInfo *result;
  result = matrix_generate_info(datatype, num_row, num_col, NULL, preallocated);
  int row_size = matrix_get_num_bytes_per_row(result);
  result->addr = trackedvar_malloc(row_size * num_row);
  assert(result->addr);
  result->is_array_allocated = 1;
  matrix_set_stride(result, row_size);
  return result;
}

void matrix_free(ErvpMatrixInfo *ptr)
{
  assert(ptr);
  if (ptr->is_array_allocated)
    trackedvar_free(ptr->addr);
  free(ptr);
}

void matrix_list_free(ErvpMatrixInfo **ptr, int num)
{
  assert(ptr);
  for (int i = 0; i < num; i++)
    matrix_free(ptr[i]);
  free(ptr);
}

static int _matrix_equal_float(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, int prints)
{
  assert(result->datatype == MATRIX_DATATYPE_FLOAT32);
  assert(ref->datatype == MATRIX_DATATYPE_FLOAT32);

  int all_are_equal = 1;
  int num_similar = 0;
  for (int i = 0; i < result->num_row; i++)
  {
    float *result_row_addr = (float *)matrix_get_row_addr(result, i);
    float *ref_row_addr = (float *)matrix_get_row_addr(ref, i);
    for (int j = 0; j < result->num_col; j++)
    {
      int similarity = get_float32_similarity(result_row_addr[j], ref_row_addr[j]);
      if (similarity == 32)
        continue;
      if (similarity >= 8)
      {
        num_similar++;
        continue;
      }

      all_are_equal = 0;
      printf("\n[%d,%d] 0x%08x", i, j, &(result_row_addr[j]));
      Float32Binary result_binary, ref_binary;
      result_binary.value = result_row_addr[j];
      ref_binary.value = ref_row_addr[j];
      printf("\n0x%08x, 0x%08x", result_binary.hex, ref_binary.hex);
      printf("\n%f, %f", result_binary.value, ref_binary.value);
    }
  }
  if (prints)
  {
    if (all_are_equal)
    {
      if (num_similar == 0)
        printf_must("\nall correct");
      else
        printf_must("\nsimilar %d", num_similar);
    }
    else
      printf_must("\nsome incorrect");
  }
  return all_are_equal;
}

static int _matrix_equal_fixed(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, int prints)
{
  int all_are_equal = 1;
  for (int i = 0; i < result->num_row; i++)
  {
    for (int j = 0; j < result->num_col; j++)
    {
      UNKNOWN_TYPE result_element = _matrix_read_element(result, i, j);
      UNKNOWN_TYPE ref_element = _matrix_read_element(ref, i, j);
      if (result_element.value_signed != ref_element.value_signed)
      {
        printf("\nDiff: (%d, %d) %x %x", i, j, result_element.value_signed, ref_element.value_signed);
        all_are_equal = 0;
        break;
      }
    }
    if (!all_are_equal)
      break;
  }
  if (prints)
  {
    if (all_are_equal)
      printf_must("\nall correct");
    else
      printf_must("\nsome incorrect");
  }
  return all_are_equal;
}

static int _matrix_equal_fast(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, int prints)
{
  int all_are_equal = 1;
  int row_size = matrix_get_num_bytes_per_row(result);
  for (int i = 0; i < result->num_row; i++)
  {
    void *result_row_addr = matrix_get_row_addr(result, i);
    void *ref_row_addr = matrix_get_row_addr(ref, i);
    all_are_equal &= memory_compare(result_row_addr, ref_row_addr, row_size, 0);
    if (all_are_equal == 0)
    {
      printf("\nDiff row: %d", i);
      break;
    }
  }
  if (prints)
  {
    if (all_are_equal)
      printf("\nall correct");
    else
      printf("\nsome incorrect");
  }
  return all_are_equal;
}

__attribute__((weak)) int matrix_equal(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, int prints)
{
  int all_are_equal;
  assert(result);
  assert(result->addr);
  assert(ref);
  assert(ref->addr);
  all_are_equal = matrix_is_same_size(result, ref);
  if (!all_are_equal)
    ;
  else if (matrix_datatype_is_float(result->datatype))
    all_are_equal = _matrix_equal_float(result, ref, prints);
  else
  {
    int is_fast_possible = 1;
    if (result->datatype != ref->datatype)
      is_fast_possible = 0;
    int num_bits_per_row;
    num_bits_per_row = matrix_get_num_bits_per_row(result);
    if (num_bits_per_row < 8)
      is_fast_possible = 0;
    else if ((num_bits_per_row & 7) != 0)
      is_fast_possible = 0;
    num_bits_per_row = matrix_get_num_bits_per_row(ref);
    if (num_bits_per_row < 8)
      is_fast_possible = 0;
    else if ((num_bits_per_row & 7) != 0)
      is_fast_possible = 0;
    if (is_fast_possible)
      all_are_equal = _matrix_equal_fast(result, ref, prints);
    else
      all_are_equal = _matrix_equal_fixed(result, ref, prints);
  }
  return all_are_equal;
}

int matrix_equal_one_by_one(const ErvpMatrixInfo *result, const ErvpMatrixInfo *ref, float error_rate, int prints)
{
  int all_are_equal;
  assert(result);
  assert(result->addr);
  assert(ref);
  assert(ref->addr);
  all_are_equal = matrix_is_same_size(result, ref);
  if (!all_are_equal)
    ;
  else if (matrix_datatype_is_float(result->datatype))
    all_are_equal = _matrix_equal_float(result, ref, prints);
  else
    all_are_equal = _matrix_equal_fixed(result, ref, prints);
  return all_are_equal;
}

const char *matrix_datatype_get_name(ervp_matrix_datatype_t datatype)
{
  char *result;
  switch (datatype)
  {
  case MATRIX_DATATYPE_UINT01:
    result = "UINT01";
    break;
  case MATRIX_DATATYPE_SINT02:
    result = "SINT02";
    break;
  case MATRIX_DATATYPE_UINT02:
    result = "UINT02";
    break;
  case MATRIX_DATATYPE_SINT04:
    result = "SINT04";
    break;
  case MATRIX_DATATYPE_UINT04:
    result = "UINT04";
    break;
  case MATRIX_DATATYPE_SINT08:
    result = "SINT08";
    break;
  case MATRIX_DATATYPE_UINT08:
    result = "UINT08";
    break;
  case MATRIX_DATATYPE_SINT16:
    result = "SINT16";
    break;
  case MATRIX_DATATYPE_UINT16:
    result = "UINT16";
    break;
  case MATRIX_DATATYPE_SINT32:
    result = "SINT32";
    break;
  case MATRIX_DATATYPE_UINT32:
    result = "UINT32";
    break;
  case MATRIX_DATATYPE_FLOAT32:
    result = "FLOAT32";
    break;
  default:
    result = NULL;
    assert(0);
  }
  return result;
}

void matrix_print_brief(const ErvpMatrixInfo *mat)
{
  printf("\n\n0x%08x, 0x%08x", mat->addr, mat->stride_ls3 >> 3);
  printf("\n%s %d x %d", matrix_datatype_get_name(mat->datatype), mat->num_row, mat->num_col);
}

void matrix_print(const ErvpMatrixInfo *mat)
{
  printf("\n\n0x%08p, 0x%08p", mat->addr, mat->stride_ls3 >> 3);
  printf("\n%s %d x %d", matrix_datatype_get_name(mat->datatype), mat->num_row, mat->num_col);
  for (int i = 0; i < mat->num_row; i++)
  {
    printf("\n");
    for (int j = 0; j < mat->num_col; j++)
    {
      UNKNOWN_TYPE data = _matrix_read_element(mat, i, j);
      if (mat->datatype == MATRIX_DATATYPE_FLOAT32)
        printf(" %.2f", data.value_f32);
      else if (mat->datatype == MATRIX_DATATYPE_UINT32)
        printf(" %8u", data.value_unsigned);
      else
        printf(" %8d", data.value_signed);
    }
  }
}

void matrix_print_hex_bit(const ErvpMatrixInfo *mat, int num_bits)
{
  char *format = " %00x";
  int num_digits = ((num_bits - 1) / 4) + 1;
  assert(num_digits <= 8);
  format[3] = '0' + num_digits;
  printf("\n%s", matrix_datatype_get_name(mat->datatype));
  for (int i = 0; i < mat->num_row; i++)
  {
    printf("\n");
    for (int j = 0; j < mat->num_col; j++)
    {
      UNKNOWN_TYPE data = _matrix_read_element(mat, i, j);
      printf(format, data.hex);
    }
  }
}

void matrix_print_hex(const ErvpMatrixInfo *mat)
{
  matrix_print_hex_bit(mat, matrix_datatype_get_num_bits(mat->datatype));
}
