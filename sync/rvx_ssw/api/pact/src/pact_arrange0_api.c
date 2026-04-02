#include "ervp_malloc.h"
#include "ervp_matrix_element.h"
#include "core_dependent.h"

#include "pact.h"
#include "pact_arch_config.h"

#ifdef PACT_INCLUDE_NODE_LSU0
static const int lsu_output_reg = PACT_LSU0_OR00;
static const int lsu_input_reg = PACT_LSU0_IR00;
static void (*fx_insert_lsu_load)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu0_load;
static void (*fx_insert_lsu_store)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu0_store;
static PactInstBinary (*fx_insert_lsu_flush)() = pact_inst_lsu0_flush;
#else
#ifdef PACT_INCLUDE_NODE_LSU1
static const int lsu_output_reg = PACT_LSU1_OR00;
static const int lsu_input_reg = PACT_LSU1_IR00;
static void (*fx_insert_lsu_load)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu1_load;
static void (*fx_insert_lsu_store)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu1_store;
static PactInstBinary (*fx_insert_lsu_flush)() = pact_inst_lsu1_flush;
#endif
#endif

#ifdef PACT_INCLUDE_NODE_ARRANGE0

 // resource conflict with multicore
static PactFx* arrange_fx = 0;
static PactFx* arrange_cross_fx = 0;
static PactFx* scalar_mult_fx = 0;

static void __attribute__ ((constructor)) init_pact_arrange0_function() 
{
	// arrange_fx
	arrange_fx = pact_fx_alloc();
	fx_insert_lsu_load(arrange_fx, &invalid_matrix_info);
	pact_fx_insert_inst(arrange_fx, pact_inst_move(lsu_output_reg, PACT_ARRANGE0_BR00));
	pact_fx_insert_inst(arrange_fx, pact_transpose());
	pact_fx_insert_inst(arrange_fx, pact_inst_move(PACT_ARRANGE0_BR00, lsu_input_reg));
	fx_insert_lsu_store(arrange_fx, &invalid_matrix_info);
	pact_fx_insert_inst(arrange_fx, fx_insert_lsu_flush());
	// arrange_cross_fx
	arrange_cross_fx = pact_fx_alloc();
	fx_insert_lsu_load(arrange_cross_fx, &invalid_matrix_info);
	pact_fx_insert_inst(arrange_cross_fx, pact_inst_move(lsu_output_reg, PACT_ARRANGE0_BR00));
	fx_insert_lsu_load(arrange_cross_fx, &invalid_matrix_info);  // cross
	pact_fx_insert_inst(arrange_cross_fx, pact_transpose());
	pact_fx_insert_inst(arrange_cross_fx, pact_inst_move(PACT_ARRANGE0_BR00, lsu_input_reg));
	fx_insert_lsu_store(arrange_cross_fx, &invalid_matrix_info);
	pact_fx_insert_inst(arrange_cross_fx, pact_inst_move(lsu_output_reg, PACT_ARRANGE0_BR00)); // cross
	pact_fx_insert_inst(arrange_cross_fx, pact_transpose()); // cross
	pact_fx_insert_inst(arrange_cross_fx, pact_inst_move(PACT_ARRANGE0_BR00, lsu_input_reg)); // cross
	fx_insert_lsu_store(arrange_cross_fx, &invalid_matrix_info); // cross
	pact_fx_insert_inst(arrange_cross_fx, fx_insert_lsu_flush());
	// scalar_mult_fx
	scalar_mult_fx = pact_fx_alloc();
	fx_insert_lsu_load(scalar_mult_fx, &invalid_matrix_info);
	pact_fx_insert_reconfigure_inst(scalar_mult_fx);
	pact_fx_insert_inst(scalar_mult_fx, pact_inst_move(PACT_ARRANGE0_BR00, PACT_MAC0_IR01));	
	pact_fx_insert_inst(scalar_mult_fx, pact_inst_move(lsu_output_reg, PACT_MAC0_IR00));
	pact_fx_insert_reconfigure_inst(scalar_mult_fx);
	pact_fx_insert_inst(scalar_mult_fx, pact_inst_move(PACT_MAC0_OR00, lsu_input_reg));
	fx_insert_lsu_store(scalar_mult_fx, &invalid_matrix_info);
	pact_fx_insert_inst(scalar_mult_fx, fx_insert_lsu_flush());
	//
	flush_cache();
}

static void __pact_matrix_transpose(const ErvpMatrixInfo *a, ErvpMatrixInfo *c)
{
	pact_fx_config_memory_access(arrange_fx, 0, a);
	pact_fx_config_memory_access(arrange_fx, 1, c);
	//
	pact_fx_execute(arrange_fx);
	pact_wait_until_finished();
}

static void __pact_matrix_transpose_and_cross(const ErvpMatrixInfo *a1,
    const ErvpMatrixInfo *a2, ErvpMatrixInfo *c1, ErvpMatrixInfo *c2)
{
	pact_fx_config_memory_access(arrange_cross_fx, 0, a1);
	pact_fx_config_memory_access(arrange_cross_fx, 1, a2);
	pact_fx_config_memory_access(arrange_cross_fx, 2, c1);
	pact_fx_config_memory_access(arrange_cross_fx, 3, c2);
	//
	pact_fx_execute(arrange_cross_fx);
	pact_wait_until_finished();
}

