#ifndef __ERVP_MATRIX_DATATYPE_DEFINE_H__
#define __ERVP_MATRIX_DATATYPE_DEFINE_H__

#include "ervp_round_int.h"

typedef union
{
  unsigned int value;
  struct
  {
    int8_t addr_lsa;
    uint8_t is_signed;
    uint8_t is_float;
    uint8_t num_bits;
  } br;
} ErvpMatrixDataType;

#define GEN_MATRIX_DATATYPE(is_float, is_signed, addr_lsa, num_bits) (((num_bits&0xFF)<<24)|((is_float&0xFF)<<16)|((is_signed&0xFF)<<8)|((addr_lsa&0xFF)<<0))
#define GET_NUM_BITS(datatype) (datatype>>24)

static inline int matrix_datatype_get_addr_lsa(int datatype)
{
  ErvpMatrixDataType temp;
  temp.value = datatype;
  return temp.br.addr_lsa;
}

static inline int matrix_datatype_get_num_bits(int datatype)
{
  ErvpMatrixDataType temp;
  temp.value = datatype;
  int result = temp.br.num_bits;
  return result;
}

static inline int matrix_datatype_size(int datatype)
{
  return rshift_ru(matrix_datatype_get_num_bits(datatype),3);
}

static inline int matrix_datatype_is_subbyte(int datatype)
{
  return (matrix_datatype_get_addr_lsa(datatype)<0);
}

static inline int matrix_datatype_is_float(int datatype)
{
  ErvpMatrixDataType temp;
  temp.value = datatype;
  return temp.br.is_float;
}

static inline int matrix_datatype_is_signed(int datatype)
{
  ErvpMatrixDataType temp;
  temp.value = datatype;
  return temp.br.is_signed;
}

static inline int matrix_datatype_get_lower_bound(int datatype)
{
  int bound;
  if(matrix_datatype_is_signed(datatype))
    bound = -(1<<(matrix_datatype_get_num_bits(datatype)-1));
  else
    bound = 0;
  return bound;
}

static inline int matrix_datatype_get_upper_bound(int datatype)
{
  int bound;
  if(matrix_datatype_is_signed(datatype))
    bound = (1<<(matrix_datatype_get_num_bits(datatype)-1))-1;
  else
    bound = (1<<matrix_datatype_get_num_bits(datatype))-1;
  return bound;
}

#define GET_MATRIX_CASE_TYPE(a, b, c) (((a)<<8) + ((b)<<8) + (c))

typedef union
{
  unsigned int hex;
	uint8_t br08;
  uint16_t br16;
  uint32_t br32;

  int value_signed;
  unsigned int value_unsigned;
  float value_f32;
} UNKNOWN_TYPE;

#endif
