#include <string.h>
#include "ervp_assert.h"
#include "ervp_matrix_op.h"
#include "ervp_smart_flush.h"

void _matrix_min_float_sw(ervp_mop_mapping_t* mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
  // printf_function();
  int i, j;
  assert(!a->is_scalar);

  if(b->is_scalar)
  {
    float b_value = matrix_read_float_element(b, 0, 0);
    for(i=0; i<a->num_row; i++)
    {
      for(j=0; j<a->num_col; j++)
      {
        float result;
        float a_value = matrix_read_float_element(a, i, j);
        
        result = (a_value <= b_value)? a_value : b_value;
        matrix_write_float_element(c, i, j, result);
      }
    }
  }
  else
  {
    for(i=0; i<a->num_row; i++)
    {
      for(j=0; j<a->num_col; j++)
      {
        float result;
        float a_value = matrix_read_float_element(a, i, j);
        float b_value = matrix_read_float_element(b, i, j);
        
        result = (a_value <= b_value)? a_value : b_value;
        matrix_write_float_element(c, i, j, result);
      }
    }
  }
  //
  trackedvar_add(a->addr, 0);
  trackedvar_add(b->addr, 0);
  trackedvar_add(c->addr, 1);
}