static inline void __pact_set_parted(const ErvpMatrixInfo *a, ErvpMatrixInfo *parted_a, int row, int col)
{
  parted_a->num_row = GET_PARTED_NUM_ROW_COL(a->num_row, row);
  parted_a->num_col = GET_PARTED_NUM_ROW_COL(a->num_col, col);
  parted_a->addr = matrix_get_element_addr(a, row, col);
}

void pact_matrix_transpose(const ErvpMatrixInfo *a, ErvpMatrixInfo *c)
{
  int row, col;
  ErvpMatrixInfo parted_a1 = *a;
  ErvpMatrixInfo parted_c1 = *c;
  ErvpMatrixInfo parted_a2 = *a;
  ErvpMatrixInfo parted_c2 = *c;

  const int MAX_SIZE = (a->num_row > a->num_col)? a->num_row: a->num_col;

  for (row = 0; row < MAX_SIZE; row += PACT_MATRIX_SIZE) {
    for (col = 0; col < MAX_SIZE; col += PACT_MATRIX_SIZE) {
      if(row == col){
        if((a->num_row > row) && (a->num_col > col))
        {
          __pact_set_parted(a, &parted_a1, row, col);
          __pact_set_parted(c, &parted_c1, row, col);

          __pact_matrix_transpose(&parted_a1, &parted_c1);
        }
      }
      else if(col > row){
        if(col >= a->num_col){
          __pact_set_parted(a, &parted_a1, col, row);
          __pact_set_parted(c, &parted_c1, row, col);

          __pact_matrix_transpose(&parted_a1, &parted_c1);
        }
        else if(col >= a->num_row){
          __pact_set_parted(a, &parted_a2, row, col);
          __pact_set_parted(c, &parted_c2, col, row);

          __pact_matrix_transpose(&parted_a2, &parted_c2);
        }
        else{
          __pact_set_parted(a, &parted_a1, col, row);
          __pact_set_parted(c, &parted_c1, row, col);
          __pact_set_parted(a, &parted_a2, row, col);
          __pact_set_parted(c, &parted_c2, col, row);

          __pact_matrix_transpose_and_cross(&parted_a1, &parted_a2, &parted_c1, &parted_c2);
        }
      }
    }
  }
}

static void __pact_matrix_scalar_mult_int(const ErvpMatrixInfo *a, int value, ErvpMatrixInfo *c)
{
	pact_fx_set_reconfigure_inst(scalar_mult_fx, 0, pact_inst_fixed_scalar(value));
	pact_fx_set_reconfigure_inst(scalar_mult_fx, 1, pact_inst_fixed_ewmult());
	pact_fx_config_memory_access(scalar_mult_fx, 0, a);
	pact_fx_config_memory_access(scalar_mult_fx, 1, c);
	//
	pact_fx_execute(scalar_mult_fx);
	pact_wait_until_finished();
}

void pact_matrix_scalar_mult_int(const ErvpMatrixInfo *a, int value, ErvpMatrixInfo *c)
{
  int m, n;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int N = a->num_col;

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;

    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_a.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_a.addr = matrix_get_element_addr(a, m, n);
      parted_c.addr = matrix_get_element_addr(c, m, n);

      __pact_matrix_scalar_mult_int(&parted_a, value, &parted_c);
    }
  }
}

static void __pact_matrix_scalar_mult_float(const ErvpMatrixInfo *a, float value, ErvpMatrixInfo *c)
{
	pact_fx_set_reconfigure_inst(scalar_mult_fx, 0, pact_inst_float_scalar(value));
	pact_fx_set_reconfigure_inst(scalar_mult_fx, 1, pact_inst_float_ewmult());
	pact_fx_config_memory_access(scalar_mult_fx, 0, a);
	pact_fx_config_memory_access(scalar_mult_fx, 1, c);
	//
	pact_fx_execute(scalar_mult_fx);
	pact_wait_until_finished();
}

void pact_matrix_scalar_mult_float(const ErvpMatrixInfo *a, float value, ErvpMatrixInfo *c)
{
  int m, n;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int N = a->num_col;

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;

    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_a.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_a.addr = matrix_get_element_addr(a, m, n);
      parted_c.addr = matrix_get_element_addr(c, m, n);

      __pact_matrix_scalar_mult_float(&parted_a, value, &parted_c);
    }
  }
}

#else // PACT_INCLUDE_NODE_ARRANGE0

void pact_matrix_transpose(const ErvpMatrixInfo *a, ErvpMatrixInfo *c){}
void pact_matrix_scalar_mult_int(const ErvpMatrixInfo *a, int value, ErvpMatrixInfo *c){}
void pact_matrix_scalar_mult_float(const ErvpMatrixInfo *a, float value, ErvpMatrixInfo *c){}

#endif
