#ifndef __ERVP_MATRIX_ELEMENT_H__
#define __ERVP_MATRIX_ELEMENT_H__

#include "ervp_assert.h"
#include "ervp_matrix.h"
#include "ervp_matrix_datatype_define.h"
#include "ervp_memory_util.h"
#include "ervp_bit_util.h"
#include "ervp_round_int.h"

#define MATRIX_DATATYPE_UINT32 GEN_MATRIX_DATATYPE(0, 0, 2, 0)

static inline int matrix_get_num_bits_per_row(const ErvpMatrixInfo *matrix)
{
  return matrix_datatype_get_num_bits(matrix->datatype) * matrix->num_col;
}

static inline int matrix_get_num_bytes_per_row(const ErvpMatrixInfo *matrix)
{
  int row_bitsize = matrix_get_num_bits_per_row(matrix);
  int row_size = rshift_ru(row_bitsize, 3);
  return row_size;
}

// check whether the matrix is stored as a 1D array
static inline int matrix_has_simple_layout(const ErvpMatrixInfo *matrix)
{
  int row_bitsize = matrix_get_num_bits_per_row(matrix);
  return (row_bitsize == matrix->stride_ls3);
}

static inline void *matrix_get_row_addr(const ErvpMatrixInfo *matrix, int row_index)
{
  assert(row_index < matrix->num_row);
  void *addr = 0;
  addr = matrix->addr;
  addr += ((matrix->stride_ls3 * row_index) >> 3);
  return addr;
}

static inline int matrix_get_element_addr_offset_in_row(const ErvpMatrixInfo *matrix, int col_index)
{
  int shift_amount = matrix_datatype_get_addr_lsa(matrix->datatype);
  int offset;
  if (shift_amount >= 0)
    offset = col_index << shift_amount;
  else
    offset = col_index >> (-shift_amount);
  return offset;
}

static inline int _matrix_get_element_addr_bit_offset(const ErvpMatrixInfo *matrix, int col_index)
{
  int shift_amount = matrix_datatype_get_addr_lsa(matrix->datatype);
  int bit_offset = 0;
  if (shift_amount < 0)
    bit_offset = (col_index << (3 + shift_amount)) & 7;
  return bit_offset;
}

static inline void *matrix_get_element_addr(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  assert(col_index < matrix->num_col);
  void *row_addr = matrix_get_row_addr(matrix, row_index);
  int offset = matrix_get_element_addr_offset_in_row(matrix, col_index);
  void *addr = row_addr + offset;
  assert_invalid_data_pointer(addr);
  return addr;
}

static inline int matrix_get_subbyte_rsa(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  int rsa;
  switch (matrix_datatype_get_num_bits(matrix->datatype))
  {
  case 1:
    rsa = col_index & 7;
    break;
  case 2:
    rsa = (col_index & 3) << 1;
    break;
  case 4:
    rsa = (col_index & 1) << 2;
    break;
  default:
    rsa = 0;
    assert(0);
  }
  int bit_offset = (row_index * matrix->stride_ls3) & 7;
  return bit_offset + rsa;
}

typedef union
{
  uint8_t byte;
  unsigned int value_u01 : 1;
  int value_s02 : 2;
  unsigned int value_u02 : 2;
  int value_s04 : 4;
  unsigned int value_u04 : 4;
} SubbyteType;

