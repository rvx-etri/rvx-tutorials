#ifndef __DCA_MATRIX_INFO_H__
#define __DCA_MATRIX_INFO_H__

#include "ervp_assert.h"
#include "ervp_matrix.h"
#include "ervp_reg_util.h"

typedef union
{
  uint32_t hex[5];
  struct
  {
    void *addr;
    int stride_ls3;
    unsigned int num_row_m1 : 32;
    unsigned int num_col_m1 : 32;
    unsigned int is_signed : 1;
    unsigned int is_float : 1;
    unsigned int addr_lsa_p3 : 3;
    unsigned int bit_offset : 3;
    unsigned int is_binary : 1;
  } br;
} dca_matrix_info_t;

dca_matrix_info_t *dca_matrix_info_generate(const ErvpMatrixInfo *info, dca_matrix_info_t *prealloated);
dca_matrix_info_t *dca_matrix_info_init(dca_matrix_info_t *prealloated);

static inline void dca_inst_init_except_matrix_info(void *inst, size_t inst_size, int num_matrix_info)
{
  unsigned int p = (unsigned int)inst;
  assert((p & 0x3) == 0);
  assert((sizeof(dca_matrix_info_t) & 0x3) == 0);
  assert((inst_size & 0x3) == 0);
  for (int i = (num_matrix_info * sizeof(dca_matrix_info_t)); i < inst_size; i += 4)
  {
    REG32(p + i) = 0;
  }
}

#endif // __DCA_MATRIX_INFO_H__