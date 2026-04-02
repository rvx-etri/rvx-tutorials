#include <string.h>
#include "ervp_assert.h"
#include "ervp_matrix_op.h"

void _matrix_add_fixed_sw(ervp_mop_mapping_t* mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int option_value)
{
  int i, j;
  ervp_mop_option_t mop_option = mop_option_alloc(option_value);
  assert(_mop_option_check(c, option_value));
  assert(mop_option.br.stride_m1==0);
	for(i=0; i<a->num_row; i++)
	{
		for(j=0; j<a->num_col; j++)
		{
      int result = 0;
      int a_value = matrix_read_fixed_element(a, i, j);
      int b_value = matrix_read_fixed_element(b, i, j);
      
      result = a_value + b_value;
      result = _melement_perform_rshift_and_clip(result,mop_option.br.rshift,mop_option.br.performs_cliping,c->datatype);
      if(mop_option.br.acc)
        result += matrix_read_fixed_element(c, i, j);
      matrix_write_fixed_element(c, i, j, result);
    }
	}
  c->is_binary = 0;
  mop_option_free(mop_option);
}