static inline UNKNOWN_TYPE _matrix_read_element(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  void *addr = matrix_get_element_addr(matrix, row_index, col_index);
  UNKNOWN_TYPE data;
  if (matrix_datatype_is_subbyte(matrix->datatype))
  {
    SubbyteType subbyte;
    subbyte.byte = *((uint8_t *)addr);
    subbyte.byte >>= matrix_get_subbyte_rsa(matrix, row_index, col_index);
    switch (matrix->datatype)
    {
    case MATRIX_DATATYPE_UINT01:
      data.value_unsigned = subbyte.value_u01;
      break;
    case MATRIX_DATATYPE_SINT02:
      data.value_signed = subbyte.value_s02;
      break;
    case MATRIX_DATATYPE_UINT02:
      data.value_unsigned = subbyte.value_u02;
      break;
    case MATRIX_DATATYPE_SINT04:
      data.value_signed = subbyte.value_s04;
      break;
    case MATRIX_DATATYPE_UINT04:
      data.value_unsigned = subbyte.value_u04;
      break;
    default:
      assert_msg(0, "%d %d", matrix->datatype, matrix_datatype_is_subbyte(matrix->datatype));
      data.value_unsigned = 0;
    }
  }
  else
    switch (matrix->datatype)
    {
    case MATRIX_DATATYPE_SINT08:
      data.value_signed = *((int8_t *)addr);
      break;
    case MATRIX_DATATYPE_UINT08:
      data.value_unsigned = *((uint8_t *)addr);
      break;
    case MATRIX_DATATYPE_SINT16:
      data.value_signed = *((int16_t *)addr);
      break;
    case MATRIX_DATATYPE_UINT16:
      data.value_unsigned = *((uint16_t *)addr);
      break;
    case MATRIX_DATATYPE_SINT32:
      data.value_signed = *((int32_t *)addr);
      break;
    case MATRIX_DATATYPE_UINT32:
    case MATRIX_DATATYPE_FLOAT32:
      data.value_unsigned = *((uint32_t *)addr);
      break;
    default:
      assert_msg(0, "%d %d", matrix->datatype, matrix_datatype_is_subbyte(matrix->datatype));
      data.value_unsigned = 0;
    }
  return data;
}

static inline void _matrix_write_element(const ErvpMatrixInfo *matrix, int row_index, int col_index, UNKNOWN_TYPE wdata)
{
  void *addr = matrix_get_element_addr(matrix, row_index, col_index);
  if (matrix_datatype_is_subbyte(matrix->datatype))
  {
    uint8_t byte = *((uint8_t *)addr);
    unsigned int upper_index, lower_index;
    int rsa = matrix_get_subbyte_rsa(matrix, row_index, col_index);
    lower_index = rsa;
    upper_index = lower_index + matrix_datatype_get_num_bits(matrix->datatype) - 1;
    *((uint8_t *)addr) = replace_partial_bits(byte, upper_index, lower_index, wdata.hex);
  }
  else
    switch (matrix_datatype_get_addr_lsa(matrix->datatype))
    {
    case 0:
      *((uint8_t *)addr) = wdata.br08;
      break;
    case 1:
      *((uint16_t *)addr) = wdata.br16;
      break;
    case 2:
      *((uint32_t *)addr) = wdata.br32;
      break;
    default:
      assert(0);
    }
}

static inline int matrix_read_sfixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  return _matrix_read_element(matrix, row_index, col_index).value_signed;
}

static inline void matrix_write_sfixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index, int wdata)
{
  UNKNOWN_TYPE temp;
  temp.value_signed = wdata;
  _matrix_write_element(matrix, row_index, col_index, temp);
}

static inline int matrix_read_ufixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  return _matrix_read_element(matrix, row_index, col_index).value_unsigned;
}

static inline void matrix_write_ufixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index, unsigned int wdata)
{
  UNKNOWN_TYPE temp;
  temp.value_unsigned = wdata;
  _matrix_write_element(matrix, row_index, col_index, temp);
}

static inline int matrix_read_fixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  return _matrix_read_element(matrix, row_index, col_index).value_signed;
}

static inline void matrix_write_fixed_element(const ErvpMatrixInfo *matrix, int row_index, int col_index, int wdata)
{
  UNKNOWN_TYPE temp;
  temp.value_signed = wdata;
  _matrix_write_element(matrix, row_index, col_index, temp);
}

static inline float matrix_read_float_element(const ErvpMatrixInfo *matrix, int row_index, int col_index)
{
  return _matrix_read_element(matrix, row_index, col_index).value_f32;
}

static inline void matrix_write_float_element(const ErvpMatrixInfo *matrix, int row_index, int col_index, float wdata)
{
  UNKNOWN_TYPE temp;
  temp.value_f32 = wdata;
  _matrix_write_element(matrix, row_index, col_index, temp);
}

#endif
