#include <string.h>
#include "ervp_assert.h"
#include "ervp_matrix_op.h"

ervp_hwtask_busy_fx_t _matrix_scalar_mult_float_sw(ervp_mop_mapping_t* mop_mapping, const ErvpMatrixInfo *a, float scalar_value, ErvpMatrixInfo *c, unsigned int option_value)
{
  int i, j;
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  assert(_mop_option_check(c, option_value));
  assert(mop_option.br.stride_m1==0);
  for(i=0; i<a->num_row; i++)
  {
    for(j=0; j<a->num_col; j++)
    {
      float result = 0;
      float a_value = matrix_read_float_element(a, i, j);
      result = a_value * scalar_value;
      result = _melement_perform_rshift_and_clip(result,mop_option.br.rshift,mop_option.br.performs_cliping,c->datatype);
      if(mop_option.br.acc)
        result += matrix_read_float_element(c, i, j);
      matrix_write_float_element(c, i, j, result);
    }
  }
  if(scalar_value==0)
    c->is_binary = 1;
  else if(a->is_binary)
    c->is_binary = (scalar_value==1) || (scalar_value==(-1));
  else
    c->is_binary = 0;
  mop_option_free(mop_option);
  return NULL;
}
