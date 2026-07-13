#include <string.h>
#include "ervp_assert.h"
#include "ervp_matrix_op.h"
#include "ervp_smart_flush.h"

void _matrix_compare_fixed_sw(ervp_mop_mapping_t* mop_mapping, const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c, unsigned int compare_mode)
{
  // printf_function();
  int i, j;
  assert(!a->is_scalar);

  if(b->is_scalar)
  {
    int b_value = matrix_read_fixed_element(b, 0, 0);
    for(i=0; i<a->num_row; i++)
    {
      for(j=0; j<a->num_col; j++)
      {
        int result;
        int a_value = matrix_read_fixed_element(a, i, j);

        switch(compare_mode)
        {
          case COMPARE_GE:
            result = (a_value >= b_value) ? 1 : 0;
            break;
          case COMPARE_GT:
            result = (a_value > b_value) ? 1 : 0;
            break;
          case COMPARE_LE:
            result = (a_value <= b_value) ? 1 : 0;
            break;
          case COMPARE_LT:
            result = (a_value < b_value) ? 1 : 0;
            break;
          case COMPARE_EQ:
            result = (a_value == b_value) ? 1 : 0;
            break;
          case COMPARE_NE:
            result = (a_value != b_value) ? 1 : 0;
            break;
          default:
            assert(0);
        }
        matrix_write_fixed_element(c, i, j, result);
      }
    }
  }
  else
  {
    for(i=0; i<a->num_row; i++)
    {
      for(j=0; j<a->num_col; j++)
      {
        int result;
        int a_value = matrix_read_fixed_element(a, i, j);
        int b_value = matrix_read_fixed_element(b, i, j);

        switch(compare_mode)
        {
          case COMPARE_GE:
            result = (a_value >= b_value) ? 1 : 0;
            break;
          case COMPARE_GT:
            result = (a_value > b_value) ? 1 : 0;
            break;
          case COMPARE_LE:
            result = (a_value <= b_value) ? 1 : 0;
            break;
          case COMPARE_LT:
            result = (a_value < b_value) ? 1 : 0;
            break;
          case COMPARE_EQ:
            result = (a_value == b_value) ? 1 : 0;
            break;
          case COMPARE_NE:
            result = (a_value != b_value) ? 1 : 0;
            break;
          default:
            assert(0);
        }
        matrix_write_fixed_element(c, i, j, result);
      }
    }
  }
  //
  trackedvar_add(a->addr, 0);
  trackedvar_add(b->addr, 0);
  trackedvar_add(c->addr, 1);
}
