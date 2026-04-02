#include "ervp_random_matrix.h"
#include "ervp_matrix_element.h"
#include "ervp_pseudo_random.h"

__attribute__((weak)) void generate_random_matrix(ErvpMatrixInfo *matrix_info, int min, int max)
{
  int i, j;

  assert(matrix_info);
  assert(matrix_info->addr);

  if (matrix_info->datatype == MATRIX_DATATYPE_FLOAT32)
  {
    assert(0);
  }
  else
  {
    for (j = 0; j < matrix_info->num_row; j++)
      for (i = 0; i < matrix_info->num_col; i++)
      {
        int value = rand_range(min, max);
        matrix_write_fixed_element(matrix_info, j, i, value);
      }
  }
}