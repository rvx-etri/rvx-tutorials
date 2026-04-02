//#include <stdint.h>

#include "ervp_printf.h"
#include "ervp_assert.h"
#include "npx_util.h"

ErvpMatrixInfo *matrix_cast_sint8_to_float(const ErvpMatrixInfo *mat)
{
  //printf("\n%s\n", __func__);
  assert(mat->datatype==MATRIX_DATATYPE_SINT08);
  int i;
  ErvpMatrixInfo *result = matrix_alloc(MATRIX_DATATYPE_FLOAT32, 
                  mat->num_row, mat->num_col, NULL);
 
  int8_t *input = mat->addr;
  float *output = result->addr;
  int num_elements = mat->num_row*mat->num_col;
  for(i=0; i<num_elements; i++)
    output[i] = (float)input[i];

  return result;
}
