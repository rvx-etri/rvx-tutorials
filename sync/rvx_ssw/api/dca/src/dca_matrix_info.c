#include "dca_matrix_info.h"
#include "ervp_matrix_element.h"
#include "ervp_malloc.h"
#include "ervp_assert.h"

dca_matrix_info_t *dca_matrix_info_generate(const ErvpMatrixInfo *info, dca_matrix_info_t *prealloated)
{
  dca_matrix_info_t *result;
  if (prealloated != NULL)
    result = prealloated;
  else
  {
    result = malloc(sizeof(dca_matrix_info_t));
    assert(result);
  }
  result->hex[(sizeof(dca_matrix_info_t) >> 2) - 1] = 0;
  result->br.addr = info->addr;
  result->br.stride_ls3 = info->stride_ls3;
  result->br.num_row_m1 = info->num_row - 1;
  result->br.num_col_m1 = info->num_col - 1;
  result->br.bit_offset = info->bit_offset;
  result->br.is_binary = info->is_binary;
  ErvpMatrixDataType datatype;
  datatype.value = info->datatype;
  result->br.is_signed = datatype.br.is_signed;
  result->br.is_float = datatype.br.is_float;
  result->br.addr_lsa_p3 = datatype.br.addr_lsa + 3;
  return result;
}

dca_matrix_info_t *dca_matrix_info_init(dca_matrix_info_t *prealloated)
{
  dca_matrix_info_t *result;
  if (prealloated != NULL)
    result = prealloated;
  else
  {
    result = malloc(sizeof(dca_matrix_info_t));
    assert(result);
  }
  for (int i = 0; i < (sizeof(dca_matrix_info_t) >> 2); i++)
    result->hex[i] = 0;
  return result;
}